// SaveGameData.h - Complete save/load system for BiomeSurvivor
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameData.generated.h"

// ─── Serializable structs for save data ───

USTRUCT(BlueprintType)
struct FSavedItemStack
{
    GENERATED_BODY()

    UPROPERTY(SaveGame) FPrimaryAssetId ItemId;
    UPROPERTY(SaveGame) int32 StackCount = 0;
    UPROPERTY(SaveGame) float Durability = 1.0f;
    UPROPERTY(SaveGame) int32 SlotIndex = -1;
};

USTRUCT(BlueprintType)
struct FSavedEquipment
{
    GENERATED_BODY()

    UPROPERTY(SaveGame) TMap<FName, FPrimaryAssetId> EquippedItems;
};

USTRUCT(BlueprintType)
struct FSavedPlayerStats
{
    GENERATED_BODY()

    UPROPERTY(SaveGame) float Health = 100.0f;
    UPROPERTY(SaveGame) float MaxHealth = 100.0f;
    UPROPERTY(SaveGame) float Hunger = 100.0f;
    UPROPERTY(SaveGame) float Thirst = 100.0f;
    UPROPERTY(SaveGame) float Stamina = 100.0f;
    UPROPERTY(SaveGame) float Comfort = 50.0f;
    UPROPERTY(SaveGame) float BodyTemperature = 37.0f;
    UPROPERTY(SaveGame) float Fatigue = 0.0f;
    UPROPERTY(SaveGame) float CaloriesStored = 2000.0f;
};

USTRUCT(BlueprintType)
struct FSavedStatusEffect
{
    GENERATED_BODY()

    UPROPERTY(SaveGame) FName EffectName;
    UPROPERTY(SaveGame) float Severity = 0.0f;
    UPROPERTY(SaveGame) float Duration = 0.0f;
    UPROPERTY(SaveGame) float ElapsedTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FSavedInjury
{
    GENERATED_BODY()

    UPROPERTY(SaveGame) FName InjuryType;
    UPROPERTY(SaveGame) float Severity = 0.0f;
    UPROPERTY(SaveGame) float HealProgress = 0.0f;
    UPROPERTY(SaveGame) bool bIsTreated = false;
    UPROPERTY(SaveGame) bool bIsInfected = false;
};

USTRUCT(BlueprintType)
struct FSavedBuildingPiece
{
    GENERATED_BODY()

    UPROPERTY(SaveGame) FPrimaryAssetId PieceId;
    UPROPERTY(SaveGame) FTransform Transform;
    UPROPERTY(SaveGame) float CurrentHealth = 0.0f;
    UPROPERTY(SaveGame) float MaxHealth = 0.0f;
    UPROPERTY(SaveGame) FName MaterialType;
    UPROPERTY(SaveGame) FGuid UniqueId;
};

USTRUCT(BlueprintType)
struct FSavedResourceNode
{
    GENERATED_BODY()

    UPROPERTY(SaveGame) FGuid NodeId;
    UPROPERTY(SaveGame) int32 RemainingYields = 0;
    UPROPERTY(SaveGame) float RespawnTimeRemaining = 0.0f;
    UPROPERTY(SaveGame) bool bIsDepleted = false;
};

USTRUCT(BlueprintType)
struct FSavedContainer
{
    GENERATED_BODY()

    UPROPERTY(SaveGame) FGuid ContainerId;
    UPROPERTY(SaveGame) FTransform Transform;
    UPROPERTY(SaveGame) TArray<FSavedItemStack> Contents;
    UPROPERTY(SaveGame) bool bIsLocked = false;
};

USTRUCT(BlueprintType)
struct FSavedAnimalState
{
    GENERATED_BODY()

    UPROPERTY(SaveGame) FName AnimalType;
    UPROPERTY(SaveGame) FVector Location;
    UPROPERTY(SaveGame) float Health = 0.0f;
    UPROPERTY(SaveGame) bool bIsAlive = true;
};

USTRUCT(BlueprintType)
struct FSavedWeatherState
{
    GENERATED_BODY()

    UPROPERTY(SaveGame) FName CurrentWeatherType;
    UPROPERTY(SaveGame) float WeatherIntensity = 0.0f;
    UPROPERTY(SaveGame) float TransitionProgress = 0.0f;
    UPROPERTY(SaveGame) float WindSpeed = 0.0f;
    UPROPERTY(SaveGame) FVector WindDirection;
};

USTRUCT(BlueprintType)
struct FSavedWorldState
{
    GENERATED_BODY()

    UPROPERTY(SaveGame) float TimeOfDay = 8.0f;
    UPROPERTY(SaveGame) int32 DayNumber = 1;
    UPROPERTY(SaveGame) FSavedWeatherState Weather;
    UPROPERTY(SaveGame) TArray<FSavedBuildingPiece> Buildings;
    UPROPERTY(SaveGame) TArray<FSavedResourceNode> Resources;
    UPROPERTY(SaveGame) TArray<FSavedContainer> Containers;
    UPROPERTY(SaveGame) TArray<FSavedAnimalState> Animals;
    UPROPERTY(SaveGame) TArray<FName> DiscoveredRecipes;
    UPROPERTY(SaveGame) TArray<FName> UnlockedRecipes;
};

USTRUCT(BlueprintType)
struct FSavedSkill
{
    GENERATED_BODY()

    UPROPERTY(SaveGame) FName SkillName;
    UPROPERTY(SaveGame) int32 Level = 1;
    UPROPERTY(SaveGame) float Experience = 0.0f;
};

USTRUCT(BlueprintType)
struct FSavedPlayerData
{
    GENERATED_BODY()

    UPROPERTY(SaveGame) FString PlayerName;
    UPROPERTY(SaveGame) FTransform PlayerTransform;
    UPROPERTY(SaveGame) FRotator ControlRotation;
    UPROPERTY(SaveGame) FSavedPlayerStats Stats;
    UPROPERTY(SaveGame) TArray<FSavedStatusEffect> StatusEffects;
    UPROPERTY(SaveGame) TArray<FSavedInjury> Injuries;
    UPROPERTY(SaveGame) TArray<FSavedItemStack> InventoryItems;
    UPROPERTY(SaveGame) TArray<FSavedItemStack> QuickbarItems;
    UPROPERTY(SaveGame) FSavedEquipment Equipment;
    UPROPERTY(SaveGame) TArray<FSavedSkill> Skills;
};

// ─── Main save game class ───

UCLASS()
class BIOMESURVIOR_API USaveGameData : public USaveGame
{
    GENERATED_BODY()

public:
    USaveGameData();

    // Save metadata
    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save")
    FString SaveSlotName;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save")
    int32 UserIndex = 0;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save")
    FDateTime SaveTimestamp;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save")
    FString GameVersion;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save")
    int32 SaveVersion = 1;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save")
    float TotalPlayTimeSeconds = 0.0f;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save")
    FString MapName;

    // Player data
    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save")
    FSavedPlayerData PlayerData;

    // World state
    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save")
    FSavedWorldState WorldState;

    // Settings
    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save")
    TMap<FName, float> GameSettings;
};
