// InteractionComponent.h - Component for detecting and managing interactions
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableInterface.h"
#include "InteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableFound, AActor*, InteractableActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractableLost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHoldProgress, float, Progress);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BIOMESURVIOR_API UInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInteractionComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ─── Interaction controls ───

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void BeginInteract();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void EndInteract();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void InteractOption(FName OptionId);

    // ─── Getters ───

    UFUNCTION(BlueprintPure, Category = "Interaction")
    AActor* GetCurrentInteractable() const { return CurrentInteractable; }

    UFUNCTION(BlueprintPure, Category = "Interaction")
    FInteractionData GetCurrentInteractionData() const { return CurrentInteractionData; }

    UFUNCTION(BlueprintPure, Category = "Interaction")
    bool IsInteracting() const { return bIsInteracting; }

    UFUNCTION(BlueprintPure, Category = "Interaction")
    float GetHoldProgress() const;

    // ─── Events ───

    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnInteractableFound OnInteractableFound;

    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnInteractableLost OnInteractableLost;

    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnHoldProgress OnHoldProgressChanged;

    // ─── Settings ───

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionCheckFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRadius = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TEnumAsByte<ECollisionChannel> InteractionChannel = ECC_Visibility;

protected:
    virtual void BeginPlay() override;

private:
    void PerformInteractionCheck();
    void UpdateHoldInteraction(float DeltaTime);
    void SetCurrentInteractable(AActor* NewInteractable);
    void ClearCurrentInteractable();

    UPROPERTY()
    AActor* CurrentInteractable = nullptr;

    FInteractionData CurrentInteractionData;

    bool bIsInteracting = false;
    float HoldTimer = 0.0f;
    float TimeSinceLastCheck = 0.0f;
};
