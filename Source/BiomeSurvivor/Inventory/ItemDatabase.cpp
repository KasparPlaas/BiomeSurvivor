// Copyright Biome Survivor. All Rights Reserved.

#include "Inventory/ItemDatabase.h"
#include "BiomeSurvivor.h"

TMap<FName, UItemDefinition*> FItemDatabase::Items;
bool FItemDatabase::bInitialized = false;

UItemDefinition* FItemDatabase::RegisterMaterial(FName ID, const FText& Name, const FText& Desc,
	float Weight, int32 MaxStack, EItemRarity Rarity)
{
	UItemDefinition* Def = NewObject<UItemDefinition>(GetTransientPackage(), *ID.ToString());
	Def->ItemID = ID;
	Def->DisplayName = Name;
	Def->Description = Desc;
	Def->Category = EItemCategory::Material;
	Def->Rarity = Rarity;
	Def->Weight = Weight;
	Def->MaxStackSize = MaxStack;
	Def->bConsumable = false;
	Def->bEquippable = false;
	Def->AddToRoot(); // Prevent GC

	Items.Add(ID, Def);
	return Def;
}

UItemDefinition* FItemDatabase::RegisterFood(FName ID, const FText& Name, const FText& Desc,
	float Weight, int32 MaxStack, float HungerRestore, float ThirstRestore, float HealthRestore,
	float PoisonChance, EItemRarity Rarity)
{
	UItemDefinition* Def = NewObject<UItemDefinition>(GetTransientPackage(), *ID.ToString());
	Def->ItemID = ID;
	Def->DisplayName = Name;
	Def->Description = Desc;
	Def->Category = EItemCategory::Food;
	Def->Rarity = Rarity;
	Def->Weight = Weight;
	Def->MaxStackSize = MaxStack;
	Def->bConsumable = true;
	Def->bEquippable = false;
	Def->Nutrition.HungerRestore = HungerRestore;
	Def->Nutrition.ThirstRestore = ThirstRestore;
	Def->Nutrition.HealthRestore = HealthRestore;
	Def->Nutrition.PoisonChance = PoisonChance;
	Def->AddToRoot();

	Items.Add(ID, Def);
	return Def;
}

void FItemDatabase::Initialize()
{
	if (bInitialized) return;

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Initializing runtime item database..."));

	// ============ MATERIALS ============
	RegisterMaterial(FName("Wood"),
		FText::FromString(TEXT("Wood")),
		FText::FromString(TEXT("Basic building material harvested from trees.")),
		0.5f, 50);

	RegisterMaterial(FName("Stone"),
		FText::FromString(TEXT("Stone")),
		FText::FromString(TEXT("Hard stone, useful for tools and construction.")),
		1.0f, 50);

	RegisterMaterial(FName("Fiber"),
		FText::FromString(TEXT("Plant Fiber")),
		FText::FromString(TEXT("Flexible fiber harvested from plants. Used in crafting.")),
		0.1f, 100);

	RegisterMaterial(FName("AnimalHide"),
		FText::FromString(TEXT("Animal Hide")),
		FText::FromString(TEXT("Raw hide from an animal. Can be tanned into leather.")),
		0.8f, 20);

	RegisterMaterial(FName("Bone"),
		FText::FromString(TEXT("Bone")),
		FText::FromString(TEXT("Animal bone. Used in primitive tools and crafting.")),
		0.3f, 30);

	RegisterMaterial(FName("Flint"),
		FText::FromString(TEXT("Flint")),
		FText::FromString(TEXT("Sharp flint stone. Essential for making basic tools.")),
		0.5f, 30, EItemRarity::Uncommon);

	RegisterMaterial(FName("Rope"),
		FText::FromString(TEXT("Rope")),
		FText::FromString(TEXT("Woven plant fiber rope. Useful for binding and building.")),
		0.2f, 20);

	// ============ FOOD ============
	RegisterFood(FName("Berries"),
		FText::FromString(TEXT("Wild Berries")),
		FText::FromString(TEXT("A handful of wild berries. Mildly nutritious.")),
		0.1f, 30,
		8.0f, 3.0f, 0.0f,    // hunger, thirst, health
		0.05f);               // small poison chance

	RegisterFood(FName("RawMeat"),
		FText::FromString(TEXT("Raw Meat")),
		FText::FromString(TEXT("Uncooked animal meat. Risky to eat raw.")),
		0.5f, 20,
		15.0f, 0.0f, 0.0f,
		0.35f);               // high poison chance raw

	RegisterFood(FName("CookedMeat"),
		FText::FromString(TEXT("Cooked Meat")),
		FText::FromString(TEXT("Well-cooked meat. Safe and nutritious.")),
		0.4f, 20,
		30.0f, 0.0f, 5.0f,
		0.0f, EItemRarity::Uncommon);

	RegisterFood(FName("CleanWater"),
		FText::FromString(TEXT("Clean Water")),
		FText::FromString(TEXT("Purified water. Fully hydrating.")),
		0.5f, 10,
		0.0f, 40.0f, 0.0f);

	RegisterFood(FName("DirtyWater"),
		FText::FromString(TEXT("Dirty Water")),
		FText::FromString(TEXT("Untreated water. May cause sickness.")),
		0.5f, 10,
		0.0f, 25.0f, 0.0f,
		0.25f);

	RegisterFood(FName("Mushroom"),
		FText::FromString(TEXT("Wild Mushroom")),
		FText::FromString(TEXT("A forest mushroom. Could be edible... or not.")),
		0.05f, 30,
		5.0f, 1.0f, 0.0f,
		0.15f);

	// ============ MEDICAL ============
	{
		UItemDefinition* Def = NewObject<UItemDefinition>(GetTransientPackage(), TEXT("Bandage"));
		Def->ItemID = FName("Bandage");
		Def->DisplayName = FText::FromString(TEXT("Cloth Bandage"));
		Def->Description = FText::FromString(TEXT("A simple bandage to stop bleeding and restore some health."));
		Def->Category = EItemCategory::Medical;
		Def->Rarity = EItemRarity::Common;
		Def->Weight = 0.1f;
		Def->MaxStackSize = 10;
		Def->bConsumable = true;
		Def->Nutrition.HealthRestore = 20.0f;
		Def->AddToRoot();
		Items.Add(Def->ItemID, Def);
	}

	bInitialized = true;
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Item database initialized with %d items"), Items.Num());
}

const UItemDefinition* FItemDatabase::Get(FName ItemID)
{
	if (!bInitialized) Initialize();
	if (UItemDefinition** Found = Items.Find(ItemID))
	{
		return *Found;
	}
	return nullptr;
}

FText FItemDatabase::GetDisplayName(FName ItemID)
{
	if (const UItemDefinition* Def = Get(ItemID))
	{
		return Def->DisplayName;
	}
	// Fallback: convert the FName to a readable string
	return FText::FromName(ItemID);
}
