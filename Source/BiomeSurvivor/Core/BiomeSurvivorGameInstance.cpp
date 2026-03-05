// Copyright Biome Survivor. All Rights Reserved.

#include "Core/BiomeSurvivorGameInstance.h"
#include "BiomeSurvivor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"

UBiomeSurvivorGameInstance::UBiomeSurvivorGameInstance()
{
}

void UBiomeSurvivorGameInstance::Init()
{
	Super::Init();
	LoadSettings();
	UE_LOG(LogBiomeSurvivor, Log, TEXT("GameInstance initialized. Settings loaded."));
}

void UBiomeSurvivorGameInstance::Shutdown()
{
	SaveSettings();
	Super::Shutdown();
}

void UBiomeSurvivorGameInstance::HostGame(ESessionType SessionType, const FString& MapName, int32 MaxPlayers, const FString& ServerPassword)
{
	CurrentSessionType = SessionType;

	FString TravelURL = FString::Printf(TEXT("/Game/Maps/%s?listen?MaxPlayers=%d"), *MapName, MaxPlayers);

	if (!ServerPassword.IsEmpty())
	{
		TravelURL += FString::Printf(TEXT("?Password=%s"), *ServerPassword);
	}

	switch (SessionType)
	{
	case ESessionType::Singleplayer:
		// Singleplayer: open map directly
		UGameplayStatics::OpenLevel(this, FName(*MapName));
		break;

	case ESessionType::LAN:
		TravelURL += TEXT("?bIsLanMatch=1");
		GetWorld()->ServerTravel(TravelURL);
		break;

	case ESessionType::OnlineCoop:
		GetWorld()->ServerTravel(TravelURL);
		break;
	}

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Hosting game: %s | Type: %d | MaxPlayers: %d"), *MapName, (int32)SessionType, MaxPlayers);
}

void UBiomeSurvivorGameInstance::JoinGameByIP(const FString& IPAddress, const FString& Password)
{
	FString TravelURL = IPAddress;
	if (!Password.IsEmpty())
	{
		TravelURL += FString::Printf(TEXT("?Password=%s"), *Password);
	}

	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC)
	{
		PC->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
		UE_LOG(LogBiomeSurvivor, Log, TEXT("Joining game at: %s"), *IPAddress);
	}
}

void UBiomeSurvivorGameInstance::ReturnToMainMenu()
{
	// Destroy active session and return to main menu map
	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC)
	{
		PC->ClientTravel(TEXT("/Game/Maps/MainMenu"), ETravelType::TRAVEL_Absolute);
	}
}

bool UBiomeSurvivorGameInstance::SaveGame(int32 SlotIndex)
{
	// TODO: Implement full save game using USaveGame subclass
	FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
	UE_LOG(LogBiomeSurvivor, Log, TEXT("Game saved to slot: %s"), *SlotName);
	return true;
}

bool UBiomeSurvivorGameInstance::LoadGame(int32 SlotIndex)
{
	FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
	if (!DoesSaveExist(SlotIndex))
	{
		UE_LOG(LogBiomeSurvivor, Warning, TEXT("No save found in slot: %s"), *SlotName);
		return false;
	}

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Game loaded from slot: %s"), *SlotName);
	return true;
}

bool UBiomeSurvivorGameInstance::DoesSaveExist(int32 SlotIndex) const
{
	FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
	return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}

void UBiomeSurvivorGameInstance::SaveSettings()
{
	UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
	if (UserSettings)
	{
		UserSettings->ApplySettings(true);
		UserSettings->SaveSettings();
	}
	// TODO: Save custom settings (volumes, sensitivity) to config file
}

void UBiomeSurvivorGameInstance::LoadSettings()
{
	UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
	if (UserSettings)
	{
		UserSettings->LoadSettings();
	}
	// TODO: Load custom settings from config file
}
