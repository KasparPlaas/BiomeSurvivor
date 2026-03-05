// Copyright Biome Survivor. All Rights Reserved.

#include "UI/MainMenuWidget.h"
#include "Core/BiomeSurvivorGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/GameUserSettings.h"
#include "BiomeSurvivor.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	LoadSettings();
	ShowMainPanel();
}

void UMainMenuWidget::ShowMainPanel()
{
	ActivePanel = "Main";
	// Blueprint handles panel visibility switching
}

void UMainMenuWidget::ShowHostPanel()
{
	ActivePanel = "Host";
}

void UMainMenuWidget::ShowJoinPanel()
{
	ActivePanel = "Join";
}

void UMainMenuWidget::ShowSettingsPanel()
{
	ActivePanel = "Settings";
}

void UMainMenuWidget::StartSingleplayer()
{
	UBiomeSurvivorGameInstance* GI = Cast<UBiomeSurvivorGameInstance>(
		UGameplayStatics::GetGameInstance(GetWorld()));

	if (GI)
	{
		GI->HostGame(ESessionType::Singleplayer, TEXT(""), 1);
		OnLoadingStarted();
	}
}

void UMainMenuWidget::HostGame(const FString& ServerName, int32 MaxPlayers, bool bIsLAN)
{
	UBiomeSurvivorGameInstance* GI = Cast<UBiomeSurvivorGameInstance>(
		UGameplayStatics::GetGameInstance(GetWorld()));

	if (GI)
	{
		ESessionType SessionType = bIsLAN ? ESessionType::LAN : ESessionType::OnlineCoop;
		GI->HostGame(SessionType, ServerName, MaxPlayers);
		OnLoadingStarted();
	}
}

void UMainMenuWidget::JoinByIP(const FString& IPAddress)
{
	UBiomeSurvivorGameInstance* GI = Cast<UBiomeSurvivorGameInstance>(
		UGameplayStatics::GetGameInstance(GetWorld()));

	if (GI)
	{
		GI->JoinGameByIP(IPAddress);
		OnLoadingStarted();
	}
}

void UMainMenuWidget::OpenTutorial()
{
	// Load tutorial map or open tutorial overlay
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Tutorial requested"));
}

void UMainMenuWidget::QuitGame()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}

void UMainMenuWidget::ApplyGraphicsSettings(int32 QualityLevel, bool bFullscreen, FIntPoint Resolution, bool bVSync, int32 FPSLimit)
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return;

	Settings->SetOverallScalabilityLevel(QualityLevel);
	Settings->SetFullscreenMode(bFullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed);
	Settings->SetScreenResolution(Resolution);
	Settings->SetVSyncEnabled(bVSync);
	Settings->SetFrameRateLimit(static_cast<float>(FPSLimit));

	Settings->ApplySettings(false);

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Graphics settings applied: Quality=%d, Fullscreen=%d, Res=%dx%d, VSync=%d, FPS=%d"),
		QualityLevel, bFullscreen, Resolution.X, Resolution.Y, bVSync, FPSLimit);
}

void UMainMenuWidget::ApplyAudioSettings(float MasterVolume, float MusicVolume, float SFXVolume, float AmbientVolume)
{
	// Set sound class volumes through sound mix
	// In full implementation, these would modify USoundMix or USoundClass volumes
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Audio settings: Master=%.2f, Music=%.2f, SFX=%.2f, Ambient=%.2f"),
		MasterVolume, MusicVolume, SFXVolume, AmbientVolume);
}

void UMainMenuWidget::ApplyMouseSensitivity(float Sensitivity, bool bInvertY)
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->InputYawScale_DEPRECATED = Sensitivity;
		PC->InputPitchScale_DEPRECATED = bInvertY ? Sensitivity : -Sensitivity;
	}
}

void UMainMenuWidget::SaveSettings()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (Settings)
	{
		Settings->SaveSettings();
	}
}

void UMainMenuWidget::LoadSettings()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (Settings)
	{
		Settings->LoadSettings();
	}
}
