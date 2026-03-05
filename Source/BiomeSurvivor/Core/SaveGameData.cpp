// SaveGameData.cpp - Save game data implementation
#include "Core/SaveGameData.h"

USaveGameData::USaveGameData()
{
    SaveSlotName = TEXT("BiomeSurvivor_Save");
    UserIndex = 0;
    SaveTimestamp = FDateTime::Now();
    GameVersion = TEXT("0.1.0");
    SaveVersion = 1;
    TotalPlayTimeSeconds = 0.0f;
    MapName = TEXT("MainWorld");
}
