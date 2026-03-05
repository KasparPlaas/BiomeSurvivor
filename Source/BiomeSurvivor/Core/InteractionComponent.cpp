// InteractionComponent.cpp - Interaction detection and management
#include "Core/InteractionComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UInteractionComponent::UInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Periodic interaction check
    TimeSinceLastCheck += DeltaTime;
    if (TimeSinceLastCheck >= InteractionCheckFrequency)
    {
        PerformInteractionCheck();
        TimeSinceLastCheck = 0.0f;
    }

    // Update hold interaction
    if (bIsInteracting && CurrentInteractable)
    {
        UpdateHoldInteraction(DeltaTime);
    }
}

void UInteractionComponent::PerformInteractionCheck()
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    if (!PC) return;

    // Get camera view point
    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector TraceStart = CameraLocation;
    FVector TraceEnd = CameraLocation + CameraRotation.Vector() * InteractionRange;

    // Sphere trace for interaction
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = false;

    bool bHit = GetWorld()->SweepSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        FQuat::Identity,
        InteractionChannel,
        FCollisionShape::MakeSphere(InteractionRadius),
        QueryParams
    );

    if (bHit && HitResult.GetActor())
    {
        AActor* HitActor = HitResult.GetActor();

        // Check if the hit actor implements the interactable interface
        if (HitActor->Implements<UInteractableInterface>())
        {
            // Check if we can interact
            bool bCanInteract = IInteractableInterface::Execute_CanInteract(HitActor, GetOwner());
            
            if (bCanInteract)
            {
                if (HitActor != CurrentInteractable)
                {
                    SetCurrentInteractable(HitActor);
                }
                return;
            }
        }
    }

    // Nothing valid found
    if (CurrentInteractable)
    {
        ClearCurrentInteractable();
    }
}

void UInteractionComponent::SetCurrentInteractable(AActor* NewInteractable)
{
    // Unfocus old
    if (CurrentInteractable)
    {
        IInteractableInterface::Execute_OnUnfocused(CurrentInteractable, GetOwner());
    }

    CurrentInteractable = NewInteractable;

    if (CurrentInteractable)
    {
        CurrentInteractionData = IInteractableInterface::Execute_GetInteractionData(CurrentInteractable);
        IInteractableInterface::Execute_OnFocused(CurrentInteractable, GetOwner());
        OnInteractableFound.Broadcast(CurrentInteractable);
    }
}

void UInteractionComponent::ClearCurrentInteractable()
{
    if (CurrentInteractable)
    {
        IInteractableInterface::Execute_OnUnfocused(CurrentInteractable, GetOwner());
    }

    CurrentInteractable = nullptr;
    CurrentInteractionData = FInteractionData();
    bIsInteracting = false;
    HoldTimer = 0.0f;
    OnInteractableLost.Broadcast();
}

void UInteractionComponent::BeginInteract()
{
    if (!CurrentInteractable) return;

    bIsInteracting = true;
    HoldTimer = 0.0f;

    // For simple interactions, execute immediately
    if (CurrentInteractionData.InteractionType == EInteractionType::Simple ||
        CurrentInteractionData.InteractionType == EInteractionType::Toggle)
    {
        IInteractableInterface::Execute_Interact(CurrentInteractable, GetOwner());
        bIsInteracting = false;
    }
}

void UInteractionComponent::EndInteract()
{
    if (bIsInteracting && CurrentInteractable)
    {
        // For continuous interactions, stop on release
        if (CurrentInteractionData.InteractionType == EInteractionType::Continuous)
        {
            // End continuous interaction
        }
    }

    bIsInteracting = false;
    HoldTimer = 0.0f;
}

void UInteractionComponent::InteractOption(FName OptionId)
{
    if (!CurrentInteractable) return;

    IInteractableInterface::Execute_InteractWithOption(CurrentInteractable, GetOwner(), OptionId);
}

void UInteractionComponent::UpdateHoldInteraction(float DeltaTime)
{
    if (CurrentInteractionData.InteractionType != EInteractionType::Hold) return;

    HoldTimer += DeltaTime;
    float Progress = FMath::Clamp(HoldTimer / CurrentInteractionData.HoldDuration, 0.0f, 1.0f);

    // Notify the interactable of progress
    IInteractableInterface::Execute_OnInteractionProgress(CurrentInteractable, Progress);
    OnHoldProgressChanged.Broadcast(Progress);

    // Complete hold interaction
    if (HoldTimer >= CurrentInteractionData.HoldDuration)
    {
        IInteractableInterface::Execute_Interact(CurrentInteractable, GetOwner());
        bIsInteracting = false;
        HoldTimer = 0.0f;
    }
}

float UInteractionComponent::GetHoldProgress() const
{
    if (!bIsInteracting || CurrentInteractionData.HoldDuration <= 0.0f) return 0.0f;
    return FMath::Clamp(HoldTimer / CurrentInteractionData.HoldDuration, 0.0f, 1.0f);
}
