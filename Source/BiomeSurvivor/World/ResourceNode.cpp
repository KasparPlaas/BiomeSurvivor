// Copyright Biome Survivor. All Rights Reserved.

#include "World/ResourceNode.h"
#include "BiomeSurvivor.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Inventory/InventoryComponent.h"

AResourceNode::AResourceNode()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Mesh component
	ResourceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ResourceMesh"));
	ResourceMesh->SetCollisionProfileName(TEXT("BlockAll"));
	ResourceMesh->SetGenerateOverlapEvents(false);
	RootComponent = ResourceMesh;

	// Interaction sphere
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetSphereRadius(200.0f);
	InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	InteractionSphere->SetGenerateOverlapEvents(true);
	InteractionSphere->SetupAttachment(RootComponent);

	CurrentHealth = MaxHealth;
}

void AResourceNode::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	OriginalScale = GetActorScale3D();
}

void AResourceNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AResourceNode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AResourceNode, CurrentHealth);
	DOREPLIFETIME(AResourceNode, bIsDepleted);
}

bool AResourceNode::HarvestNode(AActor* Harvester, float Damage, FName ToolType)
{
	if (!HasAuthority()) return false;
	if (bIsDepleted || !Harvester) return false;

	// Apply tool multiplier
	float FinalDamage = Damage;
	if (!RequiredToolType.IsNone() && ToolType == RequiredToolType)
	{
		FinalDamage *= CorrectToolMultiplier;
	}
	else if (!RequiredToolType.IsNone() && ToolType != RequiredToolType)
	{
		// Wrong tool = much less effective
		FinalDamage *= 0.25f;
	}

	CurrentHealth = FMath::Max(CurrentHealth - FinalDamage, 0.0f);

	// Give harvest yields on each hit
	GiveYields(Harvester, HarvestYields);

	// Visual feedback - slight scale change to show damage
	float HealthPct = GetHealthPercent();
	SetActorScale3D(OriginalScale * FMath::Lerp(0.7f, 1.0f, HealthPct));

	UE_LOG(LogBiomeSurvivor, Verbose, TEXT("Resource %s hit for %.1f damage (%.0f%% remaining)"),
		*ResourceName.ToString(), FinalDamage, HealthPct * 100.0f);

	// Check for depletion
	if (CurrentHealth <= 0.0f)
	{
		DepleteResource();
		GiveYields(Harvester, DepletionBonusYields);
	}

	return true;
}

void AResourceNode::OnRep_CurrentHealth()
{
	// Update visual state on clients
	float HealthPct = GetHealthPercent();
	SetActorScale3D(OriginalScale * FMath::Lerp(0.7f, 1.0f, HealthPct));
}

void AResourceNode::OnRep_IsDepleted()
{
	if (bIsDepleted)
	{
		// Hide the resource mesh
		ResourceMesh->SetVisibility(false);
		ResourceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		// Respawned - restore
		ResourceMesh->SetVisibility(true);
		ResourceMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SetActorScale3D(OriginalScale);
	}
}

void AResourceNode::DepleteResource()
{
	bIsDepleted = true;
	OnRep_IsDepleted(); // Run locally on server too

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Resource depleted: %s"), *ResourceName.ToString());

	// Start respawn timer
	if (RespawnTimeSeconds > 0.0f)
	{
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this,
			&AResourceNode::RespawnResource, RespawnTimeSeconds, false);
	}
}

void AResourceNode::RespawnResource()
{
	bIsDepleted = false;
	CurrentHealth = MaxHealth;
	OnRep_IsDepleted(); // Run locally on server too

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Resource respawned: %s"), *ResourceName.ToString());
}

void AResourceNode::GiveYields(AActor* Harvester, const TArray<FResourceYield>& Yields)
{
	if (!Harvester) return;

	UInventoryComponent* Inventory = Harvester->FindComponentByClass<UInventoryComponent>();
	if (!Inventory) return;

	for (const FResourceYield& Yield : Yields)
	{
		// Check drop chance
		if (FMath::FRand() > Yield.DropChance) continue;

		// Check skill requirement (future: query player skill level)
		// For now, always give if chance passes

		// Calculate quantity
		int32 Quantity = Yield.BaseQuantity;
		if (Yield.BonusQuantity > 0)
		{
			Quantity += FMath::RandRange(0, Yield.BonusQuantity);
		}

		// Create item instance and add to inventory
		FItemInstance NewItem;
		NewItem.ItemID = Yield.ItemID;
		NewItem.StackCount = Quantity;
		NewItem.InstanceID = FGuid::NewGuid();

		Inventory->AddItem(NewItem);

		UE_LOG(LogBiomeSurvivor, Verbose, TEXT("Yielded %d x %s to player"),
			Quantity, *Yield.ItemID.ToString());
	}
}
