// Copyright Biome Survivor. All Rights Reserved.

#include "BiomeSurvivor.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogBiomeSurvivor);

void FBiomeSurvivorModule::StartupModule()
{
	UE_LOG(LogBiomeSurvivor, Log, TEXT("BiomeSurvivor module started."));
}

void FBiomeSurvivorModule::ShutdownModule()
{
	UE_LOG(LogBiomeSurvivor, Log, TEXT("BiomeSurvivor module shut down."));
}

IMPLEMENT_PRIMARY_GAME_MODULE(FBiomeSurvivorModule, BiomeSurvivor, "BiomeSurvivor");
