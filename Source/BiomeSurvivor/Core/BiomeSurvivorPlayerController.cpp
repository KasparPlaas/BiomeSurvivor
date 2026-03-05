// Copyright Biome Survivor. All Rights Reserved.

#include "Core/BiomeSurvivorPlayerController.h"
#include "BiomeSurvivor.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

ABiomeSurvivorPlayerController::ABiomeSurvivorPlayerController()
{
}

void ABiomeSurvivorPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Add default input mapping context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	SetGameplayInputMode();
}

void ABiomeSurvivorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	// Input actions are bound in Blueprint or via Enhanced Input assets
}

void ABiomeSurvivorPlayerController::ToggleInventory()
{
	bInventoryOpen = !bInventoryOpen;

	if (bInventoryOpen)
	{
		// Close other menus first
		bCraftingOpen = false;
		bMapOpen = false;
		SetUIInputMode();
	}
	else if (!IsAnyMenuOpen())
	{
		SetGameplayInputMode();
	}

	UE_LOG(LogBiomeSurvivor, Verbose, TEXT("Inventory toggled: %s"), bInventoryOpen ? TEXT("Open") : TEXT("Closed"));
}

void ABiomeSurvivorPlayerController::ToggleCraftingMenu()
{
	bCraftingOpen = !bCraftingOpen;

	if (bCraftingOpen)
	{
		bInventoryOpen = true; // Crafting shows inventory alongside
		bMapOpen = false;
		SetUIInputMode();
	}
	else
	{
		bInventoryOpen = false;
		if (!IsAnyMenuOpen())
		{
			SetGameplayInputMode();
		}
	}
}

void ABiomeSurvivorPlayerController::ToggleBuildMode()
{
	bBuildModeActive = !bBuildModeActive;

	// Build mode keeps gameplay input active (player can still move)
	// but changes HUD to show building wheel

	UE_LOG(LogBiomeSurvivor, Verbose, TEXT("Build mode: %s"), bBuildModeActive ? TEXT("Active") : TEXT("Inactive"));
}

void ABiomeSurvivorPlayerController::ToggleMap()
{
	bMapOpen = !bMapOpen;

	if (bMapOpen)
	{
		bInventoryOpen = false;
		bCraftingOpen = false;
		SetUIInputMode();
	}
	else if (!IsAnyMenuOpen())
	{
		SetGameplayInputMode();
	}
}

void ABiomeSurvivorPlayerController::TogglePauseMenu()
{
	bPauseMenuOpen = !bPauseMenuOpen;

	if (bPauseMenuOpen)
	{
		SetUIInputMode();
	}
	else
	{
		// Close all menus when unpausing
		bInventoryOpen = false;
		bCraftingOpen = false;
		bMapOpen = false;
		SetGameplayInputMode();
	}
}

bool ABiomeSurvivorPlayerController::IsAnyMenuOpen() const
{
	return bInventoryOpen || bCraftingOpen || bMapOpen || bPauseMenuOpen;
}

void ABiomeSurvivorPlayerController::SetGameplayInputMode()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	SetShowMouseCursor(false);
}

void ABiomeSurvivorPlayerController::SetUIInputMode()
{
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}
