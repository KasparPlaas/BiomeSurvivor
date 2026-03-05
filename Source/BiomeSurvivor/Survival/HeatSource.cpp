// HeatSource.cpp - Campfire / heat source implementation
#include "Survival/HeatSource.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AHeatSource::AHeatSource()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick for fire updates
    bReplicates = true;

    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
    RootComponent = BaseMesh;

    FireLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FireLight"));
    FireLight->SetupAttachment(RootComponent);
    FireLight->SetRelativeLocation(FVector(0, 0, 50.0f));
    FireLight->SetIntensity(0.0f);
    FireLight->SetAttenuationRadius(LightAttenuationRadius);
    FireLight->SetLightColor(FireColor);
    FireLight->SetVisibility(false);

    FireAudioLoop = CreateDefaultSubobject<UAudioComponent>(TEXT("FireAudioLoop"));
    FireAudioLoop->SetupAttachment(RootComponent);
    FireAudioLoop->bAutoActivate = false;
}

void AHeatSource::BeginPlay()
{
    Super::BeginPlay();

    CookingSlots.SetNum(MaxCookingSlots);

    // Set default fuels if none configured
    if (AcceptedFuels.Num() == 0)
    {
        FFuelItem Wood;
        Wood.ItemId = "item_wood";
        Wood.BurnTime = 60.0f;
        Wood.HeatOutput = 1.0f;
        AcceptedFuels.Add(Wood);

        FFuelItem Stick;
        Stick.ItemId = "item_stick";
        Stick.BurnTime = 20.0f;
        Stick.HeatOutput = 0.6f;
        AcceptedFuels.Add(Stick);

        FFuelItem Coal;
        Coal.ItemId = "item_coal";
        Coal.BurnTime = 180.0f;
        Coal.HeatOutput = 1.5f;
        AcceptedFuels.Add(Coal);

        FFuelItem Cloth;
        Cloth.ItemId = "item_cloth";
        Cloth.BurnTime = 15.0f;
        Cloth.HeatOutput = 0.4f;
        AcceptedFuels.Add(Cloth);
    }

    // Set default cooking recipes if none configured
    if (CookingRecipes.Num() == 0)
    {
        auto AddRecipe = [this](const FName& In, const FName& Out, float Cook, float Burn, const FName& Burned)
        {
            FCookingRecipe R;
            R.InputItemId = In;
            R.OutputItemId = Out;
            R.CookTime = Cook;
            R.BurnTime = Burn;
            R.BurnedItemId = Burned;
            CookingRecipes.Add(R);
        };
        AddRecipe("item_raw_meat", "item_cooked_meat", 30.0f, 60.0f, "item_burned_meat");
        AddRecipe("item_raw_fish", "item_cooked_fish", 25.0f, 50.0f, "item_burned_fish");
        AddRecipe("item_raw_rabbit", "item_cooked_rabbit", 20.0f, 45.0f, "item_burned_meat");
        AddRecipe("item_snow", "item_water", 10.0f, 999.0f, "");
        AddRecipe("item_raw_berries", "item_tea", 15.0f, 40.0f, "item_charcoal");
    }

    if (bStartLit)
    {
        CurrentFuel = MaxFuel * 0.5f;
        SetFireState(EFireState::Burning);
    }
}

void AHeatSource::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (FireState == EFireState::Unlit || FireState == EFireState::Extinguished) return;

    UpdateFire(DeltaTime);
    UpdateCooking(DeltaTime);
    UpdateLighting(DeltaTime);

    if (bCanBeExtinguishedByRain)
    {
        CheckRainExtinguish(DeltaTime);
    }
}

// ─── IInteractableInterface ────────────────────────────────────────

bool AHeatSource::Interact_Implementation(AActor* Interactor)
{
    if (!Interactor) return false;

    if (!IsBurning())
    {
        return Ignite(Interactor);
    }

    // If burning, open cooking UI
    return true;
}

bool AHeatSource::InteractWithOption_Implementation(AActor* Interactor, FName OptionId)
{
    if (OptionId == "Light") return Ignite(Interactor);
    if (OptionId == "Extinguish") { Extinguish(); return true; }
    if (OptionId == "AddFuel") return AddFuel("item_wood"); // Default fuel
    return false;
}

bool AHeatSource::CanInteract_Implementation(AActor* Interactor) const
{
    if (!Interactor) return false;
    return FVector::Dist(GetActorLocation(), Interactor->GetActorLocation()) < 300.0f;
}

FInteractionData AHeatSource::GetInteractionData_Implementation() const
{
    FInteractionData Data;
    Data.InteractableName = FText::FromString(
        SourceType == EHeatSourceType::Campfire ? "Campfire" :
        SourceType == EHeatSourceType::Fireplace ? "Fireplace" :
        SourceType == EHeatSourceType::Furnace ? "Furnace" :
        SourceType == EHeatSourceType::Torch ? "Torch" :
        SourceType == EHeatSourceType::Stove ? "Stove" : "Fire"
    );

    if (!IsBurning())
    {
        Data.ActionText = FText::FromString("Light");
        FInteractionOption Opt;
        Opt.OptionId = "Light";
        Opt.DisplayText = FText::FromString("Light Fire");
        Data.Options.Add(Opt);
    }
    else
    {
        Data.ActionText = FText::FromString("Cook");

        FInteractionOption AddFuelOpt;
        AddFuelOpt.OptionId = "AddFuel";
        AddFuelOpt.DisplayText = FText::FromString("Add Fuel");
        Data.Options.Add(AddFuelOpt);

        FInteractionOption ExtinguishOpt;
        ExtinguishOpt.OptionId = "Extinguish";
        ExtinguishOpt.DisplayText = FText::FromString("Extinguish");
        Data.Options.Add(ExtinguishOpt);
    }

    Data.InteractionType = EInteractionType::Simple;
    return Data;
}

void AHeatSource::OnFocused_Implementation(AActor* Interactor)
{
    BaseMesh->SetRenderCustomDepth(true);
    BaseMesh->SetCustomDepthStencilValue(1);
}

void AHeatSource::OnUnfocused_Implementation(AActor* Interactor)
{
    BaseMesh->SetRenderCustomDepth(false);
}

void AHeatSource::OnInteractionProgress_Implementation(float Progress)
{
}

// ─── Fire API ──────────────────────────────────────────────────────

bool AHeatSource::Ignite(AActor* Igniter)
{
    if (IsBurning()) return false;

    if (CurrentFuel <= 0.0f)
    {
        // Need fuel first
        return false;
    }

    SetFireState(EFireState::Igniting);

    // Brief ignition delay then full burn
    FTimerHandle IgniteTimer;
    GetWorldTimerManager().SetTimer(IgniteTimer, [this]()
    {
        if (FireState == EFireState::Igniting)
        {
            SetFireState(EFireState::Burning);
        }
    }, 1.5f, false);

    if (IgniteSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, IgniteSound, GetActorLocation());
    }

    return true;
}

void AHeatSource::Extinguish()
{
    if (!IsBurning() && FireState != EFireState::Igniting) return;

    SetFireState(EFireState::Extinguished);

    if (ExtinguishSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ExtinguishSound, GetActorLocation());
    }
}

bool AHeatSource::AddFuel(FName FuelItemId)
{
    const FFuelItem* FuelData = FindFuelData(FuelItemId);
    if (!FuelData) return false;

    CurrentFuel = FMath::Min(CurrentFuel + FuelData->BurnTime, MaxFuel);
    HeatMultiplier = FMath::Max(HeatMultiplier, FuelData->HeatOutput);

    OnFuelChanged.Broadcast(CurrentFuel);

    if (AddFuelSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, AddFuelSound, GetActorLocation());
    }

    // If fire was dying or low, boost it back
    if (FireState == EFireState::Dying || FireState == EFireState::LowFuel)
    {
        SetFireState(EFireState::Burning);
    }

    return true;
}

bool AHeatSource::PlaceItemToCook(FName ItemId, int32 SlotIndex)
{
    if (!IsBurning()) return false;

    const FCookingRecipe* Recipe = FindRecipeForItem(ItemId);
    if (!Recipe) return false;

    if (SlotIndex < 0)
    {
        SlotIndex = FindEmptyCookingSlot();
    }

    if (SlotIndex < 0 || SlotIndex >= CookingSlots.Num()) return false;
    if (CookingSlots[SlotIndex].ItemId != NAME_None) return false;

    FCookingSlot& Slot = CookingSlots[SlotIndex];
    Slot.ItemId = ItemId;
    Slot.CookProgress = 0.0f;
    Slot.TotalCookTime = Recipe->CookTime;
    Slot.BurnTime = Recipe->BurnTime;
    Slot.bIsCooked = false;
    Slot.bIsBurned = false;
    Slot.OutputItemId = Recipe->OutputItemId;
    Slot.BurnedItemId = Recipe->BurnedItemId;

    return true;
}

FName AHeatSource::RemoveCookedItem(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= CookingSlots.Num()) return NAME_None;

    FCookingSlot& Slot = CookingSlots[SlotIndex];
    if (Slot.ItemId == NAME_None) return NAME_None;

    FName ResultItem;
    if (Slot.bIsBurned)
    {
        ResultItem = Slot.BurnedItemId;
    }
    else if (Slot.bIsCooked)
    {
        ResultItem = Slot.OutputItemId;
    }
    else
    {
        ResultItem = Slot.ItemId; // Raw item back
    }

    // Clear slot
    Slot = FCookingSlot();
    return ResultItem;
}

float AHeatSource::GetHeatIntensity() const
{
    switch (FireState)
    {
    case EFireState::Burning: return BaseHeatIntensity * HeatMultiplier;
    case EFireState::LowFuel: return BaseHeatIntensity * HeatMultiplier * 0.5f;
    case EFireState::Dying: return BaseHeatIntensity * HeatMultiplier * 0.2f;
    case EFireState::Igniting: return BaseHeatIntensity * 0.3f;
    default: return 0.0f;
    }
}

float AHeatSource::GetTemperatureBoostAt(FVector Location) const
{
    if (!IsBurning()) return 0.0f;

    const float Distance = FVector::Dist(GetActorLocation(), Location);
    if (Distance > HeatRadius) return 0.0f;

    // Inverse square falloff
    const float NormalizedDist = Distance / HeatRadius;
    const float Falloff = FMath::Clamp(1.0f - (NormalizedDist * NormalizedDist), 0.0f, 1.0f);

    return GetHeatIntensity() * Falloff;
}

// ─── Private Methods ───────────────────────────────────────────────

void AHeatSource::SetFireState(EFireState NewState)
{
    if (FireState == NewState) return;

    FireState = NewState;

    switch (NewState)
    {
    case EFireState::Burning:
    case EFireState::LowFuel:
        FireLight->SetVisibility(true);
        if (FireAudioLoop && !FireAudioLoop->IsPlaying())
        {
            FireAudioLoop->Play();
        }
        SetActorTickEnabled(true);
        break;

    case EFireState::Dying:
        // Keep light/audio but dimmed
        break;

    case EFireState::Unlit:
    case EFireState::Extinguished:
        FireLight->SetVisibility(false);
        FireLight->SetIntensity(0.0f);
        if (FireAudioLoop && FireAudioLoop->IsPlaying())
        {
            FireAudioLoop->FadeOut(2.0f, 0.0f);
        }
        break;

    default:
        break;
    }

    OnFireStateChanged.Broadcast(NewState);
}

void AHeatSource::UpdateFire(float DeltaTime)
{
    if (FireState == EFireState::Igniting) return;

    CurrentFuel -= FuelConsumptionRate * DeltaTime;
    CurrentFuel = FMath::Max(CurrentFuel, 0.0f);

    const float FuelPct = GetFuelPercentage();

    if (CurrentFuel <= 0.0f)
    {
        SetFireState(EFireState::Extinguished);
    }
    else if (FuelPct < 0.1f)
    {
        if (FireState != EFireState::Dying) SetFireState(EFireState::Dying);
    }
    else if (FuelPct < 0.25f)
    {
        if (FireState != EFireState::LowFuel) SetFireState(EFireState::LowFuel);
    }
    else
    {
        if (FireState != EFireState::Burning) SetFireState(EFireState::Burning);
    }
}

void AHeatSource::UpdateCooking(float DeltaTime)
{
    if (!IsBurning()) return;

    for (int32 i = 0; i < CookingSlots.Num(); i++)
    {
        FCookingSlot& Slot = CookingSlots[i];
        if (Slot.ItemId == NAME_None) continue;
        if (Slot.bIsBurned) continue;

        Slot.CookProgress += DeltaTime;

        if (!Slot.bIsCooked && Slot.CookProgress >= Slot.TotalCookTime)
        {
            Slot.bIsCooked = true;
            OnItemCooked.Broadcast(Slot.ItemId, Slot.OutputItemId);

            if (CookCompleteSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, CookCompleteSound, GetActorLocation());
            }
        }

        if (Slot.bIsCooked && Slot.CookProgress >= Slot.BurnTime)
        {
            Slot.bIsBurned = true;
            if (Slot.BurnedItemId != NAME_None)
            {
                OnItemCooked.Broadcast(Slot.OutputItemId, Slot.BurnedItemId);
            }
        }
    }
}

void AHeatSource::UpdateLighting(float DeltaTime)
{
    if (!FireLight) return;

    LightFlickerTimer += DeltaTime;

    const float IntensityPct = GetFuelPercentage();
    const float BaseIntensity = MaxLightIntensity * FMath::Clamp(IntensityPct, 0.1f, 1.0f);

    // Flicker effect
    const float Flicker = FMath::Sin(LightFlickerTimer * 12.0f) * 0.05f
        + FMath::Sin(LightFlickerTimer * 7.3f) * 0.08f
        + FMath::Sin(LightFlickerTimer * 19.7f) * 0.03f;

    const float WindEffect = WindFlickerIntensity * FMath::Sin(LightFlickerTimer * 3.0f) * 0.1f;

    const float FinalIntensity = BaseIntensity * (1.0f + Flicker + WindEffect);
    FireLight->SetIntensity(FMath::Max(FinalIntensity, 0.0f));

    // Slight color shift as fire dies
    if (IntensityPct < 0.3f)
    {
        const float EmberLerp = 1.0f - (IntensityPct / 0.3f);
        FLinearColor EmberColor(1.0f, 0.3f, 0.05f, 1.0f);
        FLinearColor CurrentColor = FLinearColor::LerpUsingHSV(FireColor, EmberColor, EmberLerp);
        FireLight->SetLightColor(CurrentColor);
    }
}

void AHeatSource::CheckRainExtinguish(float DeltaTime)
{
    // This would integrate with the WeatherSystem
    // For now, check if there's rain via a simple trace or weather query
    // Placeholder: will be connected to WeatherSystem in integration pass
}

const FCookingRecipe* AHeatSource::FindRecipeForItem(FName ItemId) const
{
    for (const FCookingRecipe& Recipe : CookingRecipes)
    {
        if (Recipe.InputItemId == ItemId) return &Recipe;
    }
    return nullptr;
}

const FFuelItem* AHeatSource::FindFuelData(FName ItemId) const
{
    for (const FFuelItem& Fuel : AcceptedFuels)
    {
        if (Fuel.ItemId == ItemId) return &Fuel;
    }
    return nullptr;
}

int32 AHeatSource::FindEmptyCookingSlot() const
{
    for (int32 i = 0; i < CookingSlots.Num(); i++)
    {
        if (CookingSlots[i].ItemId == NAME_None) return i;
    }
    return -1;
}
