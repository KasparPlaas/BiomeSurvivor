// MapComponent.cpp - Map/compass system implementation
#include "UI/MapComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UMapComponent::UMapComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

void UMapComponent::BeginPlay()
{
    Super::BeginPlay();
    LastExplorationPosition = GetOwner()->GetActorLocation();
}

void UMapComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateDynamicMarkers();

    ExplorationTimer += DeltaTime;
    if (ExplorationTimer >= ExplorationUpdateInterval)
    {
        ExplorationTimer = 0.0f;
        UpdateExploration();
    }
}

// ─── Marker Management ────────────────────────────────────────────

FGuid UMapComponent::AddMarker(const FMapMarkerData& MarkerData)
{
    if (Markers.Num() >= MaxMarkers) return FGuid();

    FMapMarkerData NewMarker = MarkerData;
    if (!NewMarker.MarkerId.IsValid())
    {
        NewMarker.MarkerId = FGuid::NewGuid();
    }

    if (NewMarker.MarkerColor == FLinearColor::White)
    {
        NewMarker.MarkerColor = GetDefaultColorForType(NewMarker.MarkerType);
    }

    NewMarker.DiscoveryTimestamp = GetWorld()->GetTimeSeconds();

    Markers.Add(NewMarker);
    OnMarkerAdded.Broadcast(NewMarker);

    return NewMarker.MarkerId;
}

bool UMapComponent::RemoveMarker(FGuid MarkerId)
{
    for (int32 i = Markers.Num() - 1; i >= 0; i--)
    {
        if (Markers[i].MarkerId == MarkerId)
        {
            Markers.RemoveAt(i);
            OnMarkerRemoved.Broadcast(MarkerId);
            return true;
        }
    }
    return false;
}

bool UMapComponent::UpdateMarkerLocation(FGuid MarkerId, FVector NewLocation)
{
    FMapMarkerData* Marker = FindMarker(MarkerId);
    if (!Marker) return false;

    Marker->WorldLocation = NewLocation;
    OnMarkerUpdated.Broadcast(*Marker);
    return true;
}

bool UMapComponent::SetMarkerVisibility(FGuid MarkerId, bool bVisible)
{
    FMapMarkerData* Marker = FindMarker(MarkerId);
    if (!Marker) return false;

    Marker->bIsVisible = bVisible;
    OnMarkerUpdated.Broadcast(*Marker);
    return true;
}

void UMapComponent::AddPlayerMarker(FVector Location, FText Name, EMapMarkerType Type)
{
    FMapMarkerData Data;
    Data.WorldLocation = Location;
    Data.DisplayName = Name;
    Data.MarkerType = Type;
    Data.bShowOnCompass = true;
    Data.bShowOnMap = true;
    AddMarker(Data);
}

void UMapComponent::AddDeathMarker(FVector Location)
{
    // Enforce max death markers - remove oldest
    TArray<int32> DeathIndices;
    for (int32 i = 0; i < Markers.Num(); i++)
    {
        if (Markers[i].MarkerType == EMapMarkerType::Death)
        {
            DeathIndices.Add(i);
        }
    }

    while (DeathIndices.Num() >= MaxDeathMarkers)
    {
        int32 OldestIdx = DeathIndices[0];
        OnMarkerRemoved.Broadcast(Markers[OldestIdx].MarkerId);
        Markers.RemoveAt(OldestIdx);
        DeathIndices.RemoveAt(0);
        // Adjust remaining indices
        for (int32& Idx : DeathIndices) Idx--;
    }

    FMapMarkerData Data;
    Data.WorldLocation = Location;
    Data.DisplayName = FText::FromString("Death");
    Data.MarkerType = EMapMarkerType::Death;
    Data.MarkerColor = FLinearColor::Red;
    Data.bShowOnCompass = true;
    Data.bShowOnMap = true;
    AddMarker(Data);
}

void UMapComponent::ClearAllMarkers()
{
    for (const FMapMarkerData& M : Markers)
    {
        OnMarkerRemoved.Broadcast(M.MarkerId);
    }
    Markers.Empty();
}

void UMapComponent::ClearMarkersOfType(EMapMarkerType Type)
{
    for (int32 i = Markers.Num() - 1; i >= 0; i--)
    {
        if (Markers[i].MarkerType == Type)
        {
            OnMarkerRemoved.Broadcast(Markers[i].MarkerId);
            Markers.RemoveAt(i);
        }
    }
}

// ─── Queries ──────────────────────────────────────────────────────

TArray<FMapMarkerData> UMapComponent::GetVisibleMarkers() const
{
    TArray<FMapMarkerData> Result;
    const FVector OwnerLoc = GetOwner()->GetActorLocation();

    for (const FMapMarkerData& M : Markers)
    {
        if (!M.bIsVisible) continue;
        if (M.VisibilityRange > 0.0f)
        {
            if (FVector::Dist(OwnerLoc, M.WorldLocation) > M.VisibilityRange) continue;
        }
        Result.Add(M);
    }
    return Result;
}

TArray<FMapMarkerData> UMapComponent::GetMarkersOfType(EMapMarkerType Type) const
{
    TArray<FMapMarkerData> Result;
    for (const FMapMarkerData& M : Markers)
    {
        if (M.MarkerType == Type) Result.Add(M);
    }
    return Result;
}

TArray<FMapMarkerData> UMapComponent::GetCompassMarkers() const
{
    TArray<FMapMarkerData> Result;
    const FVector OwnerLoc = GetOwner()->GetActorLocation();

    for (const FMapMarkerData& M : Markers)
    {
        if (!M.bIsVisible || !M.bShowOnCompass) continue;
        const float Dist = FVector::Dist(OwnerLoc, M.WorldLocation);
        if (Dist <= CompassVisibilityRange)
        {
            Result.Add(M);
        }
    }
    return Result;
}

FMapMarkerData UMapComponent::GetNearestMarker(FVector Location, EMapMarkerType Type) const
{
    float BestDist = FLT_MAX;
    FMapMarkerData BestMarker;

    for (const FMapMarkerData& M : Markers)
    {
        if (Type != EMapMarkerType::Custom && M.MarkerType != Type) continue;
        const float Dist = FVector::Dist(Location, M.WorldLocation);
        if (Dist < BestDist)
        {
            BestDist = Dist;
            BestMarker = M;
        }
    }
    return BestMarker;
}

float UMapComponent::GetCompassBearing() const
{
    if (!GetOwner()) return 0.0f;

    const FRotator Rotation = GetOwner()->GetActorRotation();
    float Yaw = Rotation.Yaw;
    if (Yaw < 0.0f) Yaw += 360.0f;
    return Yaw;
}

FVector2D UMapComponent::GetPlayerMapPosition() const
{
    if (!GetOwner()) return FVector2D::ZeroVector;

    const FVector Loc = GetOwner()->GetActorLocation();
    const FVector2D WorldRange = MapWorldMax - MapWorldMin;

    if (WorldRange.X <= 0 || WorldRange.Y <= 0) return FVector2D(0.5f, 0.5f);

    return FVector2D(
        (Loc.X - MapWorldMin.X) / WorldRange.X,
        (Loc.Y - MapWorldMin.Y) / WorldRange.Y
    );
}

float UMapComponent::GetDistanceToMarker(FGuid MarkerId) const
{
    for (const FMapMarkerData& M : Markers)
    {
        if (M.MarkerId == MarkerId)
        {
            return FVector::Dist(GetOwner()->GetActorLocation(), M.WorldLocation);
        }
    }
    return -1.0f;
}

float UMapComponent::GetBearingToMarker(FGuid MarkerId) const
{
    for (const FMapMarkerData& M : Markers)
    {
        if (M.MarkerId != MarkerId) continue;

        const FVector OwnerLoc = GetOwner()->GetActorLocation();
        const FVector Dir = (M.WorldLocation - OwnerLoc).GetSafeNormal2D();
        const FVector Forward = GetOwner()->GetActorForwardVector().GetSafeNormal2D();

        float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, Dir)));

        // Determine left/right
        const FVector Cross = FVector::CrossProduct(Forward, Dir);
        if (Cross.Z < 0) Angle = -Angle;

        return Angle;
    }
    return 0.0f;
}

// ─── Fog of War ───────────────────────────────────────────────────

bool UMapComponent::IsRegionExplored(FVector2D Location) const
{
    for (const FExploredRegion& Region : ExploredRegions)
    {
        if (FVector2D::Distance(Location, Region.Center) <= Region.Radius)
        {
            return true;
        }
    }
    return false;
}

float UMapComponent::GetExplorationPercentage() const
{
    // Approximate: calculate covered area vs total area
    const FVector2D WorldRange = MapWorldMax - MapWorldMin;
    const float TotalArea = WorldRange.X * WorldRange.Y;
    if (TotalArea <= 0) return 0.0f;

    // Sum up exploration circles (with overlap this overestimates, but good enough)
    float ExploredArea = 0.0f;
    for (const FExploredRegion& Region : ExploredRegions)
    {
        ExploredArea += PI * Region.Radius * Region.Radius;
    }

    return FMath::Clamp(ExploredArea / TotalArea * 100.0f, 0.0f, 100.0f);
}

// ─── Private ──────────────────────────────────────────────────────

void UMapComponent::UpdateExploration()
{
    if (!GetOwner()) return;

    const FVector CurrentLoc = GetOwner()->GetActorLocation();
    const FVector2D Loc2D(CurrentLoc.X, CurrentLoc.Y);

    // Only add new region if we've moved far enough from last point
    const float DistFromLast = FVector::Dist(CurrentLoc, LastExplorationPosition);
    if (DistFromLast < ExplorationRadius * 0.5f) return;

    // Check if already explored nearby
    bool bAlreadyExplored = false;
    for (const FExploredRegion& R : ExploredRegions)
    {
        if (FVector2D::Distance(Loc2D, R.Center) < ExplorationRadius * 0.5f)
        {
            bAlreadyExplored = true;
            break;
        }
    }

    if (!bAlreadyExplored)
    {
        FExploredRegion NewRegion;
        NewRegion.Center = Loc2D;
        NewRegion.Radius = ExplorationRadius;
        NewRegion.DiscoveryTime = GetWorld()->GetTimeSeconds();
        ExploredRegions.Add(NewRegion);

        OnRegionExplored.Broadcast(NewRegion);
    }

    LastExplorationPosition = CurrentLoc;
}

void UMapComponent::UpdateDynamicMarkers()
{
    for (FMapMarkerData& M : Markers)
    {
        if (!M.bIsDynamic || !M.TrackedActor) continue;

        if (IsValid(M.TrackedActor))
        {
            M.WorldLocation = M.TrackedActor->GetActorLocation();
        }
        else
        {
            M.bIsVisible = false;
            M.bIsDynamic = false;
            M.TrackedActor = nullptr;
        }
    }
}

FMapMarkerData* UMapComponent::FindMarker(FGuid MarkerId)
{
    for (FMapMarkerData& M : Markers)
    {
        if (M.MarkerId == MarkerId) return &M;
    }
    return nullptr;
}

FLinearColor UMapComponent::GetDefaultColorForType(EMapMarkerType Type) const
{
    switch (Type)
    {
    case EMapMarkerType::PlayerBase:      return FLinearColor(0.2f, 0.6f, 1.0f);
    case EMapMarkerType::PointOfInterest: return FLinearColor(1.0f, 0.85f, 0.0f);
    case EMapMarkerType::Resource:        return FLinearColor(0.3f, 0.9f, 0.3f);
    case EMapMarkerType::Danger:          return FLinearColor(1.0f, 0.2f, 0.2f);
    case EMapMarkerType::Quest:           return FLinearColor(1.0f, 0.6f, 0.0f);
    case EMapMarkerType::Death:           return FLinearColor(0.8f, 0.0f, 0.0f);
    case EMapMarkerType::TeamMember:      return FLinearColor(0.4f, 0.8f, 1.0f);
    case EMapMarkerType::Animal:          return FLinearColor(0.7f, 0.5f, 0.2f);
    case EMapMarkerType::Shelter:         return FLinearColor(0.5f, 0.7f, 1.0f);
    case EMapMarkerType::WaterSource:     return FLinearColor(0.1f, 0.4f, 0.9f);
    case EMapMarkerType::CraftingStation: return FLinearColor(0.8f, 0.6f, 0.2f);
    default:                              return FLinearColor::White;
    }
}
