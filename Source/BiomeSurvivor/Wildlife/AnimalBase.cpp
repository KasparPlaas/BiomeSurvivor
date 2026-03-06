// Copyright Biome Survivor. All Rights Reserved.

#include "Wildlife/AnimalBase.h"
#include "Inventory/InventoryComponent.h"
#include "BiomeSurvivor.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AAnimalBase::AAnimalBase()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 300.0f, 0.0f);

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	SetReplicates(true);

	// Visible body mesh
	AnimalBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AnimalBody"));
	AnimalBodyMesh->SetupAttachment(GetCapsuleComponent());
	AnimalBodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -20.0f));
	AnimalBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AnimalBodyMesh->CastShadow = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded())
	{
		AnimalBodyMesh->SetStaticMesh(SphereFinder.Object);
	}

	// Head mesh
	AnimalHeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AnimalHead"));
	AnimalHeadMesh->SetupAttachment(GetCapsuleComponent());
	AnimalHeadMesh->SetRelativeLocation(FVector(40.0f, 0.0f, 10.0f));
	AnimalHeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AnimalHeadMesh->CastShadow = true;

	if (SphereFinder.Succeeded())
	{
		AnimalHeadMesh->SetStaticMesh(SphereFinder.Object);
	}

	// Default capsule size
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 48.0f);
}

void AAnimalBase::BeginPlay()
{
	Super::BeginPlay();
	SpawnLocation = GetActorLocation();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// Apply animal appearance based on AnimalID
	ApplyAnimalAppearance();
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

void AAnimalBase::ApplyAnimalAppearance()
{
	// Scale and color based on AnimalID
	FLinearColor BodyColor = FLinearColor(0.45f, 0.3f, 0.15f); // Default brown
	FVector BodyScale = FVector(1.0f, 1.0f, 0.8f);
	FVector HeadScale = FVector(0.5f, 0.5f, 0.5f);
	FVector HeadOffset = FVector(40.0f, 0.0f, 10.0f);

	FString ID = AnimalID.ToString().ToLower();
	if (ID.Contains(TEXT("deer")))
	{
		BodyColor = FLinearColor(0.55f, 0.35f, 0.15f); // Warm brown
		BodyScale = FVector(1.2f, 0.8f, 0.9f);
		HeadScale = FVector(0.45f, 0.45f, 0.55f);
		HeadOffset = FVector(55.0f, 0.0f, 20.0f);
	}
	else if (ID.Contains(TEXT("wolf")))
	{
		BodyColor = FLinearColor(0.4f, 0.4f, 0.42f); // Gray
		BodyScale = FVector(1.0f, 0.7f, 0.7f);
		HeadScale = FVector(0.45f, 0.4f, 0.4f);
		HeadOffset = FVector(50.0f, 0.0f, 10.0f);
	}
	else if (ID.Contains(TEXT("rabbit")) || ID.Contains(TEXT("bunny")))
	{
		BodyColor = FLinearColor(0.85f, 0.8f, 0.7f); // Light tan
		BodyScale = FVector(0.4f, 0.35f, 0.35f);
		HeadScale = FVector(0.25f, 0.25f, 0.28f);
		HeadOffset = FVector(22.0f, 0.0f, 10.0f);
		GetCapsuleComponent()->SetCapsuleSize(20.0f, 20.0f);
	}
	else if (ID.Contains(TEXT("bear")))
	{
		BodyColor = FLinearColor(0.3f, 0.2f, 0.1f); // Dark brown
		BodyScale = FVector(1.5f, 1.2f, 1.2f);
		HeadScale = FVector(0.6f, 0.6f, 0.6f);
		HeadOffset = FVector(60.0f, 0.0f, 20.0f);
	}
	else if (ID.Contains(TEXT("boar")))
	{
		BodyColor = FLinearColor(0.35f, 0.25f, 0.15f); // Dark tan
		BodyScale = FVector(0.9f, 0.7f, 0.65f);
		HeadScale = FVector(0.4f, 0.35f, 0.35f);
		HeadOffset = FVector(45.0f, 0.0f, 5.0f);
	}

	// Apply scale
	if (AnimalBodyMesh)
	{
		AnimalBodyMesh->SetRelativeScale3D(BodyScale);

		UMaterialInterface* BaseMat = AnimalBodyMesh->GetMaterial(0);
		if (BaseMat)
		{
			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), BodyColor);
			AnimalBodyMesh->SetMaterial(0, DynMat);
		}
	}

	if (AnimalHeadMesh)
	{
		AnimalHeadMesh->SetRelativeScale3D(HeadScale);
		AnimalHeadMesh->SetRelativeLocation(HeadOffset);

		UMaterialInterface* BaseMat = AnimalHeadMesh->GetMaterial(0);
		if (BaseMat)
		{
			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			// Head slightly lighter
			FLinearColor HeadColor = BodyColor * 1.15f;
			HeadColor.A = 1.0f;
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), HeadColor);
			AnimalHeadMesh->SetMaterial(0, DynMat);
		}
	}
}
