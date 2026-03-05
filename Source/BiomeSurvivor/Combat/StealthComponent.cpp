// Copyright Biome Survivor. All Rights Reserved.

#include "Combat/StealthComponent.h"
#include "BiomeSurvivor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UStealthComponent::UStealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.2f; // 5x per second
}

void UStealthComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateVisibility();
	UpdateNoiseLevel();
}

void UStealthComponent::UpdateVisibility()
{
	Visibility = BaseVisibility;

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;

	// Crouching reduces visibility
	if (Character->bIsCrouched)
	{
		Visibility *= CrouchVisibilityMod;
	}

	// Darkness reduces visibility
	if (IsInDarkness())
	{
		Visibility *= DarknessVisibilityMod;
	}

	// Cover (foliage) reduces visibility
	if (IsInCover())
	{
		Visibility *= 0.4f;
	}

	// Moving increases visibility
	float Speed = Character->GetVelocity().Size();
	if (Speed > 10.0f)
	{
		float SpeedFactor = FMath::Clamp(Speed / 650.0f, 0.0f, 1.0f); // Normalized to sprint speed
		Visibility = FMath::Lerp(Visibility, 1.0f, SpeedFactor * 0.3f);
	}

	Visibility = FMath::Clamp(Visibility, 0.0f, 1.0f);
}

void UStealthComponent::UpdateNoiseLevel()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;

	float Speed = Character->GetVelocity().Size();

	if (Speed < 5.0f)
	{
		NoiseLevel = 0.0f; // Standing still
	}
	else if (Character->bIsCrouched)
	{
		NoiseLevel = CrouchNoise;
	}
	else if (Speed > 600.0f) // Sprinting
	{
		NoiseLevel = SprintNoise;
	}
	else
	{
		NoiseLevel = WalkNoise;
	}
}

void UStealthComponent::GenerateNoise(float Intensity, float Radius)
{
	NoiseLevel = FMath::Max(NoiseLevel, Intensity);
	
	// Report noise event to AI perception system
	// UAISense_Hearing will pick this up
	MakeNoise(Intensity, Cast<APawn>(GetOwner()), GetOwner()->GetActorLocation(), Radius);

	UE_LOG(LogBiomeSurvivor, Verbose, TEXT("Noise generated: %.2f intensity, %.0f radius"), Intensity, Radius);
}

float UStealthComponent::GetVisibilityToAI(const FVector& AIPosition, float AISightRange) const
{
	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), AIPosition);
	if (Distance > AISightRange) return 0.0f;

	// Distance falloff
	float DistanceFactor = 1.0f - (Distance / AISightRange);

	return Visibility * DistanceFactor;
}

bool UStealthComponent::IsInDarkness() const
{
	// TODO: Check actual lighting level at player location
	// For now, use time-of-day as a proxy
	// This should be connected to the DayNightCycle/GameState
	return false;
}

bool UStealthComponent::IsInCover() const
{
	// TODO: Check if player is inside foliage volumes or near bush actors
	// Use overlap check with "Cover" collision channel
	return false;
}
