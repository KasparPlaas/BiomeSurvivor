// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemDefinition.h"

/**
 * FItemDatabase
 *
 * Runtime item definition registry. Populates item definitions in code
 * so they work without Blueprint data assets (.uasset files).
 * Call Initialize() once at startup, then use Get() to resolve items.
 */
class BIOMESURVIVOR_API FItemDatabase
{
public:
	/** Initialize all item definitions. Call once at game start. */
	static void Initialize();

	/** Get an item definition by ID. Returns nullptr if not found. */
	static const UItemDefinition* Get(FName ItemID);

	/** Check if database has been initialized */
	static bool IsInitialized() { return bInitialized; }

	/** Get all registered item definitions */
	static const TMap<FName, UItemDefinition*>& GetAll() { return Items; }

	/** Get a display name for an item (falls back to ItemID string) */
	static FText GetDisplayName(FName ItemID);

private:
	/** Create and register a material item */
	static UItemDefinition* RegisterMaterial(FName ID, const FText& Name, const FText& Desc,
		float Weight, int32 MaxStack, EItemRarity Rarity = EItemRarity::Common);

	/** Create and register a food item */
	static UItemDefinition* RegisterFood(FName ID, const FText& Name, const FText& Desc,
		float Weight, int32 MaxStack, float HungerRestore, float ThirstRestore, float HealthRestore,
		float PoisonChance = 0.0f, EItemRarity Rarity = EItemRarity::Common);

	static TMap<FName, UItemDefinition*> Items;
	static bool bInitialized;
};
