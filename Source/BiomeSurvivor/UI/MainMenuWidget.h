// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UEditableTextBox;
class UComboBoxString;
class USlider;

/**
 * Main menu widget with:
 * - Singleplayer / Host Game / Join Game options
 * - Settings (graphics, audio, controls)
 * - Credits, Quit
 * 
 * Designed for Blueprint subclassing with full UMG layout.
 */
UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIOR_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// --- Navigation ---

	/** Show the main menu panel */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowMainPanel();

	/** Show the host game configuration panel */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowHostPanel();

	/** Show the join game panel */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowJoinPanel();

	/** Show the settings panel */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowSettingsPanel();

	// --- Actions ---

	/** Start a singleplayer game */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void StartSingleplayer();

	/** Host a LAN/Online game */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void HostGame(const FString& ServerName, int32 MaxPlayers, bool bIsLAN);

	/** Join a game by IP address */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void JoinByIP(const FString& IPAddress);

	/** Open tutorial/how to play */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void OpenTutorial();

	/** Quit the game */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void QuitGame();

	// --- Settings ---

	/** Apply graphics settings */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyGraphicsSettings(int32 QualityLevel, bool bFullscreen, FIntPoint Resolution, bool bVSync, int32 FPSLimit);

	/** Apply audio settings */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyAudioSettings(float MasterVolume, float MusicVolume, float SFXVolume, float AmbientVolume);

	/** Apply mouse sensitivity */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyMouseSensitivity(float Sensitivity, bool bInvertY);

	/** Save settings to disk */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SaveSettings();

	/** Load settings from disk */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadSettings();

	// --- Events (for Blueprint binding) ---

	UFUNCTION(BlueprintImplementableEvent, Category = "Menu")
	void OnConnectionFailed(const FText& ErrorMessage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Menu")
	void OnLoadingStarted();

protected:
	virtual void NativeConstruct() override;

	/** Current active panel name */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	FName ActivePanel = "Main";
};
