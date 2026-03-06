// HeatSource.h - Campfire / heat source with cooking, warmth, and fuel system
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/InteractableInterface.h"
#include "HeatSource.generated.h"

class UPointLightComponent;
class UAudioComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EHeatSourceType : uint8
{
    Campfire     UMETA(DisplayName = "Campfire"),
    Fireplace    UMETA(DisplayName = "Fireplace"),
    Furnace      UMETA(DisplayName = "Furnace"),
    Torch        UMETA(DisplayName = "Torch"),
    Stove        UMETA(DisplayName = "Stove"),
    Barrel_Fire  UMETA(DisplayName = "Barrel Fire")
};

UENUM(BlueprintType)
enum class EFireState : uint8
{
    Unlit       UMETA(DisplayName = "Unlit"),
    Igniting    UMETA(DisplayName = "Igniting"),
    Burning     UMETA(DisplayName = "Burning"),
    LowFuel     UMETA(DisplayName = "Low Fuel"),
    Dying       UMETA(DisplayName = "Dying"),
    Extinguished UMETA(DisplayName = "Extinguished")
};

USTRUCT(BlueprintType)
struct FFuelItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ItemId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BurnTime = 60.0f;        // Seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HeatOutput = 1.0f;       // Multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float IgnitionChance = 0.8f;   // Success chance to light
};

USTRUCT(BlueprintType)
struct FCookingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName InputItemId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName OutputItemId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CookTime = 30.0f;         // Seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BurnTime = 60.0f;         // Time until burned
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName BurnedItemId;             // What it becomes if overcooked
};

USTRUCT(BlueprintType)
struct FCookingSlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FName ItemId;
    UPROPERTY(BlueprintReadOnly) float CookProgress = 0.0f;
    UPROPERTY(BlueprintReadOnly) float TotalCookTime = 0.0f;
    UPROPERTY(BlueprintReadOnly) float BurnTime = 0.0f;
    UPROPERTY(BlueprintReadOnly) bool bIsCooked = false;
    UPROPERTY(BlueprintReadOnly) bool bIsBurned = false;
    UPROPERTY(BlueprintReadOnly) FName OutputItemId;
    UPROPERTY(BlueprintReadOnly) FName BurnedItemId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireStateChanged, EFireState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemCooked, FName, InputItem, FName, OutputItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFuelChanged, float, RemainingFuel);

UCLASS()
class BIOMESURVIVOR_API AHeatSource : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    AHeatSource();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── IInteractableInterface ───

    virtual bool Interact_Implementation(AActor* Interactor) override;
    virtual bool InteractWithOption_Implementation(AActor* Interactor, FName OptionId) override;
    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual FInteractionData GetInteractionData_Implementation() const override;
    virtual void OnFocused_Implementation(AActor* Interactor) override;
    virtual void OnUnfocused_Implementation(AActor* Interactor) override;
    virtual void OnInteractionProgress_Implementation(float Progress) override;

    // ─── Fire API ───

    UFUNCTION(BlueprintCallable, Category = "HeatSource")
    bool Ignite(AActor* Igniter);

    UFUNCTION(BlueprintCallable, Category = "HeatSource")
    void Extinguish();

    UFUNCTION(BlueprintCallable, Category = "HeatSource")
    bool AddFuel(FName FuelItemId);

    UFUNCTION(BlueprintCallable, Category = "HeatSource")
    bool PlaceItemToCook(FName ItemId, int32 SlotIndex = -1);

    UFUNCTION(BlueprintCallable, Category = "HeatSource")
    FName RemoveCookedItem(int32 SlotIndex);

    // ─── Getters ───

    UFUNCTION(BlueprintPure, Category = "HeatSource")
    EFireState GetFireState() const { return FireState; }

    UFUNCTION(BlueprintPure, Category = "HeatSource")
    bool IsBurning() const { return FireState == EFireState::Burning || FireState == EFireState::LowFuel; }

    UFUNCTION(BlueprintPure, Category = "HeatSource")
    float GetHeatRadius() const { return HeatRadius; }

    UFUNCTION(BlueprintPure, Category = "HeatSource")
    float GetHeatIntensity() const;

    UFUNCTION(BlueprintPure, Category = "HeatSource")
    float GetRemainingFuel() const { return CurrentFuel; }

    UFUNCTION(BlueprintPure, Category = "HeatSource")
    float GetFuelPercentage() const { return MaxFuel > 0 ? CurrentFuel / MaxFuel : 0.0f; }

    UFUNCTION(BlueprintPure, Category = "HeatSource")
    TArray<FCookingSlot> GetCookingSlots() const { return CookingSlots; }

    UFUNCTION(BlueprintPure, Category = "HeatSource")
    float GetTemperatureBoostAt(FVector Location) const;

    // ─── Events ───

    UPROPERTY(BlueprintAssignable) FOnFireStateChanged OnFireStateChanged;
    UPROPERTY(BlueprintAssignable) FOnItemCooked OnItemCooked;
    UPROPERTY(BlueprintAssignable) FOnFuelChanged OnFuelChanged;

protected:
    // ─── Components ───

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* BaseMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* FireLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* FireAudioLoop;

    // ─── Configuration ───

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatSource")
    EHeatSourceType SourceType = EHeatSourceType::Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatSource")
    float HeatRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatSource")
    float BaseHeatIntensity = 25.0f;  // Temperature degrees boost at center

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatSource")
    float MaxFuel = 300.0f;  // Max fuel in seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatSource")
    float FuelConsumptionRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatSource")
    int32 MaxCookingSlots = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatSource")
    bool bCanBeExtinguishedByRain = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatSource")
    float RainExtinguishChancePerSecond = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatSource")
    bool bStartLit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatSource")
    float WindFlickerIntensity = 0.2f;

    // ─── Fuel Table ───

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuel")
    TArray<FFuelItem> AcceptedFuels;

    // ─── Cooking ───

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking")
    TArray<FCookingRecipe> CookingRecipes;

    // ─── Audio ───

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* IgniteSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* ExtinguishSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* AddFuelSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* CookCompleteSound;

    // ─── Light Settings ───

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float MaxLightIntensity = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float LightAttenuationRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    FLinearColor FireColor = FLinearColor(1.0f, 0.6f, 0.15f, 1.0f);

private:
    void SetFireState(EFireState NewState);
    void UpdateFire(float DeltaTime);
    void UpdateCooking(float DeltaTime);
    void UpdateLighting(float DeltaTime);
    void CheckRainExtinguish(float DeltaTime);

    const FCookingRecipe* FindRecipeForItem(FName ItemId) const;
    const FFuelItem* FindFuelData(FName ItemId) const;
    int32 FindEmptyCookingSlot() const;

    EFireState FireState = EFireState::Unlit;
    float CurrentFuel = 0.0f;
    float HeatMultiplier = 1.0f;
    float LightFlickerTimer = 0.0f;

    UPROPERTY()
    TArray<FCookingSlot> CookingSlots;
};
