// CompassWidget.cpp - HUD compass bar implementation
#include "UI/CompassWidget.h"
#include "UI/MapComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/Texture2D.h"

void UCompassWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UCompassWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!MapComponent) return;

    UpdateCompassStrip();
    UpdateMarkers();
    UpdateBearing();
}

void UCompassWidget::InitializeCompass(UMapComponent* InMapComponent)
{
    MapComponent = InMapComponent;
}

void UCompassWidget::SetCompassVisible(bool bVisible)
{
    SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UCompassWidget::UpdateCompassStrip()
{
    if (!CompassStrip || !MapComponent) return;

    // Scroll the compass strip texture based on bearing
    const float Bearing = MapComponent->GetCompassBearing();
    const float NormalizedBearing = Bearing / 360.0f;

    // UMG material parameter for strip scrolling
    UMaterialInstanceDynamic* MatInst = CompassStrip->GetDynamicMaterial();
    if (MatInst)
    {
        MatInst->SetScalarParameterValue(TEXT("Offset"), NormalizedBearing);
    }
}

void UCompassWidget::UpdateMarkers()
{
    if (!MapComponent || !CompassCanvas) return;

    TArray<FMapMarkerData> CompassMarkers = MapComponent->GetCompassMarkers();
    const FVector OwnerLoc = MapComponent->GetOwner()->GetActorLocation();

    // Remove stale marker widgets
    for (int32 i = ActiveMarkerWidgets.Num() - 1; i >= 0; i--)
    {
        bool bFound = false;
        for (const FMapMarkerData& M : CompassMarkers)
        {
            if (M.MarkerId == ActiveMarkerWidgets[i].MarkerId)
            {
                bFound = true;
                break;
            }
        }

        if (!bFound)
        {
            if (ActiveMarkerWidgets[i].IconWidget)
            {
                ActiveMarkerWidgets[i].IconWidget->RemoveFromParent();
            }
            if (ActiveMarkerWidgets[i].DistanceLabel)
            {
                ActiveMarkerWidgets[i].DistanceLabel->RemoveFromParent();
            }
            ActiveMarkerWidgets.RemoveAt(i);
        }
    }

    // Update or create marker widgets
    for (const FMapMarkerData& M : CompassMarkers)
    {
        const float Dist = FVector::Dist(OwnerLoc, M.WorldLocation);
        if (Dist > MaxDisplayDistance) continue;

        const float BearingToMarker = MapComponent->GetBearingToMarker(M.MarkerId);
        const float ScreenPos = GetScreenPositionForAngle(BearingToMarker);

        // Skip if outside compass FOV
        if (FMath::Abs(BearingToMarker) > FieldOfView * 0.5f) continue;

        // Find or create widget
        FCompassMarkerWidget* Existing = nullptr;
        for (FCompassMarkerWidget& W : ActiveMarkerWidgets)
        {
            if (W.MarkerId == M.MarkerId)
            {
                Existing = &W;
                break;
            }
        }

        if (!Existing)
        {
            // Create new marker widget
            FCompassMarkerWidget NewWidget;
            NewWidget.MarkerId = M.MarkerId;

            NewWidget.IconWidget = WidgetTree->ConstructWidget<UImage>();
            if (NewWidget.IconWidget)
            {
                UCanvasPanelSlot* MarkerSlot = CompassCanvas->AddChildToCanvas(NewWidget.IconWidget);
                if (MarkerSlot)
                {
                    MarkerSlot->SetSize(FVector2D(MarkerIconSize, MarkerIconSize));
                    MarkerSlot->SetAlignment(FVector2D(0.5f, 0.5f));
                }

                UTexture2D* Icon = M.MarkerIcon;
                if (!Icon)
                {
                    if (UTexture2D** TypeIcon = MarkerTypeIcons.Find(M.MarkerType))
                    {
                        Icon = *TypeIcon;
                    }
                    else
                    {
                        Icon = DefaultMarkerIcon;
                    }
                }

                if (Icon)
                {
                    NewWidget.IconWidget->SetBrushFromTexture(Icon);
                }
                NewWidget.IconWidget->SetColorAndOpacity(M.MarkerColor);
            }

            if (bShowDistanceLabels)
            {
                NewWidget.DistanceLabel = WidgetTree->ConstructWidget<UTextBlock>();
                if (NewWidget.DistanceLabel)
                {
                    UCanvasPanelSlot* LabelSlot = CompassCanvas->AddChildToCanvas(NewWidget.DistanceLabel);
                    if (LabelSlot)
                    {
                        LabelSlot->SetAlignment(FVector2D(0.5f, 0.0f));
                    }
                }
            }

            ActiveMarkerWidgets.Add(NewWidget);
            Existing = &ActiveMarkerWidgets.Last();
        }

        // Update position
        if (Existing->IconWidget)
        {
            UCanvasPanelSlot* IconSlot = Cast<UCanvasPanelSlot>(Existing->IconWidget->Slot);
            if (IconSlot)
            {
                IconSlot->SetPosition(FVector2D(ScreenPos, 0.0f));
            }

            // Fade based on distance
            const float Alpha = FMath::Clamp(1.0f - (Dist / MaxDisplayDistance), 0.2f, 1.0f);
            Existing->IconWidget->SetRenderOpacity(Alpha);
        }

        // Update distance label
        if (Existing->DistanceLabel && bShowDistanceLabels)
        {
            const float DistMeters = Dist / 100.0f; // UE units to meters
            FString DistStr;
            if (DistMeters > 1000.0f)
            {
                DistStr = FString::Printf(TEXT("%.1fkm"), DistMeters / 1000.0f);
            }
            else
            {
                DistStr = FString::Printf(TEXT("%.0fm"), DistMeters);
            }
            Existing->DistanceLabel->SetText(FText::FromString(DistStr));

            UCanvasPanelSlot* LabelSlot = Cast<UCanvasPanelSlot>(Existing->DistanceLabel->Slot);
            if (LabelSlot)
            {
                LabelSlot->SetPosition(FVector2D(ScreenPos, MarkerIconSize + 2.0f));
            }
        }

        Existing->AngleToMarker = BearingToMarker;
        Existing->DistanceToMarker = Dist;
    }
}

void UCompassWidget::UpdateBearing()
{
    if (!MapComponent) return;

    const float Bearing = MapComponent->GetCompassBearing();

    if (BearingText && bShowBearing)
    {
        BearingText->SetText(FText::FromString(FString::Printf(TEXT("%.0f°"), Bearing)));
    }

    if (DirectionText)
    {
        DirectionText->SetText(FText::FromString(GetCardinalDirection(Bearing)));
    }
}

float UCompassWidget::NormalizeAngle(float Angle) const
{
    while (Angle > 180.0f) Angle -= 360.0f;
    while (Angle < -180.0f) Angle += 360.0f;
    return Angle;
}

FString UCompassWidget::GetCardinalDirection(float Bearing) const
{
    if (Bearing < 0) Bearing += 360.0f;

    if (Bearing >= 337.5f || Bearing < 22.5f)   return TEXT("N");
    if (Bearing >= 22.5f && Bearing < 67.5f)    return TEXT("NE");
    if (Bearing >= 67.5f && Bearing < 112.5f)   return TEXT("E");
    if (Bearing >= 112.5f && Bearing < 157.5f)  return TEXT("SE");
    if (Bearing >= 157.5f && Bearing < 202.5f)  return TEXT("S");
    if (Bearing >= 202.5f && Bearing < 247.5f)  return TEXT("SW");
    if (Bearing >= 247.5f && Bearing < 292.5f)  return TEXT("W");
    if (Bearing >= 292.5f && Bearing < 337.5f)  return TEXT("NW");
    return TEXT("N");
}

float UCompassWidget::GetScreenPositionForAngle(float Angle) const
{
    // Map angle from [-FOV/2 ... +FOV/2] to [0 ... CompassWidth]
    const float HalfFOV = FieldOfView * 0.5f;
    const float Normalized = (Angle + HalfFOV) / FieldOfView;
    return Normalized * CompassWidth;
}
