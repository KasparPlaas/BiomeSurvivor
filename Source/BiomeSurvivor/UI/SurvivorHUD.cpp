// Copyright Biome Survivor. All Rights Reserved.

#include "UI/SurvivorHUD.h"
#include "Blueprint/UserWidget.h"
#include "BiomeSurvivor.h"

ASurvivorHUD::ASurvivorHUD()
{
}

void ASurvivorHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	// Create the persistent main HUD widget (health bars, quickbar, compass, etc.)
	if (MainWidgetClass)
	{
		MainWidget = CreateWidget<UUserWidget>(PC, MainWidgetClass);
		if (MainWidget)
		{
			MainWidget->AddToViewport(0);
		}
	}

	// Pre-create menu widgets (hidden by default)
	if (InventoryWidgetClass)
	{
		InventoryWidget = CreateWidget<UUserWidget>(PC, InventoryWidgetClass);
	}

	if (CraftingWidgetClass)
	{
		CraftingWidget = CreateWidget<UUserWidget>(PC, CraftingWidgetClass);
	}

	if (MapWidgetClass)
	{
		MapWidget = CreateWidget<UUserWidget>(PC, MapWidgetClass);
	}

	if (PauseMenuWidgetClass)
	{
		PauseMenuWidget = CreateWidget<UUserWidget>(PC, PauseMenuWidgetClass);
	}

	if (DeathScreenWidgetClass)
	{
		DeathScreenWidget = CreateWidget<UUserWidget>(PC, DeathScreenWidgetClass);
	}
}

void ASurvivorHUD::ToggleInventory()
{
	if (!InventoryWidget) return;

	if (InventoryWidget->IsInViewport())
	{
		InventoryWidget->RemoveFromParent();
		// Close crafting too if it was open with inventory
		if (CraftingWidget && CraftingWidget->IsInViewport())
		{
			CraftingWidget->RemoveFromParent();
		}
		SetMenuMode(false);
	}
	else
	{
		HideAllMenus();
		InventoryWidget->AddToViewport(10);
		SetMenuMode(true);
	}
}

void ASurvivorHUD::ToggleCrafting()
{
	if (!CraftingWidget) return;

	if (CraftingWidget->IsInViewport())
	{
		CraftingWidget->RemoveFromParent();
		if (InventoryWidget && InventoryWidget->IsInViewport())
		{
			InventoryWidget->RemoveFromParent();
		}
		SetMenuMode(false);
	}
	else
	{
		HideAllMenus();
		CraftingWidget->AddToViewport(10);
		// Also show inventory alongside crafting
		if (InventoryWidget)
		{
			InventoryWidget->AddToViewport(10);
		}
		SetMenuMode(true);
	}
}

void ASurvivorHUD::ToggleMap()
{
	if (!MapWidget) return;

	if (MapWidget->IsInViewport())
	{
		MapWidget->RemoveFromParent();
		SetMenuMode(false);
	}
	else
	{
		HideAllMenus();
		MapWidget->AddToViewport(10);
		SetMenuMode(true);
	}
}

void ASurvivorHUD::ShowPauseMenu()
{
	if (!PauseMenuWidget) return;

	HideAllMenus();
	PauseMenuWidget->AddToViewport(100); // Highest Z-order
	SetMenuMode(true);
}

void ASurvivorHUD::HidePauseMenu()
{
	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->RemoveFromParent();
		SetMenuMode(false);
	}
}

void ASurvivorHUD::ShowDeathScreen()
{
	if (!DeathScreenWidget) return;

	HideAllMenus();
	DeathScreenWidget->AddToViewport(200);
	SetMenuMode(true);
}

void ASurvivorHUD::HideAllMenus()
{
	if (InventoryWidget && InventoryWidget->IsInViewport())
		InventoryWidget->RemoveFromParent();

	if (CraftingWidget && CraftingWidget->IsInViewport())
		CraftingWidget->RemoveFromParent();

	if (MapWidget && MapWidget->IsInViewport())
		MapWidget->RemoveFromParent();

	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
		PauseMenuWidget->RemoveFromParent();

	// Note: Death screen is not hidden by HideAllMenus

	SetMenuMode(false);
}

void ASurvivorHUD::ShowNotification(const FText& Message, float Duration)
{
	// Implementation: Find notification widget in MainWidget and display message
	// This would be implemented in the Blueprint widget
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Notification: %s"), *Message.ToString());
}

void ASurvivorHUD::ShowInteractionPrompt(const FText& ActionText, const FText& ObjectName)
{
	// Implementation driven by Blueprint MainWidget binding
	UE_LOG(LogBiomeSurvivor, Verbose, TEXT("Interaction: %s - %s"),
		*ActionText.ToString(), *ObjectName.ToString());
}

void ASurvivorHUD::HideInteractionPrompt()
{
	// Driven by Blueprint
}

bool ASurvivorHUD::IsAnyMenuOpen() const
{
	if (InventoryWidget && InventoryWidget->IsInViewport()) return true;
	if (CraftingWidget && CraftingWidget->IsInViewport()) return true;
	if (MapWidget && MapWidget->IsInViewport()) return true;
	if (PauseMenuWidget && PauseMenuWidget->IsInViewport()) return true;
	if (DeathScreenWidget && DeathScreenWidget->IsInViewport()) return true;
	return false;
}

void ASurvivorHUD::SetMenuMode(bool bMenuOpen)
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	if (bMenuOpen)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
	else
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
}
