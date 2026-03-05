// Copyright Biome Survivor. All Rights Reserved.

#include "Survival/NutritionComponent.h"
#include "Player/PlayerStatsComponent.h"
#include "Player/StatusEffectComponent.h"
#include "BiomeSurvivor.h"

UNutritionComponent::UNutritionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f;
}

void UNutritionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UNutritionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()->HasAuthority()) return;

	// Burn calories over time
	float BurnRate = BaseCalorieBurnRate;

	// Sprint burns more calories
	UPlayerStatsComponent* Stats = GetOwner()->FindComponentByClass<UPlayerStatsComponent>();
	if (Stats)
	{
		// Adjust burn rate based on activity
		float Speed = GetOwner()->GetVelocity().Size();
		if (Speed > 600.0f) BurnRate *= 2.0f;
		else if (Speed > 200.0f) BurnRate *= 1.3f;
	}

	CurrentCalories = FMath::Max(0.0f, CurrentCalories - BurnRate * DeltaTime);

	// Malnourishment effects
	if (IsMalnourished() && Stats)
	{
		// Reduce max stamina when malnourished
		Stats->MaxStamina = FMath::Lerp(50.0f, 100.0f, GetCaloriePercent() / 0.3f);
	}
	else if (Stats)
	{
		Stats->MaxStamina = 100.0f;
	}
}

void UNutritionComponent::ConsumeFood(float Calories, float HungerRestore, float ThirstRestore, EFoodQuality Quality, float PoisonChance)
{
	float CalorieMultiplier = 1.0f;

	switch (Quality)
	{
	case EFoodQuality::Raw:
		CalorieMultiplier = 0.7f;
		break;
	case EFoodQuality::Cooked:
		CalorieMultiplier = CookedFoodBonus;
		break;
	case EFoodQuality::WellCooked:
		CalorieMultiplier = CookedFoodBonus * 1.2f;
		break;
	case EFoodQuality::Burned:
		CalorieMultiplier = 0.5f;
		break;
	case EFoodQuality::Spoiled:
		CalorieMultiplier = 0.3f;
		PoisonChance = FMath::Max(PoisonChance, 0.5f);
		break;
	}

	CurrentCalories += Calories * CalorieMultiplier;

	// Restore hunger/thirst via PlayerStatsComponent
	UPlayerStatsComponent* Stats = GetOwner()->FindComponentByClass<UPlayerStatsComponent>();
	if (Stats)
	{
		Stats->RestoreHunger(HungerRestore * CalorieMultiplier);
		Stats->RestoreThirst(ThirstRestore);
	}

	// Food poisoning check
	if (PoisonChance > 0.0f)
	{
		float Roll = FMath::FRand();
		if (Roll < PoisonChance)
		{
			UStatusEffectComponent* StatusFX = GetOwner()->FindComponentByClass<UStatusEffectComponent>();
			if (StatusFX)
			{
				StatusFX->ApplyStatusEffect(EStatusEffectType::Poisoning, 30.0f, 60.0f);
				UE_LOG(LogBiomeSurvivor, Log, TEXT("Food poisoning! Chance was %.0f%%"), PoisonChance * 100.f);
			}
		}
	}
}
