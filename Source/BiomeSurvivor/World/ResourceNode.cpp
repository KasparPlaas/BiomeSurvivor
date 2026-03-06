// Copyright Biome Survivor. All Rights Reserved.

#include "World/ResourceNode.h"
#include "BiomeSurvivor.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AResourceNode::AResourceNode()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Mesh component
	ResourceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ResourceMesh"));
	ResourceMesh->SetCollisionProfileName(TEXT("BlockAll"));
	ResourceMesh->SetGenerateOverlapEvents(false);
	RootComponent = ResourceMesh;

	// Find basic shapes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere"));

	// Default to cylinder (tree trunk)
	if (CylinderFinder.Succeeded())
	{
		ResourceMesh->SetStaticMesh(CylinderFinder.Object);
	}

	// Secondary mesh (canopy for trees)
	SecondaryMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SecondaryMesh"));
	SecondaryMesh->SetupAttachment(RootComponent);
	SecondaryMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SecondaryMesh->SetVisibility(false); // Hidden by default, enabled for trees
	if (SphereFinder.Succeeded())
	{
		SecondaryMesh->SetStaticMesh(SphereFinder.Object);
	}

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

	// Apply appearance based on resource type
	FLinearColor PrimaryColor = FLinearColor(0.4f, 0.3f, 0.2f);
	FLinearColor SecondaryColor = FLinearColor(0.1f, 0.5f, 0.1f);

	switch (ResourceType)
	{
	case EResourceNodeType::Tree:
	{
		// Tall cylinder trunk + sphere canopy
		ResourceMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 3.0f));
		PrimaryColor = FLinearColor(0.35f, 0.22f, 0.1f); // Brown trunk

		SecondaryMesh->SetVisibility(true);
		SecondaryMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 350.0f));
		SecondaryMesh->SetRelativeScale3D(FVector(3.0f, 3.0f, 2.5f));
		SecondaryColor = FLinearColor(0.05f, 0.35f, 0.05f); // Green canopy
		break;
	}
	case EResourceNodeType::Rock:
	{
		// Use sphere mesh for rock
		static UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
		if (SphereMesh)
		{
			ResourceMesh->SetStaticMesh(SphereMesh);
		}
		ResourceMesh->SetRelativeScale3D(FVector(1.2f, 1.0f, 0.7f));
		PrimaryColor = FLinearColor(0.45f, 0.43f, 0.4f); // Gray stone
		break;
	}
	case EResourceNodeType::OreDeposit:
	{
		static UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
		if (SphereMesh)
		{
			ResourceMesh->SetStaticMesh(SphereMesh);
		}
		ResourceMesh->SetRelativeScale3D(FVector(0.9f, 0.8f, 0.6f));
		PrimaryColor = FLinearColor(0.5f, 0.35f, 0.15f); // Rusty ore
		break;
	}
	case EResourceNodeType::BerryBush:
	{
		static UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
		if (SphereMesh)
		{
			ResourceMesh->SetStaticMesh(SphereMesh);
		}
		ResourceMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.6f));
		PrimaryColor = FLinearColor(0.1f, 0.4f, 0.1f); // Green bush
		break;
	}
	case EResourceNodeType::HerbPlant:
	case EResourceNodeType::FiberPlant:
	case EResourceNodeType::CattailReed:
	{
		ResourceMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.8f));
		PrimaryColor = FLinearColor(0.15f, 0.45f, 0.1f);
		break;
	}
	default:
		break;
	}

	// Apply dynamic material to main mesh
	UMaterialInterface* BaseMat = ResourceMesh->GetMaterial(0);
	if (BaseMat)
	{
		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		DynMat->SetVectorParameterValue(TEXT("BaseColor"), PrimaryColor);
		ResourceMesh->SetMaterial(0, DynMat);
	}

	// Apply dynamic material to secondary mesh if visible
	if (SecondaryMesh && SecondaryMesh->IsVisible())
	{
		UMaterialInterface* SecMat = SecondaryMesh->GetMaterial(0);
		if (SecMat)
		{
			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(SecMat, this);
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), SecondaryColor);
			SecondaryMesh->SetMaterial(0, DynMat);
		}
	}
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
		Inventory->AddItem(Yield.ItemID, Quantity);

		UE_LOG(LogBiomeSurvivor, Verbose, TEXT("Yielded %d x %s to player"),
			Quantity, *Yield.ItemID.ToString());
	}
}
