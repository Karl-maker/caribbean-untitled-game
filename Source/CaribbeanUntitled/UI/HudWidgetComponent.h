#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HudWidgetComponent.generated.h"

class UHorizontalBox;
class UWidgetTree;
class UWidget;
class UUserWidget;
class UOverlay;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class CARIBBEANUNTITLED_API UHudWidgetComponent : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD")
    int32 Layer = 0;

    virtual void BuildWidget(UUserWidget* OwnerWidget, UWidgetTree* WidgetTree, UOverlay* LayerContainer) {}
};
