// Copyright Biome Survivor. All Rights Reserved.

#include "World/WorldPickup.h"
#include "Inventory/InventoryComponent.h"
#include "Player/SurvivorCharacter.h"
#include "BiomeSurvivor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AWorldPickup::AWorldPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);

	// Root collision sphere (physics)
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(30.0f);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->SetSimulatePhysics(true);
	CollisionSphere->SetEnableGravity(true);
	CollisionSphere->BodyInstance.bLockXRotation = true;
	CollisionSphere->BodyInstance.bLockYRotation = true;
	SetRootComponent(CollisionSphere);

	// Visual mesh
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(CollisionSphere);
	PickupMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->CastShadow = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded())
	{
		PickupMesh->SetStaticMesh(SphereFinder.Object);
	}

	// Auto pickup overlap radius
	AutoPickupRadius = CreateDefaultSubobject<USphereComponent>(TEXT("AutoPickupRadius"));
	AutoPickupRadius->SetupAttachment(CollisionSphere);
	AutoPickupRadius->InitSphereRadius(120.0f);
	AutoPickupRadius->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	AutoPickupRadius->SetGenerateOverlapEvents(true);
}

void AWorldPickup::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();

	// Bind overlap for auto-pickup
	AutoPickupRadius->OnComponentBeginOverlap.AddDynamic(this, &AWorldPickup::OnOverlapBegin);

	// After a brief delay, disable physics so it stays put and bobs nicely
	FTimerHandle StopPhysicsTimer;
	GetWorldTimerManager().SetTimer(StopPhysicsTimer, [this]()
	{
		if (CollisionSphere)
		{
			CollisionSphere->SetSimulatePhysics(false);
			SpawnLocation = GetActorLocation();
		}
	}, 1.5f, false);

	ApplyItemColor();

	// Despawn timer
	if (DespawnTime > 0.0f)
	{
		SetLifeSpan(DespawnTime);
	}

	UE_LOG(LogBiomeSurvivor, Log, TEXT("WorldPickup spawned: %s x%d"), *ItemID.ToString(), ItemCount);
}

void AWorldPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Gentle bobbing and rotation for visibility
	if (!CollisionSphere->IsSimulatingPhysics())
	{
		BobTimer += DeltaTime;
		FVector NewLoc = SpawnLocation;
		NewLoc.Z += FMath::Sin(BobTimer * 2.0f) * 8.0f;
		SetActorLocation(NewLoc);
		AddActorWorldRotation(FRotator(0.0f, DeltaTime * 45.0f, 0.0f));
	}
}

void AWorldPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWorldPickup, ItemID);
	DOREPLIFETIME(AWorldPickup, ItemCount);
}

bool AWorldPickup::PickUp(AActor* Collector)
{
	if (bCollected || !Collector) return false;
	if (ItemID.IsNone() || ItemCount <= 0) return false;

	ASurvivorCharacter* Character = Cast<ASurvivorCharacter>(Collector);
	if (!Character) return false;

	UInventoryComponent* Inventory = Character->InventoryComponent;
	if (!Inventory) return false;

	int32 NotAdded = Inventory->AddItem(ItemID, ItemCount);
	if (NotAdded >= ItemCount)
	{
		// Inventory completely full, can't pick up any
		return false;
	}

	bCollected = true;

	// If partially added, spawn a new pickup with remainder
	if (NotAdded > 0)
	{
		SpawnPickup(GetWorld(), GetActorLocation() + FVector(0, 0, 20.f), ItemID, NotAdded);
	}

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Picked up: %s x%d"), *ItemID.ToString(), ItemCount - NotAdded);

	Destroy();
	return true;
}

AWorldPickup* AWorldPickup::SpawnPickup(UWorld* World, const FVector& Location, FName InItemID, int32 Count)
{
	if (!World || InItemID.IsNone() || Count <= 0) return nullptr;

	FTransform SpawnTransform(FRotator::ZeroRotator, Location + FVector(0, 0, 50.0f));
	AWorldPickup* Pickup = World->SpawnActorDeferred<AWorldPickup>(
		AWorldPickup::StaticClass(), SpawnTransform);

	if (Pickup)
	{
		Pickup->ItemID = InItemID;
		Pickup->ItemCount = Count;
		Pickup->FinishSpawning(SpawnTransform);
	}

	return Pickup;
}

void AWorldPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bAutoPickup || bCollected) return;

	if (ASurvivorCharacter* Character = Cast<ASurvivorCharacter>(OtherActor))
	{
		PickUp(Character);
	}
}

void AWorldPickup::ApplyItemColor()
{
	if (!PickupMesh) return;

	UMaterialInterface* BaseMat = PickupMesh->GetMaterial(0);
	if (!BaseMat) return;

	UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);

	// Color based on item name
	FLinearColor Color = FLinearColor(0.7f, 0.7f, 0.7f); // Default gray
	FString ID = ItemID.ToString();

	if (ID.Contains(TEXT("Wood")))        Color = FLinearColor(0.55f, 0.35f, 0.15f); // Brown
	else if (ID.Contains(TEXT("Stone")))   Color = FLinearColor(0.5f, 0.5f, 0.5f);   // Gray
	else if (ID.Contains(TEXT("Berr")))    Color = FLinearColor(0.6f, 0.1f, 0.3f);   // Berry red
	else if (ID.Contains(TEXT("Fiber")))   Color = FLinearColor(0.4f, 0.65f, 0.2f);  // Green
	else if (ID.Contains(TEXT("Meat")))    Color = FLinearColor(0.7f, 0.2f, 0.15f);  // Raw meat red
	else if (ID.Contains(TEXT("Hide")))    Color = FLinearColor(0.6f, 0.45f, 0.25f); // Leather tan

	DynMat->SetVectorParameterValue(TEXT("BaseColor"), Color);
	PickupMesh->SetMaterial(0, DynMat);
}
