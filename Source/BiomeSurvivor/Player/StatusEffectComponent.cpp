// Copyright Biome Survivor. All Rights Reserved.

#include "Player/StatusEffectComponent.h"
#include "Player/PlayerStatsComponent.h"
#include "BiomeSurvivor.h"

UStatusEffectComponent::UStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f; // Update 2x per second
}

void UStatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()->HasAuthority()) return;

	TArray<EStatusEffectType> ToRemove;

	for (auto& Pair : ActiveEffects)
	{
		FStatusEffect& Effect = Pair.Value;
		if (!Effect.bIsActive) continue;

		// Tick duration
		if (Effect.Duration > 0.0f)
		{
			Effect.Duration -= DeltaTime;
			if (Effect.Duration <= 0.0f)
			{
				ToRemove.Add(Pair.Key);
				continue;
			}
		}

		// Increase severity over time
		Effect.Severity = FMath::Clamp(Effect.Severity + Effect.ProgressionRate * DeltaTime, 0.0f, 100.0f);

		// Process damage and penalties
		ProcessEffectTick(Effect, DeltaTime);

		OnStatusEffectChanged.Broadcast(Effect.Type, Effect.Severity);
	}

	// Remove expired effects
	for (EStatusEffectType Type : ToRemove)
	{
		RemoveStatusEffect(Type);
	}
}

void UStatusEffectComponent::ApplyStatusEffect(EStatusEffectType Type, float InitialSeverity, float Duration)
{
	if (ActiveEffects.Contains(Type))
	{
		// Increase severity of existing effect
		FStatusEffect& Existing = ActiveEffects[Type];
		Existing.Severity = FMath::Clamp(Existing.Severity + InitialSeverity, 0.0f, 100.0f);
		if (Duration > 0.0f && (Existing.Duration < 0.0f || Duration > Existing.Duration))
		{
			Existing.Duration = Duration;
		}
	}
	else
	{
		FStatusEffect NewEffect = GetDefaultEffect(Type);
		NewEffect.Severity = FMath::Clamp(InitialSeverity, 0.0f, 100.0f);
		NewEffect.Duration = Duration;
		NewEffect.bIsActive = true;
		ActiveEffects.Add(Type, NewEffect);
	}

	float CurrentSeverity = ActiveEffects[Type].Severity;
	OnStatusEffectChanged.Broadcast(Type, CurrentSeverity);
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Status effect applied: %d, Severity: %.1f"), (int32)Type, CurrentSeverity);
}

void UStatusEffectComponent::RemoveStatusEffect(EStatusEffectType Type)
{
	if (ActiveEffects.Contains(Type))
	{
		ActiveEffects.Remove(Type);
		OnStatusEffectRemoved.Broadcast(Type);
		UE_LOG(LogBiomeSurvivor, Log, TEXT("Status effect removed: %d"), (int32)Type);
	}
}

void UStatusEffectComponent::ReduceEffectSeverity(EStatusEffectType Type, float Amount)
{
	if (ActiveEffects.Contains(Type))
	{
		FStatusEffect& Effect = ActiveEffects[Type];
		Effect.Severity = FMath::Clamp(Effect.Severity - Amount, 0.0f, 100.0f);

		if (Effect.Severity <= 0.0f)
		{
			RemoveStatusEffect(Type);
		}
		else
		{
			OnStatusEffectChanged.Broadcast(Type, Effect.Severity);
		}
	}
}

bool UStatusEffectComponent::HasStatusEffect(EStatusEffectType Type) const
{
	return ActiveEffects.Contains(Type) && ActiveEffects[Type].bIsActive;
}

float UStatusEffectComponent::GetEffectSeverity(EStatusEffectType Type) const
{
	if (const FStatusEffect* Effect = ActiveEffects.Find(Type))
	{
		return Effect->Severity;
	}
	return 0.0f;
}

float UStatusEffectComponent::GetCombinedSpeedMultiplier() const
{
	float Multiplier = 1.0f;

	for (const auto& Pair : ActiveEffects)
	{
		if (!Pair.Value.bIsActive) continue;

		// Scale penalty by severity (0-100%)
		float SeverityFraction = Pair.Value.Severity / 100.0f;
		float Penalty = FMath::Lerp(1.0f, Pair.Value.SpeedPenaltyMultiplier, SeverityFraction);
		Multiplier *= Penalty;
	}

	return FMath::Clamp(Multiplier, 0.1f, 1.0f);
}

TArray<FStatusEffect> UStatusEffectComponent::GetActiveEffects() const
{
	TArray<FStatusEffect> Result;
	for (const auto& Pair : ActiveEffects)
	{
		if (Pair.Value.bIsActive)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

void UStatusEffectComponent::ClearAllEffects()
{
	for (const auto& Pair : ActiveEffects)
	{
		OnStatusEffectRemoved.Broadcast(Pair.Key);
	}
	ActiveEffects.Empty();
}

FStatusEffect UStatusEffectComponent::GetDefaultEffect(EStatusEffectType Type) const
{
	FStatusEffect Effect;
	Effect.Type = Type;

	switch (Type)
	{
	case EStatusEffectType::Hypothermia:
		Effect.ProgressionRate = 2.0f;
		Effect.MaxDamagePerSecond = 3.0f;
		Effect.SpeedPenaltyMultiplier = 0.4f;
		break;

	case EStatusEffectType::Hyperthermia:
		Effect.ProgressionRate = 1.5f;
		Effect.MaxDamagePerSecond = 2.5f;
		Effect.SpeedPenaltyMultiplier = 0.6f;
		break;

	case EStatusEffectType::Bleeding:
		Effect.ProgressionRate = 0.0f; // Doesn't progress, bleeds at constant rate
		Effect.MaxDamagePerSecond = 5.0f;
		Effect.SpeedPenaltyMultiplier = 0.8f;
		break;

	case EStatusEffectType::Fracture:
		Effect.ProgressionRate = 0.0f;
		Effect.MaxDamagePerSecond = 0.5f;
		Effect.SpeedPenaltyMultiplier = 0.3f; // Major movement penalty
		break;

	case EStatusEffectType::Poisoning:
		Effect.ProgressionRate = 1.0f;
		Effect.MaxDamagePerSecond = 2.0f;
		Effect.SpeedPenaltyMultiplier = 0.7f;
		Effect.Duration = 120.0f; // 2 minutes by default
		break;

	case EStatusEffectType::Infection:
		Effect.ProgressionRate = 0.5f;
		Effect.MaxDamagePerSecond = 1.5f;
		Effect.SpeedPenaltyMultiplier = 0.6f;
		break;

	case EStatusEffectType::Starvation:
		Effect.ProgressionRate = 0.3f;
		Effect.MaxDamagePerSecond = 2.0f;
		Effect.SpeedPenaltyMultiplier = 0.5f;
		break;

	case EStatusEffectType::Dehydration:
		Effect.ProgressionRate = 0.5f;
		Effect.MaxDamagePerSecond = 3.0f;
		Effect.SpeedPenaltyMultiplier = 0.5f;
		break;

	default:
		break;
	}

	return Effect;
}

void UStatusEffectComponent::ProcessEffectTick(FStatusEffect& Effect, float DeltaTime)
{
	// Calculate damage based on severity
	float SeverityFraction = Effect.Severity / 100.0f;
	float Damage = Effect.MaxDamagePerSecond * SeverityFraction * DeltaTime;

	if (Damage > 0.0f)
	{
		// Apply damage through PlayerStatsComponent
		if (UPlayerStatsComponent* Stats = GetOwner()->FindComponentByClass<UPlayerStatsComponent>())
		{
			Stats->ApplyDamage(Damage);
		}
	}
}
