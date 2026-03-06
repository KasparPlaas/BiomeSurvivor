// LootContainer.cpp - World container implementation
#include "World/LootContainer.h"
#include "Inventory/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

ALootContainer::ALootContainer()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    ContainerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ContainerMesh"));
    RootComponent = ContainerMesh;
    ContainerMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    // Set a visible mesh - cube for crate/chest appearance
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeFinder.Succeeded())
    {
        ContainerMesh->SetStaticMesh(CubeFinder.Object);
    }
    ContainerMesh->SetRelativeScale3D(FVector(0.8f, 0.6f, 0.5f));
    ContainerMesh->CastShadow = true;

    ContainerInventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("ContainerInventory"));

    ContainerId = FGuid::NewGuid();
}

void ALootContainer::BeginPlay()
{
    Super::BeginPlay();

    ContainerInventory->MaxSlots = ContainerSlots;

    // Apply colored material based on container type
    if (ContainerMesh)
    {
        UMaterialInterface* BaseMat = ContainerMesh->GetMaterial(0);
        if (BaseMat)
        {
            UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
            FLinearColor Color;
            switch (ContainerType)
            {
            case EContainerType::Chest:       Color = FLinearColor(0.55f, 0.35f, 0.12f); break; // Wood brown
            case EContainerType::Crate:        Color = FLinearColor(0.45f, 0.3f, 0.15f); break;  // Darker wood
            case EContainerType::Barrel:       Color = FLinearColor(0.35f, 0.25f, 0.1f); break;   // Dark wood
            case EContainerType::SupplyDrop:   Color = FLinearColor(0.2f, 0.5f, 0.2f); break;     // Military green
            case EContainerType::Corpse:       Color = FLinearColor(0.4f, 0.3f, 0.25f); break;    // Dirty
            default:                           Color = FLinearColor(0.5f, 0.35f, 0.15f); break;
            }
            DynMat->SetVectorParameterValue(TEXT("BaseColor"), Color);
            ContainerMesh->SetMaterial(0, DynMat);
        }
    }

    if (bUseLootTable && !bHasBeenLooted)
    {
        GenerateLoot();
    }
}

// ─── IInteractableInterface ───────────────────────────────────────────

bool ALootContainer::Interact_Implementation(AActor* Interactor)
{
    if (!Interactor) return false;

    if (bIsLocked)
    {
        const bool bUnlocked = Unlock(Interactor);
        if (!bUnlocked)
        {
            if (LockedSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, LockedSound, GetActorLocation());
            }
            return false;
        }
    }

    if (bIsOpen)
    {
        CloseContainer();
    }
    else
    {
        OpenContainer(Interactor);
    }

    return true;
}

bool ALootContainer::InteractWithOption_Implementation(AActor* Interactor, FName OptionId)
{
    if (OptionId == "Open" || OptionId == "Close")
    {
        return Interact_Implementation(Interactor);
    }
    if (OptionId == "TakeAll")
    {
        if (bIsLocked) return false;
        // TODO: Transfer all items to interactor's inventory
        return true;
    }
    return false;
}

bool ALootContainer::CanInteract_Implementation(AActor* Interactor) const
{
    if (!Interactor) return false;

    const float Distance = FVector::Dist(GetActorLocation(), Interactor->GetActorLocation());
    if (Distance > InteractionDistance) return false;

    // Allow interaction if unlocked, or locked (to show locked UI)
    return true;
}

FInteractionData ALootContainer::GetInteractionData_Implementation() const
{
    FInteractionData Data;
    Data.InteractionText = ContainerName;

    if (bIsLocked)
    {
        Data.InteractionType = EInteractionType::Simple;
        Data.SubText = FText::FromString("Locked");

        FInteractionOption UnlockOption;
        UnlockOption.ActionId = "Unlock";
        UnlockOption.OptionText = FText::FromString("Unlock");
        Data.Options.Add(UnlockOption);
    }
    else if (bIsOpen)
    {
        Data.InteractionType = EInteractionType::Simple;
        Data.SubText = FText::FromString("Close");
    }
    else
    {
        Data.InteractionType = EInteractionType::Simple;
        Data.SubText = FText::FromString("Open");

        FInteractionOption TakeAllOption;
        TakeAllOption.ActionId = "TakeAll";
        TakeAllOption.OptionText = FText::FromString("Take All");
        Data.Options.Add(TakeAllOption);
    }

    return Data;
}

void ALootContainer::OnFocused_Implementation(AActor* Interactor)
{
    if (ContainerMesh)
    {
        ContainerMesh->SetRenderCustomDepth(true);
        ContainerMesh->SetCustomDepthStencilValue(1);
    }
}

void ALootContainer::OnUnfocused_Implementation(AActor* Interactor)
{
    if (ContainerMesh)
    {
        ContainerMesh->SetRenderCustomDepth(false);
    }
}

void ALootContainer::OnInteractionProgress_Implementation(float Progress)
{
    // No hold interaction needed for containers
}

// ─── Container API ────────────────────────────────────────────────────

void ALootContainer::OpenContainer(AActor* Opener)
{
    if (bIsOpen || bIsLocked) return;

    bIsOpen = true;
    CurrentUser = Opener;

    if (OpenSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, OpenSound, GetActorLocation());
    }

    OnContainerOpened.Broadcast(this, Opener);
}

void ALootContainer::CloseContainer()
{
    if (!bIsOpen) return;

    bIsOpen = false;

    if (CloseSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, CloseSound, GetActorLocation());
    }

    OnContainerClosed.Broadcast(this);

    // Check if empty and should be destroyed
    if (bDestroyWhenEmpty && IsEmpty())
    {
        if (RespawnTime > 0.0f)
        {
            SetActorHiddenInGame(true);
            SetActorEnableCollision(false);
            GetWorldTimerManager().SetTimer(
                RespawnTimerHandle, this, &ALootContainer::OnRespawnTimer, RespawnTime, false
            );
        }
        else
        {
            Destroy();
        }
    }

    CurrentUser = nullptr;
}

void ALootContainer::GenerateLoot()
{
    if (!bUseLootTable || bHasBeenLooted) return;
    if (!ContainerInventory) return;

    const int32 TotalItems = FMath::RandRange(LootTable.MinTotalItems, LootTable.MaxTotalItems);
    int32 ItemsGenerated = 0;

    TArray<FLootTableEntry> ShuffledEntries = LootTable.Entries;
    for (int32 i = ShuffledEntries.Num() - 1; i > 0; i--)
    {
        const int32 j = FMath::RandRange(0, i);
        ShuffledEntries.Swap(i, j);
    }

    for (const FLootTableEntry& Entry : ShuffledEntries)
    {
        if (ItemsGenerated >= TotalItems) break;

        const float Roll = FMath::FRand();
        if (Roll > Entry.DropChance) continue;

        const int32 Count = FMath::RandRange(Entry.MinCount, Entry.MaxCount);
        const int32 ActualCount = FMath::Min(Count, TotalItems - ItemsGenerated);

        // Add item to container inventory
        // The actual AddItem logic depends on InventoryComponent implementation
        // For now we'll use a simplified approach
        // Add item to container using the InventoryComponent API
        ContainerInventory->AddItem(Entry.ItemId.PrimaryAssetName, ActualCount);
        ItemsGenerated += ActualCount;
    }

    bHasBeenLooted = true;
}

void ALootContainer::Lock(FName KeyId)
{
    bIsLocked = true;
    RequiredKeyId = KeyId;

    if (bIsOpen)
    {
        CloseContainer();
    }
}

bool ALootContainer::Unlock(AActor* Unlocker)
{
    if (!bIsLocked) return true;

    if (RequiredKeyId == NAME_None)
    {
        // No key required, just unlock
        bIsLocked = false;
        if (UnlockSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, UnlockSound, GetActorLocation());
        }
        return true;
    }

    // Check if the unlocker has the required key in their inventory
    if (Unlocker)
    {
        UInventoryComponent* InvComp = Unlocker->FindComponentByClass<UInventoryComponent>();
        if (InvComp && InvComp->HasItem(RequiredKeyId))
        {
            bIsLocked = false;
            if (UnlockSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, UnlockSound, GetActorLocation());
            }
            return true;
        }
    }

    return false;
}

bool ALootContainer::IsEmpty() const
{
    if (!ContainerInventory) return true;
    return ContainerInventory->GetUsedSlotCount() == 0;
}

void ALootContainer::OnRespawnTimer()
{
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);

    bHasBeenLooted = false;
    bIsOpen = false;
    bIsLocked = false;

    if (bUseLootTable)
    {
        GenerateLoot();
    }
}
