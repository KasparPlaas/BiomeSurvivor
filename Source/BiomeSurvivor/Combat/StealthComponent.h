// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StealthComponent.generated.h"

/**
 * UStealthComponent
 * Manages player visibility to AI, noise generation, and detection states.
 * Crouching, darkness, and environment reduce visibility.
 * Values from plan.md Section 5.5.
 */

UENUM(BlueprintType)
enum class EDetectionLevel : uint8
{
	Hidden		UMETA(DisplayName = "Hidden"),
	Suspicious	UMETA(DisplayName = "Suspicious"),
	Detected	UMETA(DisplayName = "Detected"),
	Alert		UMETA(DisplayName = "Alert")
};

UCLASS(ClassGroup=(BiomeSurvivor), meta=(BlueprintSpawnableComponent))
class BIOMESURVIVOR_API UStealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStealthComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Current visibility (0.0 = invisible, 1.0 = fully visible) */
	UPROPERTY(BlueprintReadOnly, Category="Stealth")
	float Visibility = 1.0f;

	/** Current noise level (0.0 = silent, 1.0 = max noise) */
	UPROPERTY(BlueprintReadOnly, Category="Stealth")
	float NoiseLevel = 0.0f;

	/** Base visibility when standing still */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stealth")
	float BaseVisibility = 0.8f;

	/** Crouching visibility reduction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stealth")
	float CrouchVisibilityMod = 0.5f;

	/** Night/darkness visibility reduction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stealth")
	float DarknessVisibilityMod = 0.3f;

	/** Walking noise */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stealth")
	float WalkNoise = 0.3f;

	/** Running noise */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stealth")
	float SprintNoise = 0.8f;

	/** Crouching noise */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stealth")
	float CrouchNoise = 0.1f;

	/** Generate a one-time noise event (e.g., gunshot, breaking branches) */
	UFUNCTION(BlueprintCallable, Category="Stealth")
	void GenerateNoise(float Intensity, float Radius = 1500.0f);

	/** Get how visible this player is to a specific AI at a position */
	UFUNCTION(BlueprintPure, Category="Stealth")
	float GetVisibilityToAI(const FVector& AIPosition, float AISightRange) const;

	/** Check if player is in darkness */
	UFUNCTION(BlueprintPure, Category="Stealth")
	bool IsInDarkness() const;

	/** Check if player is in foliage/cover */
	UFUNCTION(BlueprintPure, Category="Stealth")
	bool IsInCover() const;

private:
	void UpdateVisibility();
	void UpdateNoiseLevel();
};
