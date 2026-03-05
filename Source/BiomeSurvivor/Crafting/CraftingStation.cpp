// Copyright Biome Survivor. All Rights Reserved.

#include "Crafting/CraftingStation.h"
#include "Crafting/CraftingComponent.h"
#include "BiomeSurvivor.h"
#include "Components/StaticMeshComponent.h"

ACraftingStation::ACraftingStation()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f; // Tick once per second for fuel burn

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	SetReplicates(true);
}

void ACraftingStation::BeginPlay()
{
	Super::BeginPlay();
}

void ACraftingStation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority()) return;

	// Burn fuel if active
	if (bIsActive && bRequiresFuel)
	{
		FuelLevel -= DeltaTime;
		if (FuelLevel <= 0.0f)
		{
			FuelLevel = 0.0f;
			bIsActive = false;
			UE_LOG(LogBiomeSurvivor, Log, TEXT("CraftingStation %s ran out of fuel"), *GetName());
		}
	}
}

void ACraftingStation::OnInteract(AActor* Interactor)
{
	if (!Interactor) return;

	// Check fuel requirement
	if (bRequiresFuel && FuelLevel <= 0.0f)
	{
		UE_LOG(LogBiomeSurvivor, Log, TEXT("Station %s needs fuel to operate"), *GetName());
		return;
	}

	// Set the crafting component's current station type
	UCraftingComponent* CraftComp = Interactor->FindComponentByClass<UCraftingComponent>();
	if (CraftComp)
	{
		CraftComp->SetCurrentStation(StationType);
		bIsActive = true;
		UE_LOG(LogBiomeSurvivor, Log, TEXT("Player opened station: %s (%d)"), *StationName.ToString(), (int32)StationType);

		// TODO: Open crafting UI filtered by station type
	}
}

void ACraftingStation::AddFuel(float Amount)
{
	FuelLevel += Amount;
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Fuel added to %s: %.1f (total: %.1f)"), *GetName(), Amount, FuelLevel);
}
