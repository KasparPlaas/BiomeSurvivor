// Copyright Biome Survivor. All Rights Reserved.

#include "Combat/WeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
	WeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));

	SetReplicates(true);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponBase::AttachToCharacter(ACharacter* Character, FName SocketName)
{
	if (!Character) return;

	AttachToComponent(
		Character->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		SocketName
	);
}

void AWeaponBase::DetachFromCharacter()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}
