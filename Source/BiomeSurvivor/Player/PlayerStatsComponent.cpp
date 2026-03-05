// Copyright Biome Survivor. All Rights Reserved.

#include "Player/PlayerStatsComponent.h"
#include "BiomeSurvivor.h"
#include "Net/UnrealNetwork.h"

UPlayerStatsComponent::UPlayerStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // Update 10x per second
	SetIsReplicatedByDefault(true);
}

void UPlayerStatsComponent::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogBiomeSurvivor, Log, TEXT("PlayerStatsComponent initialized on %s"), *GetOwner()->GetName());
}

void UPlayerStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Only run stat decay on the server
	if (!GetOwner()->HasAuthority()) return;

	// ---- Hunger Drain ----
	float OldHunger = Hunger;
	Hunger = FMath::Clamp(Hunger - HungerDrainRate * DeltaTime, 0.0f, MaxHunger);
	if (Hunger != OldHunger)
	{
		OnHungerChanged.Broadcast(Hunger);
		if (Hunger <= 0.0f)
		{
			OnHungerDepleted.Broadcast();
			// Starvation damage: 2 HP/sec
			ApplyDamage(2.0f * DeltaTime);
		}
	}

	// ---- Thirst Drain ----
	float OldThirst = Thirst;
	Thirst = FMath::Clamp(Thirst - ThirstDrainRate * DeltaTime, 0.0f, MaxThirst);
	if (Thirst != OldThirst)
	{
		OnThirstChanged.Broadcast(Thirst);
		if (Thirst <= 0.0f)
		{
			OnThirstDepleted.Broadcast();
			// Dehydration damage: 3 HP/sec
			ApplyDamage(3.0f * DeltaTime);
		}
	}

	// ---- Stamina ----
	float OldStamina = Stamina;
	if (bSprinting)
	{
		Stamina = FMath::Clamp(Stamina - SprintStaminaDrain * DeltaTime, 0.0f, MaxStamina);
	}
	else
	{
		// Regenerate stamina when not sprinting
		float RegenMultiplier = 1.0f;
		if (Hunger < 20.0f) RegenMultiplier *= 0.5f;
		if (Thirst < 20.0f) RegenMultiplier *= 0.5f;
		Stamina = FMath::Clamp(Stamina + StaminaRegenRate * RegenMultiplier * DeltaTime, 0.0f, MaxStamina);
	}
	if (Stamina != OldStamina)
	{
		OnStaminaChanged.Broadcast(Stamina);
	}

	// ---- Health Regen (when well-fed and hydrated) ----
	if (Hunger > 50.0f && Thirst > 50.0f && Health < MaxHealth)
	{
		Heal(HealthRegenRate * DeltaTime);
	}

	// ---- Overencumbered penalty ----
	if (IsOverEncumbered())
	{
		// Drain stamina while overweight
		ConsumeStamina(5.0f * DeltaTime);
	}
}

void UPlayerStatsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerStatsComponent, Health);
	DOREPLIFETIME(UPlayerStatsComponent, Hunger);
	DOREPLIFETIME(UPlayerStatsComponent, Thirst);
	DOREPLIFETIME(UPlayerStatsComponent, Stamina);
	DOREPLIFETIME(UPlayerStatsComponent, Comfort);
	DOREPLIFETIME(UPlayerStatsComponent, CurrentWeight);
}

void UPlayerStatsComponent::ApplyDamage(float Amount)
{
	if (Amount <= 0.0f) return;

	Health = FMath::Clamp(Health - Amount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(Health);

	if (Health <= 0.0f)
	{
		OnHealthDepleted.Broadcast();
	}
}

void UPlayerStatsComponent::Heal(float Amount)
{
	if (Amount <= 0.0f) return;
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(Health);
}

void UPlayerStatsComponent::ConsumeStamina(float Amount)
{
	Stamina = FMath::Clamp(Stamina - Amount, 0.0f, MaxStamina);
	OnStaminaChanged.Broadcast(Stamina);
}

void UPlayerStatsComponent::RestoreHunger(float Amount)
{
	Hunger = FMath::Clamp(Hunger + Amount, 0.0f, MaxHunger);
	OnHungerChanged.Broadcast(Hunger);
}

void UPlayerStatsComponent::RestoreThirst(float Amount)
{
	Thirst = FMath::Clamp(Thirst + Amount, 0.0f, MaxThirst);
	OnThirstChanged.Broadcast(Thirst);
}

void UPlayerStatsComponent::RestoreComfort(float Amount)
{
	Comfort = FMath::Clamp(Comfort + Amount, 0.0f, MaxComfort);
}

// ---- RepNotify Callbacks ----
void UPlayerStatsComponent::OnRep_Health()  { OnHealthChanged.Broadcast(Health); }
void UPlayerStatsComponent::OnRep_Hunger()  { OnHungerChanged.Broadcast(Hunger); }
void UPlayerStatsComponent::OnRep_Thirst()  { OnThirstChanged.Broadcast(Thirst); }
void UPlayerStatsComponent::OnRep_Stamina() { OnStaminaChanged.Broadcast(Stamina); }
void UPlayerStatsComponent::OnRep_Comfort() { /* Update UI if needed */ }
