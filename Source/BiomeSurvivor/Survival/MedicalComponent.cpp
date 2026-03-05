// Copyright Biome Survivor. All Rights Reserved.

#include "Survival/MedicalComponent.h"
#include "Player/PlayerStatsComponent.h"
#include "Player/StatusEffectComponent.h"
#include "BiomeSurvivor.h"

UMedicalComponent::UMedicalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f;
}

void UMedicalComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMedicalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()->HasAuthority()) return;

	for (int32 i = ActiveInjuries.Num() - 1; i >= 0; --i)
	{
		FInjury& Injury = ActiveInjuries[i];

		if (Injury.bTreated)
		{
			// Healing progresses when treated
			Injury.HealingProgress += BaseHealingRate * DeltaTime;

			if (Injury.HealingProgress >= 100.0f)
			{
				EInjuryType Type = Injury.Type;
				ActiveInjuries.RemoveAt(i);
				OnInjuryHealed.Broadcast(Type);
				UE_LOG(LogBiomeSurvivor, Log, TEXT("Injury healed: %d"), (int32)Type);
			}
		}
		else
		{
			// Untreated injuries cause ongoing effects
			UPlayerStatsComponent* Stats = GetOwner()->FindComponentByClass<UPlayerStatsComponent>();
			UStatusEffectComponent* StatusFX = GetOwner()->FindComponentByClass<UStatusEffectComponent>();

			switch (Injury.Type)
			{
			case EInjuryType::Laceration:
				// Bleeding
				if (Stats) Stats->ApplyDamage(1.5f * DeltaTime);
				if (StatusFX) StatusFX->ApplyStatusEffect(EStatusEffectType::Bleeding, 0.1f * DeltaTime);
				break;

			case EInjuryType::Bite:
				// Risk of infection
				if (StatusFX && !StatusFX->HasStatusEffect(EStatusEffectType::Infection))
				{
					if (FMath::FRand() < 0.01f * DeltaTime) // 1% chance per second
					{
						StatusFX->ApplyStatusEffect(EStatusEffectType::Infection, 10.0f);
					}
				}
				if (Stats) Stats->ApplyDamage(1.0f * DeltaTime);
				break;

			case EInjuryType::Fracture:
				if (StatusFX) StatusFX->ApplyStatusEffect(EStatusEffectType::Fracture, 0.1f * DeltaTime);
				break;

			case EInjuryType::Burn:
				if (Stats) Stats->ApplyDamage(0.5f * DeltaTime);
				break;

			case EInjuryType::Sprain:
				// Movement penalty only, handled by StatusEffect
				break;

			default:
				break;
			}
		}
	}
}

void UMedicalComponent::ApplyInjury(EInjuryType Type, float Severity)
{
	FInjury NewInjury;
	NewInjury.Type = Type;
	NewInjury.Severity = FMath::Clamp(Severity, 0.0f, 100.0f);
	NewInjury.bTreated = false;
	NewInjury.HealingProgress = 0.0f;

	ActiveInjuries.Add(NewInjury);
	OnInjuryApplied.Broadcast(Type);
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Injury applied: %d (Severity: %.1f)"), (int32)Type, Severity);
}

bool UMedicalComponent::TreatInjury(int32 InjuryIndex)
{
	if (InjuryIndex < 0 || InjuryIndex >= ActiveInjuries.Num()) return false;

	FInjury& Injury = ActiveInjuries[InjuryIndex];
	if (Injury.bTreated) return false;

	Injury.bTreated = true;

	// Remove corresponding status effect when treated
	UStatusEffectComponent* StatusFX = GetOwner()->FindComponentByClass<UStatusEffectComponent>();
	if (StatusFX)
	{
		switch (Injury.Type)
		{
		case EInjuryType::Laceration:
		case EInjuryType::Bite:
			StatusFX->ReduceEffectSeverity(EStatusEffectType::Bleeding, 50.0f);
			break;
		case EInjuryType::Fracture:
			// Fracture stays but stops worsening
			break;
		default:
			break;
		}
	}

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Injury treated: %d"), (int32)Injury.Type);
	return true;
}

void UMedicalComponent::UseMedicalItem(FName ItemID)
{
	// TODO: Look up medical item properties and apply effects
	// Bandage -> treat laceration/bleeding
	// Splint -> treat fracture
	// Antiseptic -> treat/prevent infection
	// Painkillers -> reduce pain/discomfort
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Medical item used: %s"), *ItemID.ToString());
}

bool UMedicalComponent::IsBleeding() const
{
	for (const FInjury& Injury : ActiveInjuries)
	{
		if ((Injury.Type == EInjuryType::Laceration || Injury.Type == EInjuryType::Bite) && !Injury.bTreated)
		{
			return true;
		}
	}
	return false;
}
