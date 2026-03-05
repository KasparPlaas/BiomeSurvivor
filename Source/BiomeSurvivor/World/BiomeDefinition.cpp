// Copyright Biome Survivor. All Rights Reserved.

#include "World/BiomeDefinition.h"

float UBiomeDefinition::GetTemperatureAtHour(float Hour) const
{
	// Day peak at 14:00, night low at 02:00
	// Use cosine curve for smooth interpolation
	const float PeakHour = 14.0f;
	const float CycleFraction = FMath::Fmod(Hour - PeakHour + 24.0f, 24.0f) / 24.0f;
	const float CosValue = FMath::Cos(CycleFraction * 2.0f * PI);

	// Map cosine [1, -1] to [DayTemp, NightTemp]
	const float Alpha = (CosValue + 1.0f) / 2.0f;
	return FMath::Lerp(NightTemperature, DayTemperature, Alpha);
}

FPrimaryAssetId UBiomeDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("BiomeDefinition", BiomeID);
}
