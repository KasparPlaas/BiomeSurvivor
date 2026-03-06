// LootContainer.h - World container with inventory and loot table support
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/InteractableInterface.h"
#include "Inventory/InventoryComponent.h"
#include "LootContainer.generated.h"

UENUM(BlueprintType)
enum class EContainerType : uint8
{
    Chest          UMETA(DisplayName = "Chest"),
    Crate          UMETA(DisplayName = "Crate"),
    Barrel         UMETA(DisplayName = "Barrel"),
    Backpack       UMETA(DisplayName = "Backpack"),
    SupplyDrop     UMETA(DisplayName = "Supply Drop"),
    Corpse         UMETA(DisplayName = "Corpse"),
    NaturalCache   UMETA(DisplayName = "Natural Cache"),
    Custom         UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct FLootTableEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FPrimaryAssetId ItemId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MinCount = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxCount = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DropChance = 1.0f;    // 0.0 - 1.0
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MinDurability = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxDurability = 1.0f;
};

USTRUCT(BlueprintType)
struct FLootTable
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FLootTableEntry> Entries;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MinTotalItems = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxTotalItems = 5;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float QualityMultiplier = 1.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnContainerOpened, ALootContainer*, Container, AActor*, Opener);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContainerClosed, ALootContainer*, Container);

UCLASS()
class BIOMESURVIVOR_API ALootContainer : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    ALootContainer();

    virtual void BeginPlay() override;

    // ─── IInteractableInterface ───

    virtual bool Interact_Implementation(AActor* Interactor) override;
    virtual bool InteractWithOption_Implementation(AActor* Interactor, FName OptionId) override;
    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual FInteractionData GetInteractionData_Implementation() const override;
    virtual void OnFocused_Implementation(AActor* Interactor) override;
    virtual void OnUnfocused_Implementation(AActor* Interactor) override;
    virtual void OnInteractionProgress_Implementation(float Progress) override;

    // ─── Container API ───

    UFUNCTION(BlueprintCallable, Category = "Container")
    void OpenContainer(AActor* Opener);

    UFUNCTION(BlueprintCallable, Category = "Container")
    void CloseContainer();

    UFUNCTION(BlueprintCallable, Category = "Container")
    void GenerateLoot();

    UFUNCTION(BlueprintCallable, Category = "Container")
    void Lock(FName KeyId = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Container")
    bool Unlock(AActor* Unlocker);

    UFUNCTION(BlueprintPure, Category = "Container")
    bool IsOpen() const { return bIsOpen; }

    UFUNCTION(BlueprintPure, Category = "Container")
    bool IsLocked() const { return bIsLocked; }

    UFUNCTION(BlueprintPure, Category = "Container")
    bool IsEmpty() const;

    UFUNCTION(BlueprintPure, Category = "Container")
    UInventoryComponent* GetContainerInventory() const { return ContainerInventory; }

    UFUNCTION(BlueprintPure, Category = "Container")
    FGuid GetContainerId() const { return ContainerId; }

    // ─── Events ───

    UPROPERTY(BlueprintAssignable, Category = "Container")
    FOnContainerOpened OnContainerOpened;

    UPROPERTY(BlueprintAssignable, Category = "Container")
    FOnContainerClosed OnContainerClosed;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ContainerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInventoryComponent* ContainerInventory;

    // ─── Configuration ───

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
    EContainerType ContainerType = EContainerType::Chest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
    FText ContainerName = FText::FromString("Container");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
    int32 ContainerSlots = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
    bool bIsLocked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
    FName RequiredKeyId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
    bool bDestroyWhenEmpty = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
    float InteractionDistance = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
    bool bUseLootTable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container", meta = (EditCondition = "bUseLootTable"))
    FLootTable LootTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
    bool bHasBeenLooted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
    float RespawnTime = -1.0f; // -1 = no respawn

    // ─── Audio ───

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* OpenSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* CloseSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* LockedSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* UnlockSound;

private:
    bool bIsOpen = false;
    FGuid ContainerId;

    UPROPERTY()
    AActor* CurrentUser = nullptr;

    FTimerHandle RespawnTimerHandle;
    void OnRespawnTimer();
};
