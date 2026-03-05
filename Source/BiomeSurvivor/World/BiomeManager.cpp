// Copyright Biome Survivor. All Rights Reserved.

#include "World/BiomeManager.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BiomeSurvivor.h"

// ============================================================================
// ABiomeZone
// ============================================================================
ABiomeZone::ABiomeZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneVolume"));
	ZoneVolume->SetBoxExtent(FVector(5000.0f, 5000.0f, 2000.0f));
	ZoneVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ZoneVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	ZoneVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ZoneVolume->SetGenerateOverlapEvents(true);
	RootComponent = ZoneVolume;

#if WITH_EDITORONLY_DATA
	ZoneVolume->SetLineThickness(3.0f);
#endif
}

bool ABiomeZone::IsLocationInZone(const FVector& Location) const
{
	if (!ZoneVolume) return false;

	FVector ClosestPoint;
	float DistSq = ZoneVolume->GetSquaredDistanceToCollision(Location, ClosestPoint);
	return DistSq <= 0.0f; // 0 means point is inside
}

// ============================================================================
// ABiomeManager
// ============================================================================
ABiomeManager::ABiomeManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABiomeManager::BeginPlay()
{
	Super::BeginPlay();

	// Find all biome zones in the level
	TArray<AActor*> FoundZones;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABiomeZone::StaticClass(), FoundZones);

	for (AActor* Actor : FoundZones)
	{
		if (ABiomeZone* Zone = Cast<ABiomeZone>(Actor))
		{
			BiomeZones.Add(Zone);
		}
	}

	// Sort by priority (highest first)
	BiomeZones.Sort([](const ABiomeZone& A, const ABiomeZone& B)
	{
		return A.Priority > B.Priority;
	});

	UE_LOG(LogBiomeSurvivor, Log, TEXT("BiomeManager: Found %d biome zones"), BiomeZones.Num());
}

UBiomeDefinition* ABiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
	for (const ABiomeZone* Zone : BiomeZones)
	{
		if (Zone && Zone->BiomeData && Zone->IsLocationInZone(Location))
		{
			return Zone->BiomeData;
		}
	}

	return DefaultBiome;
}

float ABiomeManager::GetTemperatureAtLocation(const FVector& Location, float GameHour) const
{
	UBiomeDefinition* Biome = GetBiomeAtLocation(Location);
	if (Biome)
	{
		return Biome->GetTemperatureAtHour(GameHour);
	}

	return 15.0f; // Default fallback temperature
}

EBiomeType ABiomeManager::GetBiomeTypeAtLocation(const FVector& Location) const
{
	UBiomeDefinition* Biome = GetBiomeAtLocation(Location);
	if (Biome)
	{
		return Biome->BiomeType;
	}

	return EBiomeType::Forest; // Default biome type
}
