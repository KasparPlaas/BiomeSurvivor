// Copyright Biome Survivor. All Rights Reserved.

#include "UI/SurvivorHUD.h"
#include "Player/SurvivorCharacter.h"
#include "Player/PlayerStatsComponent.h"
#include "BiomeSurvivor.h"

// Slate includes
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

// Static brush for Slate backgrounds
namespace
{
	const FSlateColorBrush SolidWhiteBrush(FLinearColor::White);
}

// ==================================================================
// Lifecycle
// ==================================================================

ASurvivorHUD::ASurvivorHUD()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASurvivorHUD::BeginPlay()
{
	Super::BeginPlay();

	CreateMainMenuSlate();
	CreatePauseMenuSlate();

	// Start with main menu visible
	ShowMainMenu();
}

void ASurvivorHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GEngine && GEngine->GameViewport)
	{
		if (MainMenuOverlay.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(MainMenuOverlay.ToSharedRef());
			MainMenuOverlay.Reset();
		}
		if (PauseMenuOverlay.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(PauseMenuOverlay.ToSharedRef());
			PauseMenuOverlay.Reset();
		}
	}
	Super::EndPlay(EndPlayReason);
}

void ASurvivorHUD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Smooth stat bar animations
	if (UPlayerStatsComponent* Stats = GetPlayerStats())
	{
		const float InterpSpeed = 8.0f;
		DisplayHealth = FMath::FInterpTo(DisplayHealth, Stats->GetHealthPercent(), DeltaTime, InterpSpeed);
		DisplayHunger = FMath::FInterpTo(DisplayHunger, Stats->GetHungerPercent(), DeltaTime, InterpSpeed);
		DisplayThirst = FMath::FInterpTo(DisplayThirst, Stats->GetThirstPercent(), DeltaTime, InterpSpeed);
		DisplayStamina = FMath::FInterpTo(DisplayStamina, Stats->GetStaminaPercent(), DeltaTime, InterpSpeed);
	}

	// Tick notifications
	for (int32 i = ActiveNotifications.Num() - 1; i >= 0; --i)
	{
		ActiveNotifications[i].TimeRemaining -= DeltaTime;
		if (ActiveNotifications[i].TimeRemaining <= 0.0f)
		{
			ActiveNotifications.RemoveAt(i);
		}
	}
}

void ASurvivorHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas) return;

	// Don't draw gameplay HUD when main menu is showing
	if (bMainMenuVisible) return;

	DrawCrosshair();
	DrawStatBars();
	DrawCompass();
	DrawInteractionPromptHUD();
	DrawNotifications();

	if (bDeathScreenVisible)
	{
		DrawDeathOverlay();
	}
}

// ==================================================================
// Canvas HUD Elements
// ==================================================================

void ASurvivorHUD::DrawCrosshair()
{
	if (bPauseMenuVisible) return;

	const float CX = Canvas->SizeX * 0.5f;
	const float CY = Canvas->SizeY * 0.5f;
	const FLinearColor CrossColor(1.0f, 1.0f, 1.0f, 0.75f);

	// Center dot
	DrawRect(CrossColor, CX - 2, CY - 2, 4, 4);

	// Lines with gap
	const float Gap = 6.0f;
	const float Len = 12.0f;
	const float Thick = 1.5f;

	DrawLine(CX, CY - Gap - Len, CX, CY - Gap, CrossColor, Thick);
	DrawLine(CX, CY + Gap, CX, CY + Gap + Len, CrossColor, Thick);
	DrawLine(CX - Gap - Len, CY, CX - Gap, CY, CrossColor, Thick);
	DrawLine(CX + Gap, CY, CX + Gap + Len, CY, CrossColor, Thick);
}

void ASurvivorHUD::DrawStatBars()
{
	UPlayerStatsComponent* Stats = GetPlayerStats();
	if (!Stats) return;

	const float BarW = 200.0f;
	const float BarH = 14.0f;
	const float Pad = 6.0f;
	const float StartX = 30.0f;
	const float StartY = Canvas->SizeY - 130.0f;

	// Health (red)
	DrawStatBar(StartX, StartY, BarW, BarH, DisplayHealth,
		FLinearColor(0.85f, 0.15f, 0.15f), TEXT("HP"),
		FString::Printf(TEXT("%.0f/%.0f"), Stats->GetHealth(), Stats->MaxHealth));

	// Hunger (orange)
	DrawStatBar(StartX, StartY + BarH + Pad, BarW, BarH, DisplayHunger,
		FLinearColor(0.9f, 0.55f, 0.12f), TEXT("Food"),
		FString::Printf(TEXT("%.0f/%.0f"), Stats->GetHunger(), Stats->MaxHunger));

	// Thirst (blue)
	DrawStatBar(StartX, StartY + 2 * (BarH + Pad), BarW, BarH, DisplayThirst,
		FLinearColor(0.2f, 0.5f, 0.9f), TEXT("Water"),
		FString::Printf(TEXT("%.0f/%.0f"), Stats->GetThirst(), Stats->MaxThirst));

	// Stamina (green)
	DrawStatBar(StartX, StartY + 3 * (BarH + Pad), BarW, BarH, DisplayStamina,
		FLinearColor(0.3f, 0.82f, 0.3f), TEXT("Stam"),
		FString::Printf(TEXT("%.0f/%.0f"), Stats->GetStamina(), Stats->MaxStamina));
}

void ASurvivorHUD::DrawStatBar(float X, float Y, float Width, float Height, float Percent,
	const FLinearColor& BarColor, const FString& Label, const FString& ValueText)
{
	Percent = FMath::Clamp(Percent, 0.0f, 1.0f);

	const float LabelW = 48.0f;
	const float BarX = X + LabelW;
	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font) return;

	// Label
	DrawText(Label, FLinearColor(0.9f, 0.9f, 0.9f), X, Y - 1.0f, Font, 1.0f);

	// Bar background
	DrawRect(FLinearColor(0.04f, 0.04f, 0.06f, 0.75f), BarX, Y, Width, Height);

	// Bar fill
	FLinearColor FillColor = BarColor;
	if (Percent < 0.2f)
	{
		// Pulse when low
		const float Pulse = (FMath::Sin(GetWorld()->GetTimeSeconds() * 6.0f) + 1.0f) * 0.3f;
		FillColor = FLinearColor::LerpUsingHSV(BarColor, FLinearColor(1.0f, 0.0f, 0.0f), Pulse);
	}
	if (Percent > 0.0f)
	{
		DrawRect(FillColor, BarX + 1, Y + 1, (Width - 2) * Percent, Height - 2);
	}

	// Border
	const FLinearColor BorderCol(0.3f, 0.3f, 0.3f, 0.6f);
	DrawLine(BarX, Y, BarX + Width, Y, BorderCol);
	DrawLine(BarX, Y + Height, BarX + Width, Y + Height, BorderCol);
	DrawLine(BarX, Y, BarX, Y + Height, BorderCol);
	DrawLine(BarX + Width, Y, BarX + Width, Y + Height, BorderCol);

	// Value text
	DrawText(ValueText, FLinearColor(0.85f, 0.85f, 0.85f, 0.8f),
		BarX + Width + 8.0f, Y - 1.0f, Font, 0.85f);
}

void ASurvivorHUD::DrawCompass()
{
	if (bPauseMenuVisible) return;

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return;

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font) return;

	const float CenterX = Canvas->SizeX * 0.5f;
	const float Y = 28.0f;
	const float CompassW = 400.0f;

	// Background
	DrawRect(FLinearColor(0.02f, 0.02f, 0.04f, 0.45f),
		CenterX - CompassW * 0.5f, Y - 4.0f, CompassW, 28.0f);

	// Get yaw normalized to 0-360
	float Yaw = Pawn->GetActorRotation().Yaw;
	while (Yaw < 0.0f) Yaw += 360.0f;
	while (Yaw > 360.0f) Yaw -= 360.0f;

	// Cardinal directions
	struct FCompassPt { FString Label; float Deg; FLinearColor Color; };
	const TArray<FCompassPt> Points = {
		{TEXT("N"), 0.0f, FLinearColor(1.0f, 0.3f, 0.3f)},
		{TEXT("NE"), 45.0f, FLinearColor(0.6f, 0.6f, 0.6f)},
		{TEXT("E"), 90.0f, FLinearColor::White},
		{TEXT("SE"), 135.0f, FLinearColor(0.6f, 0.6f, 0.6f)},
		{TEXT("S"), 180.0f, FLinearColor::White},
		{TEXT("SW"), 225.0f, FLinearColor(0.6f, 0.6f, 0.6f)},
		{TEXT("W"), 270.0f, FLinearColor::White},
		{TEXT("NW"), 315.0f, FLinearColor(0.6f, 0.6f, 0.6f)},
	};

	for (const auto& Pt : Points)
	{
		float Diff = Pt.Deg - Yaw;
		while (Diff > 180.0f) Diff -= 360.0f;
		while (Diff < -180.0f) Diff += 360.0f;

		if (FMath::Abs(Diff) < 90.0f)
		{
			const float ScreenX = CenterX + (Diff / 90.0f) * (CompassW * 0.5f);
			float TW = 0, TH = 0;
			GetTextSize(Pt.Label, TW, TH, Font, 1.0f);
			DrawText(Pt.Label, Pt.Color, ScreenX - TW * 0.5f, Y, Font, 1.0f);
		}
	}

	// Bearing number
	const FString Bearing = FString::Printf(TEXT("%.0f%s"), FMath::Fmod(Yaw + 360.0f, 360.0f), TEXT("\u00B0"));
	float BW = 0, BH = 0;
	GetTextSize(Bearing, BW, BH, Font, 0.8f);
	DrawText(Bearing, FLinearColor(0.7f, 0.7f, 0.7f, 0.5f), CenterX - BW * 0.5f, Y + 14.0f, Font, 0.8f);

	// Center tick mark
	DrawLine(CenterX, Y - 4.0f, CenterX, Y + 2.0f, FLinearColor(1.0f, 0.8f, 0.3f, 0.8f), 2.0f);
}

void ASurvivorHUD::DrawInteractionPromptHUD()
{
	if (!bShowInteraction || bPauseMenuVisible) return;

	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	if (!Font) return;

	const float CenterX = Canvas->SizeX * 0.5f;
	const float YPos = Canvas->SizeY * 0.62f;

	const FString PromptLine = FString::Printf(TEXT("[E]  %s"), *InteractObjectText.ToString());
	float TW = 0, TH = 0;
	GetTextSize(PromptLine, TW, TH, Font, 1.0f);

	const float Pad = 12.0f;
	DrawRect(FLinearColor(0.02f, 0.02f, 0.04f, 0.55f),
		CenterX - TW * 0.5f - Pad, YPos - Pad * 0.5f,
		TW + Pad * 2, TH + Pad);

	DrawText(PromptLine, FLinearColor(1.0f, 0.9f, 0.7f),
		CenterX - TW * 0.5f, YPos, Font, 1.0f);
}

void ASurvivorHUD::DrawNotifications()
{
	if (ActiveNotifications.Num() == 0) return;

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font) return;

	const float CenterX = Canvas->SizeX * 0.5f;
	const float BaseY = Canvas->SizeY * 0.22f;

	for (int32 i = 0; i < ActiveNotifications.Num() && i < 5; ++i)
	{
		const FHUDNotification& N = ActiveNotifications[i];
		const float Alpha = FMath::Clamp(N.TimeRemaining, 0.0f, 1.0f);

		const FString Txt = N.Message.ToString();
		float TW = 0, TH = 0;
		GetTextSize(Txt, TW, TH, Font, 1.0f);

		const float YPos = BaseY + i * 26.0f;

		DrawRect(FLinearColor(0.02f, 0.02f, 0.04f, 0.45f * Alpha),
			CenterX - TW * 0.5f - 8, YPos - 3, TW + 16, TH + 6);

		DrawText(Txt, FLinearColor(1.0f, 0.9f, 0.7f, Alpha),
			CenterX - TW * 0.5f, YPos, Font, 1.0f);
	}
}

void ASurvivorHUD::DrawDeathOverlay()
{
	// Full screen red overlay
	DrawRect(FLinearColor(0.25f, 0.0f, 0.0f, 0.55f), 0, 0, Canvas->SizeX, Canvas->SizeY);

	UFont* LargeFont = GEngine ? GEngine->GetLargeFont() : nullptr;
	if (!LargeFont) return;

	const FString Title = TEXT("YOU DIED");
	float TW = 0, TH = 0;
	GetTextSize(Title, TW, TH, LargeFont, 2.5f);
	DrawText(Title, FLinearColor(0.9f, 0.1f, 0.1f),
		Canvas->SizeX * 0.5f - TW * 0.5f, Canvas->SizeY * 0.35f, LargeFont, 2.5f);

	UFont* SmallFont = GEngine->GetSmallFont();
	const FString Sub = TEXT("Press any key to respawn...");
	float SW = 0, SH = 0;
	GetTextSize(Sub, SW, SH, SmallFont, 1.2f);
	DrawText(Sub, FLinearColor(0.7f, 0.7f, 0.7f, 0.75f),
		Canvas->SizeX * 0.5f - SW * 0.5f, Canvas->SizeY * 0.5f, SmallFont, 1.2f);
}

// ==================================================================
// Slate Menu Creation
// ==================================================================

void ASurvivorHUD::CreateMainMenuSlate()
{
	if (!GEngine || !GEngine->GameViewport) return;

	// Helper: create a styled menu button
	auto MakeBtn = [](const FText& Label, TFunction<void()> OnClick) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(320.0f)
			.HeightOverride(52.0f)
			[
				SNew(SButton)
				.ButtonColorAndOpacity(FLinearColor(0.10f, 0.10f, 0.14f, 0.85f))
				.OnClicked_Lambda([OnClick]() -> FReply {
					if (OnClick) OnClick();
					return FReply::Handled();
				})
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.88f, 0.76f)))
					.Justification(ETextJustify::Center)
				]
			];
	};

	// Build widget tree
	SAssignNew(MainMenuOverlay, SOverlay)

	// Dark background
	+ SOverlay::Slot()
	[
		SNew(SBorder)
		.BorderImage(&SolidWhiteBrush)
		.BorderBackgroundColor(FLinearColor(0.012f, 0.015f, 0.035f, 0.92f))
	]

	// Content
	+ SOverlay::Slot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SVerticalBox)

		// Title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 8)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("BIOME SURVIVOR")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 64))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.88f, 0.80f, 0.55f)))
			.Justification(ETextJustify::Center)
		]

		// Subtitle
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 60)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Survive.  Adapt.  Thrive.")))
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 16))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.52f, 0.45f)))
			.Justification(ETextJustify::Center)
		]

		// Play
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 14)
		.HAlign(HAlign_Center)
		[
			MakeBtn(FText::FromString(TEXT("P L A Y")), [this]() {
				HideMainMenu();
				if (APlayerController* PC = GetOwningPlayerController())
				{
					FInputModeGameOnly Mode;
					PC->SetInputMode(Mode);
					PC->SetShowMouseCursor(false);
				}
			})
		]

		// Settings
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 14)
		.HAlign(HAlign_Center)
		[
			MakeBtn(FText::FromString(TEXT("S E T T I N G S")), [this]() {
				ShowNotification(FText::FromString(TEXT("Settings coming soon...")), 2.0f);
			})
		]

		// Quit
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 14)
		.HAlign(HAlign_Center)
		[
			MakeBtn(FText::FromString(TEXT("Q U I T")), [this]() {
				UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
			})
		]

		// Version
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 50, 0, 0)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Pre-Alpha Build  v0.1.0")))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.35f, 0.35f, 0.35f)))
		]
	];

	// Add to viewport at high Z-order
	GEngine->GameViewport->AddViewportWidgetContent(MainMenuOverlay.ToSharedRef(), 100);
}

void ASurvivorHUD::CreatePauseMenuSlate()
{
	if (!GEngine || !GEngine->GameViewport) return;

	auto MakeBtn = [](const FText& Label, TFunction<void()> OnClick) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(300.0f)
			.HeightOverride(48.0f)
			[
				SNew(SButton)
				.ButtonColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.12f, 0.88f))
				.OnClicked_Lambda([OnClick]() -> FReply {
					if (OnClick) OnClick();
					return FReply::Handled();
				})
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.90f, 0.87f, 0.76f)))
					.Justification(ETextJustify::Center)
				]
			];
	};

	SAssignNew(PauseMenuOverlay, SOverlay)

	// Dim background
	+ SOverlay::Slot()
	[
		SNew(SBorder)
		.BorderImage(&SolidWhiteBrush)
		.BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.02f, 0.72f))
	]

	// Content
	+ SOverlay::Slot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SVerticalBox)

		// Title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 40)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("PAUSED")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.90f, 0.85f, 0.70f)))
			.Justification(ETextJustify::Center)
		]

		// Resume
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 12)
		.HAlign(HAlign_Center)
		[
			MakeBtn(FText::FromString(TEXT("R E S U M E")), [this]() {
				HidePauseMenu();
			})
		]

		// Settings
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 12)
		.HAlign(HAlign_Center)
		[
			MakeBtn(FText::FromString(TEXT("S E T T I N G S")), [this]() {
				ShowNotification(FText::FromString(TEXT("Settings coming soon...")), 2.0f);
			})
		]

		// Quit to Desktop
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 12)
		.HAlign(HAlign_Center)
		[
			MakeBtn(FText::FromString(TEXT("Q U I T   T O   D E S K T O P")), [this]() {
				UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
			})
		]
	];

	GEngine->GameViewport->AddViewportWidgetContent(PauseMenuOverlay.ToSharedRef(), 200);

	// Start hidden
	if (PauseMenuOverlay.IsValid())
	{
		PauseMenuOverlay->SetVisibility(EVisibility::Collapsed);
	}
}

// ==================================================================
// Menu Control
// ==================================================================

void ASurvivorHUD::ShowMainMenu()
{
	bMainMenuVisible = true;
	if (MainMenuOverlay.IsValid())
	{
		MainMenuOverlay->SetVisibility(EVisibility::Visible);
	}
	if (APlayerController* PC = GetOwningPlayerController())
	{
		FInputModeUIOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void ASurvivorHUD::HideMainMenu()
{
	bMainMenuVisible = false;
	if (MainMenuOverlay.IsValid())
	{
		MainMenuOverlay->SetVisibility(EVisibility::Collapsed);
	}
}

void ASurvivorHUD::ShowPauseMenu()
{
	bPauseMenuVisible = true;
	if (PauseMenuOverlay.IsValid())
	{
		PauseMenuOverlay->SetVisibility(EVisibility::Visible);
	}
	if (APlayerController* PC = GetOwningPlayerController())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
		PC->SetPause(true);
	}
}

void ASurvivorHUD::HidePauseMenu()
{
	bPauseMenuVisible = false;
	if (PauseMenuOverlay.IsValid())
	{
		PauseMenuOverlay->SetVisibility(EVisibility::Collapsed);
	}
	if (APlayerController* PC = GetOwningPlayerController())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
		PC->SetPause(false);
	}
}

void ASurvivorHUD::TogglePauseMenu()
{
	if (bMainMenuVisible) return;

	if (bPauseMenuVisible)
	{
		HidePauseMenu();
	}
	else
	{
		ShowPauseMenu();
	}
}

void ASurvivorHUD::ShowDeathScreen()
{
	bDeathScreenVisible = true;
}

void ASurvivorHUD::HideDeathScreen()
{
	bDeathScreenVisible = false;
}

void ASurvivorHUD::HideAllMenus()
{
	HideMainMenu();
	HidePauseMenu();
	HideDeathScreen();
}

// ==================================================================
// HUD Prompts
// ==================================================================

void ASurvivorHUD::ShowInteractionPrompt(const FText& ActionText, const FText& ObjectName)
{
	bShowInteraction = true;
	InteractActionText = ActionText;
	InteractObjectText = ObjectName;
}

void ASurvivorHUD::HideInteractionPrompt()
{
	bShowInteraction = false;
}

void ASurvivorHUD::ShowNotification(const FText& Message, float Duration)
{
	FHUDNotification Notif;
	Notif.Message = Message;
	Notif.TimeRemaining = Duration;
	ActiveNotifications.Add(Notif);
}

// ==================================================================
// Queries
// ==================================================================

bool ASurvivorHUD::IsAnyMenuOpen() const
{
	return bMainMenuVisible || bPauseMenuVisible || bDeathScreenVisible;
}

UPlayerStatsComponent* ASurvivorHUD::GetPlayerStats() const
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return nullptr;

	ASurvivorCharacter* Character = Cast<ASurvivorCharacter>(PC->GetPawn());
	if (!Character) return nullptr;

	return Character->StatsComponent;
}
