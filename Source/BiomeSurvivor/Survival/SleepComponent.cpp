// Copyright Biome Survivor. All Rights Reserved.

#include "Survival/SleepComponent.h"
#include "Player/PlayerStatsComponent.h"
#include "BiomeSurvivor.h"
#include "Net/UnrealNetwork.h"

USleepComponent::USleepComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f;
	SetIsReplicatedByDefault(true);
}

void USleepComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USleepComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()->HasAuthority()) return;

	if (bIsSleeping)
	{
		// Recovery while sleeping
		Fatigue = FMath::Max(0.0f, Fatigue - SleepRecoveryRate * SleepQualityMultiplier * DeltaTime);

		// Restore comfort while sleeping
		UPlayerStatsComponent* Stats = GetOwner()->FindComponentByClass<UPlayerStatsComponent>();
		if (Stats)
		{
			Stats->RestoreComfort(2.0f * SleepQualityMultiplier * DeltaTime);
			Stats->Heal(0.5f * SleepQualityMultiplier * DeltaTime);
		}

		// Auto-wake when fully rested
		if (Fatigue <= 0.0f)
		{
			StopSleep();
		}
	}
	else
	{
		// Fatigue accumulates while awake
		Fatigue = FMath::Min(100.0f, Fatigue + FatigueRate * DeltaTime);

		// Exhaustion effects
		if (IsTired())
		{
			UPlayerStatsComponent* Stats = GetOwner()->FindComponentByClass<UPlayerStatsComponent>();
			if (Stats)
			{
				// Reduce stamina regen when tired
				float TiredMultiplier = 1.0f - ((Fatigue - FatiguePenaltyThreshold) / (100.0f - FatiguePenaltyThreshold));
				Stats->StaminaRegenRate = 8.0f * FMath::Max(0.2f, TiredMultiplier);
			}
		}

		// Exhaustion threshold
		if (IsExhausted() && !bWasExhausted)
		{
			bWasExhausted = true;
			OnExhausted.Broadcast();
			UE_LOG(LogBiomeSurvivor, Log, TEXT("Player is exhausted!"));
		}
		else if (!IsExhausted())
		{
			bWasExhausted = false;
		}
	}
}

void USleepComponent::StartSleep()
{
	if (bIsSleeping) return;

	bIsSleeping = true;
	OnFellAsleep.Broadcast();
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Player started sleeping (Quality: %.1fx)"), SleepQualityMultiplier);

	// TODO: Disable player movement/input, fade to black, enable time skip
}

void USleepComponent::StopSleep()
{
	if (!bIsSleeping) return;

	bIsSleeping = false;
	OnWokeUp.Broadcast();
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Player woke up (Fatigue: %.1f%%)"), Fatigue);

	// Restore normal stamina regen
	UPlayerStatsComponent* Stats = GetOwner()->FindComponentByClass<UPlayerStatsComponent>();
	if (Stats)
	{
		Stats->StaminaRegenRate = 8.0f;
	}

	// TODO: Re-enable player movement/input, fade from black
}

void USleepComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USleepComponent, Fatigue);
	DOREPLIFETIME(USleepComponent, bIsSleeping);
}
