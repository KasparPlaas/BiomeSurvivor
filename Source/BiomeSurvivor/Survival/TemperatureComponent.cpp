// Copyright Biome Survivor. All Rights Reserved.

#include "Survival/TemperatureComponent.h"
#include "Player/StatusEffectComponent.h"
#include "Inventory/EquipmentComponent.h"
#include "BiomeSurvivor.h"
#include "Net/UnrealNetwork.h"

UTemperatureComponent::UTemperatureComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f; // Once per second
	SetIsReplicatedByDefault(true);
}

void UTemperatureComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTemperatureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()->HasAuthority()) return;

	float EffectiveTemp = GetEffectiveAmbientTemp();

	// Move body temp toward effective ambient
	float TempDiff = EffectiveTemp - BodyTemperature;
	float Change = FMath::Sign(TempDiff) * FMath::Min(FMath::Abs(TempDiff), TempChangeRate * DeltaTime);

	// Body naturally regulates toward 37C
	float RegulationPull = (NormalBodyTemp - BodyTemperature) * 0.02f * DeltaTime;

	float OldTemp = BodyTemperature;
	BodyTemperature += Change + RegulationPull;
	BodyTemperature = FMath::Clamp(BodyTemperature, 25.0f, 45.0f);

	if (FMath::Abs(BodyTemperature - OldTemp) > 0.01f)
	{
		OnTemperatureChanged.Broadcast(BodyTemperature);
	}

	ApplyTemperatureEffects(DeltaTime);
}

void UTemperatureComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UTemperatureComponent, BodyTemperature);
}

float UTemperatureComponent::GetEffectiveAmbientTemp() const
{
	float Temp = AmbientTemperature;

	// Wind chill reduces effective temperature
	Temp -= WindChill * 10.0f;

	// Wetness makes cold worse
	if (Temp < NormalBodyTemp)
	{
		Temp -= Wetness * 5.0f;
	}

	// Heat source warms
	if (bNearHeatSource)
	{
		Temp += HeatSourceWarmth;
	}

	// Clothing warmth
	UEquipmentComponent* Equipment = GetOwner()->FindComponentByClass<UEquipmentComponent>();
	if (Equipment)
	{
		float ClothingWarmth = Equipment->GetTotalWarmth();
		float WindRes = Equipment->GetTotalWindResistance();
		float WaterRes = Equipment->GetTotalWaterResistance();

		Temp += ClothingWarmth;
		// Wind resistance reduces wind chill effect
		Temp += WindChill * 10.0f * WindRes;
		// Water resistance reduces wetness effect
		Temp += Wetness * 5.0f * WaterRes;
	}

	return Temp;
}

FText UTemperatureComponent::GetTemperatureStatusText() const
{
	if (IsHypothermic())
	{
		return FText::FromString(FString::Printf(TEXT("HYPOTHERMIA (%.1f°C)"), BodyTemperature));
	}
	if (IsHyperthermic())
	{
		return FText::FromString(FString::Printf(TEXT("HYPERTHERMIA (%.1f°C)"), BodyTemperature));
	}
	return FText::FromString(FString::Printf(TEXT("%.1f°C"), BodyTemperature));
}

void UTemperatureComponent::ApplyTemperatureEffects(float DeltaTime)
{
	UStatusEffectComponent* StatusEffects = GetOwner()->FindComponentByClass<UStatusEffectComponent>();
	if (!StatusEffects) return;

	if (IsHypothermic())
	{
		float Severity = (HypothermiaThreshold - BodyTemperature) * 5.0f;
		StatusEffects->ApplyStatusEffect(EStatusEffectType::Hypothermia, Severity * DeltaTime);
	}
	else if (StatusEffects->HasStatusEffect(EStatusEffectType::Hypothermia))
	{
		StatusEffects->ReduceEffectSeverity(EStatusEffectType::Hypothermia, 5.0f * DeltaTime);
	}

	if (IsHyperthermic())
	{
		float Severity = (BodyTemperature - HyperthermiaThreshold) * 5.0f;
		StatusEffects->ApplyStatusEffect(EStatusEffectType::Hyperthermia, Severity * DeltaTime);
	}
	else if (StatusEffects->HasStatusEffect(EStatusEffectType::Hyperthermia))
	{
		StatusEffects->ReduceEffectSeverity(EStatusEffectType::Hyperthermia, 5.0f * DeltaTime);
	}
}
