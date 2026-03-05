// Copyright Biome Survivor. All Rights Reserved.

#include "Building/BuildingPiece.h"
#include "BiomeSurvivor.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"

ABuildingPiece::ABuildingPiece()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;
	MeshComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	SetReplicates(true);
}

void ABuildingPiece::BeginPlay()
{
	Super::BeginPlay();
	SetMaterial(Material);
}

void ABuildingPiece::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABuildingPiece, Health);
	DOREPLIFETIME(ABuildingPiece, OwnerPlayerID);
}

void ABuildingPiece::ApplyDamage(float DamageAmount, AActor* DamageCauser)
{
	if (!HasAuthority()) return;

	Health = FMath::Max(0.0f, Health - DamageAmount);
	OnBuildingDamaged.Broadcast(Health);

	if (Health <= 0.0f)
	{
		OnBuildingDestroyed.Broadcast();
		UE_LOG(LogBiomeSurvivor, Log, TEXT("Building piece destroyed: %s"), *GetName());

		// TODO: Spawn destruction particles/debris
		// TODO: Drop partial refund materials
		Destroy();
	}
}

void ABuildingPiece::Repair(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
}

void ABuildingPiece::SetPreviewMode(bool bPreview)
{
	bIsPreview = bPreview;

	if (bPreview)
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetCastShadow(false);
		// Set translucent material for preview
		// TODO: Apply ghost/holographic material instance
	}
	else
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetCastShadow(true);
		// Restore original material
	}
}

void ABuildingPiece::ConfirmPlacement(const FString& PlayerID)
{
	bIsPreview = false;
	bValidPlacement = true;
	OwnerPlayerID = PlayerID;

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCastShadow(true);
	// Restore original material

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Building piece placed: %s by %s"), *GetName(), *PlayerID);
}

bool ABuildingPiece::TrySnapToNearby(float SearchRadius)
{
	TArray<AActor*> NearbyActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));

	UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetActorLocation(),
		SearchRadius,
		ObjectTypes,
		ABuildingPiece::StaticClass(),
		TArray<AActor*>{this},
		NearbyActors
	);

	float BestDistance = SearchRadius;
	FVector BestSnapLocation = GetActorLocation();
	FRotator BestSnapRotation = GetActorRotation();
	bool bFoundSnap = false;

	for (AActor* Actor : NearbyActors)
	{
		ABuildingPiece* Other = Cast<ABuildingPiece>(Actor);
		if (!Other || Other->bIsPreview) continue;

		for (const FSnapPoint& OtherSnap : Other->SnapPoints)
		{
			// Check compatibility
			bool bCompatible = false;
			for (EBuildingPieceType CompatType : OtherSnap.CompatibleTypes)
			{
				if (CompatType == PieceType)
				{
					bCompatible = true;
					break;
				}
			}
			if (!bCompatible) continue;

			FVector SnapWorldPos = Other->GetActorLocation() + Other->GetActorRotation().RotateVector(OtherSnap.LocalOffset);
			float Distance = FVector::Dist(GetActorLocation(), SnapWorldPos);

			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				BestSnapLocation = SnapWorldPos;
				BestSnapRotation = Other->GetActorRotation() + OtherSnap.LocalRotation;
				bFoundSnap = true;
			}
		}
	}

	if (bFoundSnap)
	{
		SetActorLocation(BestSnapLocation);
		SetActorRotation(BestSnapRotation);
		bValidPlacement = true;
	}

	return bFoundSnap;
}

void ABuildingPiece::SetMaterial(EBuildingMaterial NewMaterial)
{
	Material = NewMaterial;

	switch (Material)
	{
	case EBuildingMaterial::Thatch:
		MaxHealth = 50.0f;
		break;
	case EBuildingMaterial::Wood:
		MaxHealth = 200.0f;
		break;
	case EBuildingMaterial::Stone:
		MaxHealth = 500.0f;
		break;
	case EBuildingMaterial::Metal:
		MaxHealth = 1000.0f;
		break;
	}

	Health = MaxHealth;
}
