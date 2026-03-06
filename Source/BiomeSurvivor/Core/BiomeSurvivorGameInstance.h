// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BiomeSurvivorGameInstance.generated.h"

/**
 * UBiomeSurvivorGameInstance
 *
 * Persistent across level loads. Manages:
 * - Player profile / save data
 * - Audio settings
 * - Graphics settings
 * - Session management (singleplayer, LAN, online)
 * - Transition between menus and gameplay
 */
UENUM(BlueprintType)
enum class ESessionType : uint8
{
	Singleplayer  UMETA(DisplayName = "Singleplayer"),
	LAN           UMETA(DisplayName = "LAN"),
	OnlineCoop    UMETA(DisplayName = "Online Co-op")
};

UCLASS(Blueprintable)
class BIOMESURVIVOR_API UBiomeSurvivorGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UBiomeSurvivorGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	// ---- Session Management ----

	/** Host a game session. */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void HostGame(ESessionType SessionType, const FString& MapName, int32 MaxPlayers = 8, const FString& ServerPassword = TEXT(""));

	/** Join a game by IP address (LAN or direct connect). */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinGameByIP(const FString& IPAddress, const FString& Password = TEXT(""));

	/** Return to main menu, destroying current session. */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void ReturnToMainMenu();

	/** Current session type. */
	UPROPERTY(BlueprintReadOnly, Category = "Session")
	ESessionType CurrentSessionType = ESessionType::Singleplayer;

	// ---- Save System ----

	/** Save current game to a slot (singleplayer). */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveGame(int32 SlotIndex);

	/** Load a game from a slot (singleplayer). */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadGame(int32 SlotIndex);

	/** Check if a save exists in the given slot. */
	UFUNCTION(BlueprintPure, Category = "Save")
	bool DoesSaveExist(int32 SlotIndex) const;

	// ---- Settings ----

	/** Master volume (0.0 - 1.0). */
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
	float MasterVolume = 1.0f;

	/** Music volume (0.0 - 1.0). */
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
	float MusicVolume = 0.7f;

	/** SFX volume (0.0 - 1.0). */
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
	float SFXVolume = 1.0f;

	/** Mouse sensitivity. */
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Controls")
	float MouseSensitivity = 1.0f;

	/** Field of view. */
	UPROPERTY(BlueprintReadWrite, Category = "Settings|Graphics")
	float FieldOfView = 90.0f;

	/** Save settings to disk. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SaveSettings();

	/** Load settings from disk. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadSettings();
};
