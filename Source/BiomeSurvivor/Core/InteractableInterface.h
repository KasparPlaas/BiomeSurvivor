// InteractableInterface.h - Interface for all interactable objects
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
    Simple       UMETA(DisplayName = "Simple"),       // Single press
    Hold         UMETA(DisplayName = "Hold"),          // Hold for duration
    Toggle       UMETA(DisplayName = "Toggle"),        // Toggle on/off
    Continuous   UMETA(DisplayName = "Continuous"),     // While held
    MultiOption  UMETA(DisplayName = "Multi Option")   // Shows radial menu
};

USTRUCT(BlueprintType)
struct FInteractionOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText OptionText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ActionId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Icon = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
};

USTRUCT(BlueprintType)
struct FInteractionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText InteractionText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText SubText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EInteractionType InteractionType = EInteractionType::Simple;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HoldDuration = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxInteractionDistance = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FInteractionOption> Options;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRequiresLineOfSight = true;
};

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableInterface : public UInterface
{
    GENERATED_BODY()
};

class BIOMESURVIVOR_API IInteractableInterface
{
    GENERATED_BODY()

public:
    // Perform the interaction
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    bool Interact(AActor* Interactor);

    // Perform a specific option interaction
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    bool InteractWithOption(AActor* Interactor, FName OptionId);

    // Can this actor be interacted with right now?
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    bool CanInteract(AActor* Interactor) const;

    // Get the interaction display data
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    FInteractionData GetInteractionData() const;

    // Called when the player starts looking at this object
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnFocused(AActor* Interactor);

    // Called when the player stops looking at this object
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnUnfocused(AActor* Interactor);

    // Called during hold interactions for progress feedback
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnInteractionProgress(float Progress);
};
