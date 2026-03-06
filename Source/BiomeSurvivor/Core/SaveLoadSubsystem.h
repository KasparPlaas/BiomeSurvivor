// SaveLoadSubsystem.h - Manages save/load operations
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/SaveGameData.h"
#include "SaveLoadSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveCompleted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadCompleted, bool, bSuccess);

UCLASS()
class BIOMESURVIVOR_API USaveLoadSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ─── Save operations ───

    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    bool SaveGame(const FString& SlotName = TEXT("Default"), int32 UserIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    void SaveGameAsync(const FString& SlotName = TEXT("Default"), int32 UserIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    bool QuickSave();

    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    void AutoSave();

    // ─── Load operations ───

    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    bool LoadGame(const FString& SlotName = TEXT("Default"), int32 UserIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    void LoadGameAsync(const FString& SlotName = TEXT("Default"), int32 UserIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    bool QuickLoad();

    // ─── Slot management ───

    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    bool DeleteSave(const FString& SlotName, int32 UserIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    bool DoesSaveExist(const FString& SlotName, int32 UserIndex = 0) const;

    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    TArray<FString> GetAllSaveSlots() const;

    UFUNCTION(BlueprintPure, Category = "SaveLoad")
    USaveGameData* GetLastLoadedSave() const { return LastLoadedSave; }

    // ─── Data collection ───

    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    USaveGameData* CollectSaveData(const FString& SlotName, int32 UserIndex);

    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    bool ApplyLoadedData(USaveGameData* SaveData);

    // ─── Events ───

    UPROPERTY(BlueprintAssignable, Category = "SaveLoad")
    FOnSaveCompleted OnSaveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "SaveLoad")
    FOnLoadCompleted OnLoadCompleted;

    // ─── Settings ───

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveLoad")
    float AutoSaveInterval = 300.0f; // 5 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveLoad")
    int32 MaxAutoSaveSlots = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveLoad")
    bool bAutoSaveEnabled = true;

private:
    void HandleAsyncSaveComplete(const FString& SlotName, int32 UserIndex, bool bSuccess);
    void HandleAsyncLoadComplete(const FString& SlotName, int32 UserIndex, USaveGame* LoadedData);

    void CollectPlayerData(FSavedPlayerData& OutData);
    void CollectWorldData(FSavedWorldState& OutData);
    void ApplyPlayerData(const FSavedPlayerData& Data);
    void ApplyWorldData(const FSavedWorldState& Data);

    UPROPERTY()
    USaveGameData* LastLoadedSave = nullptr;

    FTimerHandle AutoSaveTimerHandle;
    int32 CurrentAutoSaveIndex = 0;
    float SessionPlayTime = 0.0f;
};
