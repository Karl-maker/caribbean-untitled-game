#pragma once

#include "CoreMinimal.h"
#include "HudWidgetComponent.h"
#include "LifeHudWidgetComponent.generated.h"

class UImage;
class UOverlay;
class UStatsComponent;
class UTexture2D;
class UWidgetTree;
class UUserWidget;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class CARIBBEANUNTITLED_API ULifeHudWidgetComponent : public UHudWidgetComponent
{
    GENERATED_BODY()

public:
    ULifeHudWidgetComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD")
    bool bDrawBackgroundTexture = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD")
    FVector2D ScreenOffset = FVector2D(24.0f, -24.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD")
    FVector2D WidgetSize = FVector2D(720.0f, 216.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD")
    float TextureScale = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD")
    FVector2D BarOffset = FVector2D(150.0f, 98.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD")
    FVector2D BarSize = FVector2D(520.0f, 32.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD")
    FLinearColor BarColor = FLinearColor(0.96f, 0.92f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD")
    TObjectPtr<UTexture2D> LifeTexture;

    virtual void BuildWidget(UUserWidget* OwnerWidget, UWidgetTree* WidgetTree, UOverlay* LayerContainer) override;
};
