// Copyright Biome Survivor. All Rights Reserved.

#include "Wildlife/AnimalBase.h"
#include "Inventory/InventoryComponent.h"
#include "BiomeSurvivor.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

AAnimalBase::AAnimalBase()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 300.0f, 0.0f);

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	SetReplicates(true);
}

void AAnimalBase::BeginPlay()
{
	Super::BeginPlay();
	SpawnLocation = GetActorLocation();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AAnimalBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAnimalBase, Health);
	DOREPLIFETIME(AAnimalBase, CurrentState);
}

void AAnimalBase::SetState(EAnimalState NewState)
{
	if (CurrentState == NewState) return;
	CurrentState = NewState;
	OnAnimalStateChanged.Broadcast(NewState);

	switch (NewState)
	{
	case EAnimalState::Wandering:
	case EAnimalState::Idle:
	case EAnimalState::Feeding:
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		break;
	case EAnimalState::Fleeing:
	case EAnimalState::Attacking:
	case EAnimalState::Stalking:
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		break;
	default:
		break;
	}
}

float AAnimalBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	OnDamaged(ActualDamage, DamageCauser);
	return ActualDamage;
}

void AAnimalBase::OnDamaged(float DamageAmount, AActor* DamageCauser)
{
	if (IsDead()) return;

	Health = FMath::Max(0.0f, Health - DamageAmount);
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Animal %s damaged: %.1f (HP: %.1f/%.1f)"),
		*AnimalName.ToString(), DamageAmount, Health, MaxHealth);

	if (Health <= 0.0f)
	{
		SetState(EAnimalState::Dying);
		OnAnimalDied.Broadcast();

		// Disable movement
		GetCharacterMovement()->DisableMovement();

		// Set corpse despawn timer
		SetLifeSpan(CorpseDespawnTime);

		UE_LOG(LogBiomeSurvivor, Log, TEXT("Animal died: %s"), *AnimalName.ToString());
	}
	else
	{
		// React to damage based on behavior type
		switch (BehaviorType)
		{
		case EAnimalBehavior::Passive:
			SetState(EAnimalState::Fleeing);
			break;
		case EAnimalBehavior::Neutral:
		case EAnimalBehavior::Territorial:
			SetState(EAnimalState::Attacking);
			break;
		case EAnimalBehavior::Aggressive:
			SetState(EAnimalState::Attacking);
			break;
		}
	}
}

TArray<FAnimalLootDrop> AAnimalBase::GenerateLoot() const
{
	TArray<FAnimalLootDrop> Result;

	for (const FAnimalLootDrop& Drop : LootTable)
	{
		if (FMath::FRand() <= Drop.DropChance)
		{
			FAnimalLootDrop LootDrop = Drop;
			LootDrop.MinCount = FMath::RandRange(Drop.MinCount, Drop.MaxCount);
			Result.Add(LootDrop);
		}
	}

	return Result;
}

void AAnimalBase::Harvest(AActor* Harvester)
{
	if (!IsDead() || bHasBeenHarvested || !bHarvestable) return;

	bHasBeenHarvested = true;

	UInventoryComponent* Inventory = Harvester->FindComponentByClass<UInventoryComponent>();
	if (Inventory)
	{
		TArray<FAnimalLootDrop> Loot = GenerateLoot();
		for (const FAnimalLootDrop& Drop : Loot)
		{
			Inventory->AddItem(Drop.ItemID, Drop.MinCount);
			UE_LOG(LogBiomeSurvivor, Log, TEXT("Harvested %d x %s from %s"),
				Drop.MinCount, *Drop.ItemID.ToString(), *AnimalName.ToString());
		}
	}

	// Destroy corpse after harvesting
	SetLifeSpan(2.0f);
}
