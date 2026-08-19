#include "PlayerHudController.h"

#include "HudWidget.h"
#include "LifeHudWidgetComponent.h"
#include "Blueprint/UserWidget.h"

APlayerHudController::APlayerHudController()
{
    bShowMouseCursor = false;
    LifeComponent = CreateDefaultSubobject<ULifeHudWidgetComponent>(TEXT("LifeComponent"));
}

void APlayerHudController::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalController())
    {
        CreateHud();
    }
}

void APlayerHudController::CreateHud()
{
    if (!HudWidgetClass)
    {
        HudWidgetClass = UHudWidget::StaticClass();
    }

    if (!ActiveHudWidget && HudWidgetClass)
    {
        ActiveHudWidget = CreateWidget<UHudWidget>(this, HudWidgetClass);
        if (ActiveHudWidget)
        {
            if (LifeComponent)
            {
                ActiveHudWidget->Components.Add(LifeComponent);
            }

            ActiveHudWidget->AddToViewport();
            ActiveHudWidget->RebuildHud();
        }
    }
}
