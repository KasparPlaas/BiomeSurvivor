// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.generated.h"

/**
 * Item system definitions - enums, structs, and data assets for all items.
 * Values from plan.md Section 5.2.
 */

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	None		UMETA(DisplayName = "None"),
	Material	UMETA(DisplayName = "Material"),
	Tool		UMETA(DisplayName = "Tool"),
	Weapon		UMETA(DisplayName = "Weapon"),
	Food		UMETA(DisplayName = "Food"),
	Medical		UMETA(DisplayName = "Medical"),
	Clothing	UMETA(DisplayName = "Clothing"),
	Building	UMETA(DisplayName = "Building"),
	Ammo		UMETA(DisplayName = "Ammo"),
	Fuel		UMETA(DisplayName = "Fuel"),
	Key			UMETA(DisplayName = "Key"),
	Misc		UMETA(DisplayName = "Misc")
};

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	Epic		UMETA(DisplayName = "Epic"),
	Legendary	UMETA(DisplayName = "Legendary")
};

UENUM(BlueprintType)
enum class EEquipSlot : uint8
{
	None		UMETA(DisplayName = "None"),
	Head		UMETA(DisplayName = "Head"),
	Chest		UMETA(DisplayName = "Chest"),
	Legs		UMETA(DisplayName = "Legs"),
	Feet		UMETA(DisplayName = "Feet"),
	Hands		UMETA(DisplayName = "Hands"),
	Back		UMETA(DisplayName = "Back"),
	MainHand	UMETA(DisplayName = "Main Hand"),
	OffHand		UMETA(DisplayName = "Off Hand")
};

/** Nutritional values for food/drink items */
USTRUCT(BlueprintType)
struct FItemNutrition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HungerRestore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThirstRestore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthRestore = 0.0f;

	/** Chance of food poisoning (0.0 - 1.0) for raw/spoiled food */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="1"))
	float PoisonChance = 0.0f;

	/** Calories provided */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Calories = 0.0f;
};

/** Combat stats for weapons */
USTRUCT(BlueprintType)
struct FItemCombatStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 150.0f; // cm

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StaminaCost = 5.0f;

	/** For ranged weapons: projectile speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProjectileSpeed = 0.0f;

	/** For ranged weapons: ammo type required (item ID) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredAmmoID;
};

/** Protection stats for clothing/armor */
USTRUCT(BlueprintType)
struct FItemProtection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Armor = 0.0f;

	/** Warmth rating in degrees C */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Warmth = 0.0f;

	/** Water resistance (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WaterResistance = 0.0f;

	/** Wind resistance (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WindResistance = 0.0f;
};

/**
 * UItemDefinition - Primary Data Asset
 * Defines all properties of an item type. Referenced by FName ID.
 */
UCLASS(BlueprintType)
class BIOMESURVIVOR_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Unique string ID for this item (e.g., "item_stone_axe") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FName ItemID;

	/** Display name shown in UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FText DisplayName;

	/** Description shown in tooltip */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item", meta=(MultiLine="true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	EItemCategory Category = EItemCategory::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	EItemRarity Rarity = EItemRarity::Common;

	/** Icon for UI display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 3D mesh for world/equipped display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	/** Weight per unit in kg */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	float Weight = 0.1f;

	/** Max stack size */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	int32 MaxStackSize = 1;

	/** Can this item be used/consumed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	bool bConsumable = false;

	/** Can this item be equipped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	bool bEquippable = false;

	/** Equipment slot (if equippable) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item", meta=(EditCondition="bEquippable"))
	EEquipSlot EquipSlot = EEquipSlot::None;

	/** Durability (0 = indestructible, > 0 = degrades) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	float MaxDurability = 0.0f;

	/** Can be used as fuel? If so, burn time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	float BurnTime = 0.0f;

	// ---- Category-Specific Data ----

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Food", meta=(EditCondition="Category==EItemCategory::Food"))
	FItemNutrition Nutrition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Combat", meta=(EditCondition="Category==EItemCategory::Weapon || Category==EItemCategory::Tool"))
	FItemCombatStats CombatStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Protection", meta=(EditCondition="Category==EItemCategory::Clothing"))
	FItemProtection Protection;

	// UPrimaryDataAsset override
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("ItemDefinition", ItemID);
	}
};

/** Runtime item instance in inventory */
USTRUCT(BlueprintType)
struct FItemInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StackCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentDurability = -1.0f; // -1 = use MaxDurability from definition

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex = -1;

	/** Cached definition pointer (not serialized) */
	UPROPERTY(Transient, BlueprintReadOnly)
	const UItemDefinition* CachedDefinition = nullptr;

	bool IsValid() const { return !ItemID.IsNone() && StackCount > 0; }
	bool IsEmpty() const { return ItemID.IsNone() || StackCount <= 0; }

	void Clear()
	{
		ItemID = NAME_None;
		StackCount = 0;
		CurrentDurability = -1.0f;
		SlotIndex = -1;
		CachedDefinition = nullptr;
	}
};
