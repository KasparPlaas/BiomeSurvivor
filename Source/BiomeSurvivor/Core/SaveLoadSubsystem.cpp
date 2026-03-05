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
        OutData.Stats.MaxHealth = Stats->GetMaxHealth();
        OutData.Stats.Hunger = Stats->GetHunger();
        OutData.Stats.Thirst = Stats->GetThirst();
        OutData.Stats.Stamina = Stats->GetStamina();
        OutData.Stats.Comfort = Stats->GetComfort();
    }

    // Temperature
    if (UTemperatureComponent* Temp = Character->FindComponentByClass<UTemperatureComponent>())
    {
        OutData.Stats.BodyTemperature = Temp->GetBodyTemperature();
    }

    // Sleep
    if (USleepComponent* Sleep = Character->FindComponentByClass<USleepComponent>())
    {
        OutData.Stats.Fatigue = Sleep->GetFatigue();
    }

    // Inventory
    if (UInventoryComponent* Inv = Character->FindComponentByClass<UInventoryComponent>())
    {
        // Main inventory
        const TArray<FInventorySlot>& Slots = Inv->GetInventorySlots();
        for (int32 i = 0; i < Slots.Num(); ++i)
        {
            if (Slots[i].ItemDefinition)
            {
                FSavedItemStack Stack;
                Stack.ItemId = Slots[i].ItemDefinition->GetPrimaryAssetId();
                Stack.StackCount = Slots[i].StackCount;
                Stack.Durability = Slots[i].Durability;
                Stack.SlotIndex = i;
                OutData.InventoryItems.Add(Stack);
            }
        }

        // Quickbar
        const TArray<FInventorySlot>& Quickbar = Inv->GetQuickbarSlots();
        for (int32 i = 0; i < Quickbar.Num(); ++i)
        {
            if (Quickbar[i].ItemDefinition)
            {
                FSavedItemStack Stack;
                Stack.ItemId = Quickbar[i].ItemDefinition->GetPrimaryAssetId();
                Stack.StackCount = Quickbar[i].StackCount;
                Stack.Durability = Quickbar[i].Durability;
                Stack.SlotIndex = i;
                OutData.QuickbarItems.Add(Stack);
            }
        }
    }

    // Status effects
    if (UStatusEffectComponent* Effects = Character->FindComponentByClass<UStatusEffectComponent>())
    {
        for (const FActiveStatusEffect& Effect : Effects->GetActiveEffects())
        {
            FSavedStatusEffect Saved;
            Saved.EffectName = Effect.EffectName;
            Saved.Severity = Effect.Severity;
            Saved.Duration = Effect.Duration;
            Saved.ElapsedTime = Effect.ElapsedTime;
            OutData.StatusEffects.Add(Saved);
        }
    }

    // Injuries
    if (UMedicalComponent* Medical = Character->FindComponentByClass<UMedicalComponent>())
    {
        for (const FActiveInjury& Injury : Medical->GetActiveInjuries())
        {
            FSavedInjury Saved;
            Saved.InjuryType = Injury.InjuryName;
            Saved.Severity = Injury.Severity;
            Saved.HealProgress = Injury.HealProgress;
            Saved.bIsTreated = Injury.bIsTreated;
            Saved.bIsInfected = Injury.bIsInfected;
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
            OutData.TimeOfDay = DayNight->GetCurrentHour();
            OutData.DayNumber = DayNight->GetCurrentDay();
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
            OutData.Weather.CurrentWeatherType = Weather->GetCurrentWeatherName();
            OutData.Weather.WeatherIntensity = Weather->GetWeatherIntensity();
            OutData.Weather.WindSpeed = Weather->GetWindSpeed();
            OutData.Weather.WindDirection = Weather->GetWindDirection();
        }
    }

    // Building pieces
    TArray<AActor*> BuildingActors;
    UGameplayStatics::GetAllActorsOfClass(World, ABuildingPiece::StaticClass(), BuildingActors);
    for (AActor* Actor : BuildingActors)
    {
        ABuildingPiece* Piece = Cast<ABuildingPiece>(Actor);
        if (Piece && Piece->IsPlaced())
        {
            FSavedBuildingPiece Saved;
            Saved.Transform = Piece->GetActorTransform();
            Saved.CurrentHealth = Piece->GetCurrentHealth();
            Saved.MaxHealth = Piece->GetMaxHealth();
            Saved.UniqueId = Piece->GetUniqueId();
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
        Stats->SetHealth(Data.Stats.Health);
        Stats->SetHunger(Data.Stats.Hunger);
        Stats->SetThirst(Data.Stats.Thirst);
        Stats->SetStamina(Data.Stats.Stamina);
        Stats->SetComfort(Data.Stats.Comfort);
    }

    // Restore temperature
    if (UTemperatureComponent* Temp = Character->FindComponentByClass<UTemperatureComponent>())
    {
        Temp->SetBodyTemperature(Data.Stats.BodyTemperature);
    }

    // Restore fatigue
    if (USleepComponent* Sleep = Character->FindComponentByClass<USleepComponent>())
    {
        Sleep->SetFatigue(Data.Stats.Fatigue);
    }

    // Restore inventory (clear first, then add items via AssetManager)
    if (UInventoryComponent* Inv = Character->FindComponentByClass<UInventoryComponent>())
    {
        Inv->ClearInventory();
        
        for (const FSavedItemStack& Stack : Data.InventoryItems)
        {
            Inv->AddItemByAssetId(Stack.ItemId, Stack.StackCount, Stack.SlotIndex);
        }

        for (const FSavedItemStack& Stack : Data.QuickbarItems)
        {
            Inv->AddItemToQuickbar(Stack.ItemId, Stack.StackCount, Stack.SlotIndex);
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
            DayNight->SetDay(Data.DayNumber);
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
            Weather->ForceWeather(Data.Weather.CurrentWeatherType, Data.Weather.WeatherIntensity);
        }
    }

    // Rebuild building pieces
    for (const FSavedBuildingPiece& PieceData : Data.Buildings)
    {
        // Spawn building piece from asset ID and set transform
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        ABuildingPiece* Piece = World->SpawnActor<ABuildingPiece>(
            ABuildingPiece::StaticClass(), PieceData.Transform, SpawnParams);
        
        if (Piece)
        {
            Piece->SetCurrentHealth(PieceData.CurrentHealth);
            Piece->FinalizePlace();
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
