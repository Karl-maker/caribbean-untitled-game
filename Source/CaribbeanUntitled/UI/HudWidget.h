#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HudWidget.generated.h"

class UHorizontalBox;
class UHudWidgetComponent;
class UCanvasPanel;
class UOverlay;
class UWidgetTree;

UCLASS()
class CARIBBEANUNTITLED_API UHudWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="HUD")
    void RebuildHud();

    UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category="HUD")
    TArray<TObjectPtr<UHudWidgetComponent>> Components;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;

    UWidgetTree* GetHudWidgetTree() const { return WidgetTree; }

private:
    UPROPERTY(Transient)
    TObjectPtr<UCanvasPanel> RootCanvas;

    UPROPERTY(Transient)
    TObjectPtr<UOverlay> BackgroundLayer;

    UPROPERTY(Transient)
    TObjectPtr<UOverlay> Layer1Overlay;
};
