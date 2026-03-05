// Copyright Biome Survivor. All Rights Reserved.

#include "Building/BuildingComponent.h"
#include "Inventory/InventoryComponent.h"
#include "BiomeSurvivor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UBuildingComponent::UBuildingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f; // Every frame for smooth preview
}

void UBuildingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuildingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bBuildModeActive && PreviewPiece)
	{
		UpdatePreviewPosition();
	}
}

void UBuildingComponent::ToggleBuildMode()
{
	bBuildModeActive = !bBuildModeActive;

	if (!bBuildModeActive)
	{
		CancelPreview();
	}

	OnBuildModeToggled.Broadcast();
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Build mode: %s"), bBuildModeActive ? TEXT("ON") : TEXT("OFF"));
}

void UBuildingComponent::SelectBuildingPiece(TSubclassOf<ABuildingPiece> PieceClass)
{
	if (!bBuildModeActive || !PieceClass) return;

	// Remove old preview
	CancelPreview();

	SelectedPieceClass = PieceClass;
	PreviewYawRotation = 0.0f;

	// Spawn preview ghost
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = GetOwner();

	PreviewPiece = GetWorld()->SpawnActor<ABuildingPiece>(PieceClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (PreviewPiece)
	{
		PreviewPiece->SetPreviewMode(true);
	}
}

void UBuildingComponent::RotatePreview(float YawDelta)
{
	PreviewYawRotation += YawDelta;
	// Snap to 15 degree increments
	PreviewYawRotation = FMath::RoundToFloat(PreviewYawRotation / 15.0f) * 15.0f;
}

bool UBuildingComponent::PlaceBuilding()
{
	if (!bBuildModeActive || !PreviewPiece || !SelectedPieceClass) return false;
	if (!IsPlacementValid()) return false;

	FVector Location = PreviewPiece->GetActorLocation();
	FRotator Rotation = PreviewPiece->GetActorRotation();

	// Server-side placement
	if (GetOwner()->HasAuthority())
	{
		// Spawn the actual building piece
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ABuildingPiece* PlacedPiece = GetWorld()->SpawnActor<ABuildingPiece>(SelectedPieceClass, Location, Rotation, Params);
		if (PlacedPiece)
		{
			ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
			FString PlayerID = OwnerChar ? OwnerChar->GetName() : TEXT("Unknown");
			PlacedPiece->ConfirmPlacement(PlayerID);
			OnBuildingPlaced.Broadcast(PlacedPiece);

			UE_LOG(LogBiomeSurvivor, Log, TEXT("Building placed: %s at %s"),
				*SelectedPieceClass->GetName(), *Location.ToString());
		}
	}
	else
	{
		ServerPlaceBuilding(SelectedPieceClass, Location, Rotation);
	}

	// Remove preview and respawn a new one for continuous building
	CancelPreview();
	SelectBuildingPiece(SelectedPieceClass);

	return true;
}

void UBuildingComponent::CancelPreview()
{
	if (PreviewPiece)
	{
		PreviewPiece->Destroy();
		PreviewPiece = nullptr;
	}
}

bool UBuildingComponent::DemolishTarget()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return false;

	FVector Start = Character->GetActorLocation() + FVector(0, 0, 60);
	FVector Forward = Character->GetControlRotation().Vector();
	FVector End = Start + Forward * MaxPlaceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	if (PreviewPiece) Params.AddIgnoredActor(PreviewPiece);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		ABuildingPiece* Piece = Cast<ABuildingPiece>(Hit.GetActor());
		if (Piece && !Piece->bIsPreview)
		{
			// Check ownership
			// TODO: Verify player owns this piece or is admin

			if (GetOwner()->HasAuthority())
			{
				// Refund some materials
				// TODO: Calculate refund based on piece health
				Piece->Destroy();
				UE_LOG(LogBiomeSurvivor, Log, TEXT("Building piece demolished: %s"), *Piece->GetName());
				return true;
			}
			else
			{
				ServerDemolishPiece(Piece);
				return true;
			}
		}
	}

	return false;
}

void UBuildingComponent::UpdatePreviewPosition()
{
	FVector Location;
	FRotator Rotation;

	if (GetPlacementTransform(Location, Rotation))
	{
		PreviewPiece->SetActorLocation(Location);
		PreviewPiece->SetActorRotation(Rotation);

		// Try snap to nearby pieces
		PreviewPiece->TrySnapToNearby();

		// Update placement validity
		PreviewPiece->bValidPlacement = IsPlacementValid();

		// TODO: Update material color (green = valid, red = invalid)
	}
}

bool UBuildingComponent::IsPlacementValid() const
{
	if (!PreviewPiece) return false;

	// Check distance from player
	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PreviewPiece->GetActorLocation());
	if (Distance > MaxPlaceDistance) return false;

	// Check not overlapping other actors (basic check)
	// TODO: More sophisticated overlap checks

	return true;
}

bool UBuildingComponent::GetPlacementTransform(FVector& OutLocation, FRotator& OutRotation) const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return false;

	FVector Start = Character->GetActorLocation() + FVector(0, 0, 60);
	FVector Forward = Character->GetControlRotation().Vector();
	FVector End = Start + Forward * MaxPlaceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	if (PreviewPiece) Params.AddIgnoredActor(PreviewPiece);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		// Snap to grid
		float Grid = PreviewPiece ? PreviewPiece->GridSize : 300.0f;
		OutLocation = FVector(
			FMath::RoundToFloat(Hit.Location.X / Grid) * Grid,
			FMath::RoundToFloat(Hit.Location.Y / Grid) * Grid,
			Hit.Location.Z
		);
		OutRotation = FRotator(0.0f, PreviewYawRotation, 0.0f);
		return true;
	}

	// No hit - place at max distance
	OutLocation = Start + Forward * MaxPlaceDistance;
	OutRotation = FRotator(0.0f, PreviewYawRotation, 0.0f);
	return true;
}

// ---- Server RPCs ----

void UBuildingComponent::ServerPlaceBuilding_Implementation(TSubclassOf<ABuildingPiece> PieceClass, FVector Location, FRotator Rotation)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ABuildingPiece* Placed = GetWorld()->SpawnActor<ABuildingPiece>(PieceClass, Location, Rotation, Params);
	if (Placed)
	{
		ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
		FString PlayerID = OwnerChar ? OwnerChar->GetName() : TEXT("Unknown");
		Placed->ConfirmPlacement(PlayerID);
		OnBuildingPlaced.Broadcast(Placed);
	}
}

void UBuildingComponent::ServerDemolishPiece_Implementation(ABuildingPiece* Piece)
{
	if (Piece)
	{
		Piece->Destroy();
	}
}
