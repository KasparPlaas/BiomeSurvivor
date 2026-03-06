// CompassWidget.h - HUD compass bar with marker icons and cardinal directions
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MapMarkerTypes.h"
#include "CompassWidget.generated.h"

class UMapComponent;
class UImage;
class UOverlay;
class UCanvasPanel;
class UTextBlock;

USTRUCT(BlueprintType)
struct FCompassMarkerWidget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FGuid MarkerId;
    UPROPERTY(BlueprintReadOnly) UImage* IconWidget = nullptr;
    UPROPERTY(BlueprintReadOnly) UTextBlock* DistanceLabel = nullptr;
    UPROPERTY(BlueprintReadOnly) float AngleToMarker = 0.0f;
    UPROPERTY(BlueprintReadOnly) float DistanceToMarker = 0.0f;
};

UCLASS()
class BIOMESURVIVOR_API UCompassWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Compass")
    void InitializeCompass(UMapComponent* InMapComponent);

    UFUNCTION(BlueprintCallable, Category = "Compass")
    void SetCompassVisible(bool bVisible);

protected:
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UCanvasPanel* CompassCanvas;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UImage* CompassStrip;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* BearingText;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* DirectionText;

    // ─── Settings ───

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass")
    float CompassWidth = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass")
    float FieldOfView = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass")
    float MarkerIconSize = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass")
    float MaxDisplayDistance = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass")
    bool bShowDistanceLabels = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass")
    bool bShowBearing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass")
    UTexture2D* DefaultMarkerIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass")
    TMap<EMapMarkerType, UTexture2D*> MarkerTypeIcons;

private:
    UPROPERTY() UMapComponent* MapComponent;
    UPROPERTY() TArray<FCompassMarkerWidget> ActiveMarkerWidgets;

    void UpdateCompassStrip();
    void UpdateMarkers();
    void UpdateBearing();

    float NormalizeAngle(float Angle) const;
    FString GetCardinalDirection(float Bearing) const;
    float GetScreenPositionForAngle(float Angle) const;
};
