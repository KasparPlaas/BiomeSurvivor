// SaveLoadSubsystem.cpp - Save/Load subsystem implementation
#include "Core/SaveLoadSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

// Include game systems
#include "Player/SurvivorCharacter.h"
#include "Player/PlayerStatsComponent.h"
#include "Player/StatusEffectComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/EquipmentComponent.h"
#include "Survival/TemperatureComponent.h"
#include "Survival/SleepComponent.h"
#include "Survival/MedicalComponent.h"
#include "World/DayNightCycle.h"
#include "World/WeatherSystem.h"
#include "Core/BiomeSurvivorGameState.h"
#include "Building/BuildingPiece.h"

void USaveLoadSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("SaveLoadSubsystem initialized"));
}

void USaveLoadSubsystem::Deinitialize()
{
    if (UWorld* World = GetGameInstance()->GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
    }
    Super::Deinitialize();
}

// ─── Save Operations ───

bool USaveLoadSubsystem::SaveGame(const FString& SlotName, int32 UserIndex)
{
    USaveGameData* SaveData = CollectSaveData(SlotName, UserIndex);
    if (!SaveData) return false;

    bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveData, SlotName, UserIndex);
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Game saved to slot: %s"), *SlotName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save game to slot: %s"), *SlotName);
    }

    OnSaveCompleted.Broadcast(bSuccess);
    return bSuccess;
}

void USaveLoadSubsystem::SaveGameAsync(const FString& SlotName, int32 UserIndex)
{
    USaveGameData* SaveData = CollectSaveData(SlotName, UserIndex);
    if (!SaveData) return;

    FAsyncSaveGameToSlotDelegate Delegate;
    Delegate.BindUObject(this, &USaveLoadSubsystem::HandleAsyncSaveComplete);
    UGameplayStatics::AsyncSaveGameToSlot(SaveData, SlotName, UserIndex, Delegate);
}

bool USaveLoadSubsystem::QuickSave()
{
    return SaveGame(TEXT("QuickSave"), 0);
}

void USaveLoadSubsystem::AutoSave()
{
    if (!bAutoSaveEnabled) return;

    FString AutoSlotName = FString::Printf(TEXT("AutoSave_%d"), CurrentAutoSaveIndex);
    SaveGameAsync(AutoSlotName, 0);

    CurrentAutoSaveIndex = (CurrentAutoSaveIndex + 1) % MaxAutoSaveSlots;
}

// ─── Load Operations ───

bool USaveLoadSubsystem::LoadGame(const FString& SlotName, int32 UserIndex)
{
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Save slot does not exist: %s"), *SlotName);
        OnLoadCompleted.Broadcast(false);
        return false;
    }

    USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);
    USaveGameData* SaveData = Cast<USaveGameData>(LoadedGame);

    if (!SaveData)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load or cast save data from slot: %s"), *SlotName);
        OnLoadCompleted.Broadcast(false);
        return false;
    }

    LastLoadedSave = SaveData;
    bool bSuccess = ApplyLoadedData(SaveData);
    
    UE_LOG(LogTemp, Log, TEXT("Game loaded from slot: %s (Day %d, %.1f hours)"),
        *SlotName, SaveData->WorldState.DayNumber, SaveData->WorldState.TimeOfDay);

    OnLoadCompleted.Broadcast(bSuccess);
    return bSuccess;
}

void USaveLoadSubsystem::LoadGameAsync(const FString& SlotName, int32 UserIndex)
{
    FAsyncLoadGameFromSlotDelegate Delegate;
    Delegate.BindUObject(this, &USaveLoadSubsystem::HandleAsyncLoadComplete);
    UGameplayStatics::AsyncLoadGameFromSlot(SlotName, UserIndex, Delegate);
}

bool USaveLoadSubsystem::QuickLoad()
{
    return LoadGame(TEXT("QuickSave"), 0);
}

// ─── Slot Management ───

bool USaveLoadSubsystem::DeleteSave(const FString& SlotName, int32 UserIndex)
{
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex)) return false;
    return UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
}

bool USaveLoadSubsystem::DoesSaveExist(const FString& SlotName, int32 UserIndex) const
{
    return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

TArray<FString> USaveLoadSubsystem::GetAllSaveSlots() const
{
    TArray<FString> Slots;

    // Check standard slots
    if (UGameplayStatics::DoesSaveGameExist(TEXT("QuickSave"), 0))
        Slots.Add(TEXT("QuickSave"));

    // Check numbered save slots
    for (int32 i = 1; i <= 10; ++i)
    {
        FString SlotName = FString::Printf(TEXT("Save_%02d"), i);
        if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
            Slots.Add(SlotName);
    }

    // Check auto-save slots
    for (int32 i = 0; i < MaxAutoSaveSlots; ++i)
    {
        FString SlotName = FString::Printf(TEXT("AutoSave_%d"), i);
        if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
            Slots.Add(SlotName);
    }

    return Slots;
}

// ─── Data Collection ───

USaveGameData* USaveLoadSubsystem::CollectSaveData(const FString& SlotName, int32 UserIndex)
{
    USaveGameData* SaveData = Cast<USaveGameData>(
        UGameplayStatics::CreateSaveGameObject(USaveGameData::StaticClass()));
    
    if (!SaveData) return nullptr;

    SaveData->SaveSlotName = SlotName;
    SaveData->UserIndex = UserIndex;
    SaveData->SaveTimestamp = FDateTime::Now();
    SaveData->GameVersion = TEXT("0.1.0");
    SaveData->TotalPlayTimeSeconds = SessionPlayTime;

    UWorld* World = GetGameInstance()->GetWorld();
    if (!World) return nullptr;

    SaveData->MapName = World->GetMapName();

    // Collect player data
    CollectPlayerData(SaveData->PlayerData);

    // Collect world data
    CollectWorldData(SaveData->WorldState);

    return SaveData;
}

void USaveLoadSubsystem::CollectPlayerData(FSavedPlayerData& OutData)
{
    UWorld* World = GetGameInstance()->GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    ASurvivorCharacter* Character = Cast<ASurvivorCharacter>(PC->GetPawn());
    if (!Character) return;

    OutData.PlayerTransform = Character->GetActorTransform();
    OutData.ControlRotation = PC->GetControlRotation();

    // Stats
    if (UPlayerStatsComponent* Stats = Character->FindComponentByClass<UPlayerStatsComponent>())
    {
        OutData.Stats.Health = Stats->GetHealth();
        OutData.Stats.MaxHealth = Stats->MaxHealth;
        OutData.Stats.Hunger = Stats->GetHunger();
        OutData.Stats.Thirst = Stats->GetThirst();
        OutData.Stats.Stamina = Stats->GetStamina();
        OutData.Stats.Comfort = Stats->GetComfort();
    }

    // Temperature
    if (UTemperatureComponent* Temp = Character->FindComponentByClass<UTemperatureComponent>())
    {
        OutData.Stats.BodyTemperature = Temp->BodyTemperature;
    }

    // Sleep
    if (USleepComponent* Sleep = Character->FindComponentByClass<USleepComponent>())
    {
        OutData.Stats.Fatigue = Sleep->Fatigue;
    }

    // Inventory
    if (UInventoryComponent* Inv = Character->FindComponentByClass<UInventoryComponent>())
    {
        // Main inventory
        const TArray<FItemInstance>& Items = Inv->GetAllItems();
        for (int32 i = 0; i < Items.Num(); ++i)
        {
            if (Items[i].IsValid())
            {
                FSavedItemStack Stack;
                Stack.ItemId = FPrimaryAssetId("ItemDefinition", Items[i].ItemID);
                Stack.StackCount = Items[i].StackCount;
                Stack.Durability = Items[i].CurrentDurability;
                Stack.SlotIndex = i;
                OutData.InventoryItems.Add(Stack);
            }
        }

        // Quickbar
        for (int32 i = 0; i < Inv->QuickBarSlots; ++i)
        {
            int32 MappedSlot = Inv->GetQuickBarSlot(i);
            if (MappedSlot >= 0 && MappedSlot < Items.Num() && Items[MappedSlot].IsValid())
            {
                FSavedItemStack Stack;
                Stack.ItemId = FPrimaryAssetId("ItemDefinition", Items[MappedSlot].ItemID);
                Stack.StackCount = Items[MappedSlot].StackCount;
                Stack.Durability = Items[MappedSlot].CurrentDurability;
                Stack.SlotIndex = i;
                OutData.QuickbarItems.Add(Stack);
            }
        }
    }

    // Status effects
    if (UStatusEffectComponent* Effects = Character->FindComponentByClass<UStatusEffectComponent>())
    {
        for (const FStatusEffect& Effect : Effects->GetActiveEffects())
        {
            FSavedStatusEffect Saved;
            Saved.EffectName = FName(*UEnum::GetValueAsString(Effect.Type));
            Saved.Severity = Effect.Severity;
            Saved.Duration = Effect.Duration;
            Saved.ElapsedTime = 0.0f;
            OutData.StatusEffects.Add(Saved);
        }
    }

    // Injuries
    if (UMedicalComponent* Medical = Character->FindComponentByClass<UMedicalComponent>())
    {
        for (const FInjury& Injury : Medical->GetInjuries())
        {
            FSavedInjury Saved;
            Saved.InjuryType = FName(*UEnum::GetValueAsString(Injury.Type));
            Saved.Severity = Injury.Severity;
            Saved.HealProgress = Injury.HealingProgress;
            Saved.bIsTreated = Injury.bTreated;
            Saved.bIsInfected = false;
            OutData.Injuries.Add(Saved);
        }
    }
}

void USaveLoadSubsystem::CollectWorldData(FSavedWorldState& OutData)
{
    UWorld* World = GetGameInstance()->GetWorld();
    if (!World) return;

    // Day/Night cycle
    TArray<AActor*> DayNightActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADayNightCycle::StaticClass(), DayNightActors);
    if (DayNightActors.Num() > 0)
    {
        ADayNightCycle* DayNight = Cast<ADayNightCycle>(DayNightActors[0]);
        if (DayNight)
        {
            OutData.TimeOfDay = DayNight->GameTimeHours;
            OutData.DayNumber = DayNight->CurrentDay;
        }
    }

    // Weather
    TArray<AActor*> WeatherActors;
    UGameplayStatics::GetAllActorsOfClass(World, AWeatherSystem::StaticClass(), WeatherActors);
    if (WeatherActors.Num() > 0)
    {
        AWeatherSystem* Weather = Cast<AWeatherSystem>(WeatherActors[0]);
        if (Weather)
        {
            OutData.Weather.CurrentWeatherType = FName(*UEnum::GetValueAsString(Weather->CurrentWeather));
            OutData.Weather.WeatherIntensity = Weather->WeatherIntensity;
            OutData.Weather.WindSpeed = Weather->WindSpeed;
            OutData.Weather.WindDirection = FVector(
                FMath::Cos(FMath::DegreesToRadians(Weather->WindDirection)),
                FMath::Sin(FMath::DegreesToRadians(Weather->WindDirection)),
                0.0f);
        }
    }

    // Building pieces
    TArray<AActor*> BuildingActors;
    UGameplayStatics::GetAllActorsOfClass(World, ABuildingPiece::StaticClass(), BuildingActors);
    for (AActor* Actor : BuildingActors)
    {
        ABuildingPiece* Piece = Cast<ABuildingPiece>(Actor);
        if (Piece && !Piece->bIsPreview)
        {
            FSavedBuildingPiece Saved;
            Saved.Transform = Piece->GetActorTransform();
            Saved.CurrentHealth = Piece->Health;
            Saved.MaxHealth = Piece->MaxHealth;
            Saved.UniqueId = FGuid::NewGuid();
            OutData.Buildings.Add(Saved);
        }
    }

    // Game state for discovered recipes
    ABiomeSurvivorGameState* GS = World->GetGameState<ABiomeSurvivorGameState>();
    if (GS)
    {
        // Recipes and unlocks would be collected from player state
    }
}

bool USaveLoadSubsystem::ApplyLoadedData(USaveGameData* SaveData)
{
    if (!SaveData) return false;

    ApplyPlayerData(SaveData->PlayerData);
    ApplyWorldData(SaveData->WorldState);

    return true;
}

void USaveLoadSubsystem::ApplyPlayerData(const FSavedPlayerData& Data)
{
    UWorld* World = GetGameInstance()->GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    ASurvivorCharacter* Character = Cast<ASurvivorCharacter>(PC->GetPawn());
    if (!Character) return;

    // Teleport to saved position
    Character->SetActorTransform(Data.PlayerTransform);
    PC->SetControlRotation(Data.ControlRotation);

    // Restore stats
    if (UPlayerStatsComponent* Stats = Character->FindComponentByClass<UPlayerStatsComponent>())
    {
        Stats->Health = Data.Stats.Health;
        Stats->Hunger = Data.Stats.Hunger;
        Stats->Thirst = Data.Stats.Thirst;
        Stats->Stamina = Data.Stats.Stamina;
        Stats->Comfort = Data.Stats.Comfort;
    }

    // Restore temperature
    if (UTemperatureComponent* Temp = Character->FindComponentByClass<UTemperatureComponent>())
    {
        Temp->BodyTemperature = Data.Stats.BodyTemperature;
    }

    // Restore fatigue
    if (USleepComponent* Sleep = Character->FindComponentByClass<USleepComponent>())
    {
        Sleep->Fatigue = Data.Stats.Fatigue;
    }

    // Restore inventory
    if (UInventoryComponent* Inv = Character->FindComponentByClass<UInventoryComponent>())
    {
        // Clear existing items by removing from all slots
        const TArray<FItemInstance>& CurrentItems = Inv->GetAllItems();
        for (int32 i = CurrentItems.Num() - 1; i >= 0; --i)
        {
            if (CurrentItems[i].IsValid())
            {
                Inv->RemoveItemFromSlot(i);
            }
        }

        // Add saved items
        for (const FSavedItemStack& Stack : Data.InventoryItems)
        {
            Inv->AddItem(Stack.ItemId.PrimaryAssetName, Stack.StackCount);
        }

        // Restore quickbar mappings
        for (int32 i = 0; i < Data.QuickbarItems.Num(); ++i)
        {
            const FSavedItemStack& QBStack = Data.QuickbarItems[i];
            int32 InvSlot = Inv->FindItemSlot(QBStack.ItemId.PrimaryAssetName);
            if (InvSlot >= 0)
            {
                Inv->AssignToQuickBar(InvSlot, QBStack.SlotIndex);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Player data restored: Pos=(%s), HP=%.0f"),
        *Data.PlayerTransform.GetLocation().ToString(), Data.Stats.Health);
}

void USaveLoadSubsystem::ApplyWorldData(const FSavedWorldState& Data)
{
    UWorld* World = GetGameInstance()->GetWorld();
    if (!World) return;

    // Restore day/night
    TArray<AActor*> DayNightActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADayNightCycle::StaticClass(), DayNightActors);
    if (DayNightActors.Num() > 0)
    {
        ADayNightCycle* DayNight = Cast<ADayNightCycle>(DayNightActors[0]);
        if (DayNight)
        {
            DayNight->SetTimeOfDay(Data.TimeOfDay);
            DayNight->CurrentDay = Data.DayNumber;
        }
    }

    // Restore weather
    TArray<AActor*> WeatherActors;
    UGameplayStatics::GetAllActorsOfClass(World, AWeatherSystem::StaticClass(), WeatherActors);
    if (WeatherActors.Num() > 0)
    {
        AWeatherSystem* Weather = Cast<AWeatherSystem>(WeatherActors[0]);
        if (Weather)
        {
            // Convert saved weather name back to enum
            EWeatherType WeatherType = EWeatherType::Clear;
            const UEnum* WeatherEnum = StaticEnum<EWeatherType>();
            if (WeatherEnum)
            {
                int64 EnumValue = WeatherEnum->GetValueByNameString(Data.Weather.CurrentWeatherType.ToString());
                if (EnumValue != INDEX_NONE)
                {
                    WeatherType = static_cast<EWeatherType>(EnumValue);
                }
            }
            Weather->SetWeather(WeatherType, Data.Weather.WeatherIntensity);
        }
    }

    // Rebuild building pieces
    for (const FSavedBuildingPiece& PieceData : Data.Buildings)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        ABuildingPiece* Piece = World->SpawnActor<ABuildingPiece>(
            ABuildingPiece::StaticClass(), PieceData.Transform, SpawnParams);
        
        if (Piece)
        {
            Piece->Health = PieceData.CurrentHealth;
            Piece->ConfirmPlacement(TEXT(""));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("World restored: Day %d, Time %.1f, %d buildings"),
        Data.DayNumber, Data.TimeOfDay, Data.Buildings.Num());
}

// ─── Async Handlers ───

void USaveLoadSubsystem::HandleAsyncSaveComplete(const FString& SlotName, int32 UserIndex, bool bSuccess)
{
    UE_LOG(LogTemp, Log, TEXT("Async save %s: %s"), *SlotName, bSuccess ? TEXT("Success") : TEXT("Failed"));
    OnSaveCompleted.Broadcast(bSuccess);
}

void USaveLoadSubsystem::HandleAsyncLoadComplete(const FString& SlotName, int32 UserIndex, USaveGame* LoadedData)
{
    USaveGameData* SaveData = Cast<USaveGameData>(LoadedData);
    bool bSuccess = (SaveData != nullptr);

    if (bSuccess)
    {
        LastLoadedSave = SaveData;
        ApplyLoadedData(SaveData);
    }

    UE_LOG(LogTemp, Log, TEXT("Async load %s: %s"), *SlotName, bSuccess ? TEXT("Success") : TEXT("Failed"));
    OnLoadCompleted.Broadcast(bSuccess);
}
