#pragma once

#include "CoreMinimal.h"
#include "CharacterActor.h"
#include "../Components/CharacterBrainComponent.h"
#include "NonPlayableCharacter.generated.h"


UCLASS()
class CARIBBEANUNTITLED_API ANonPlayableCharacter : public ACharacterActor
{
    GENERATED_BODY()

public:

    ANonPlayableCharacter();

protected:

    // ============================================================
    // NPC COMPONENTS
    // ============================================================

    /**
     * Controls the high-level decisions of this character.
     *
     * The Brain produces intent.
     * Gameplay systems execute that intent.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category="NPC|Components"
    )
    TObjectPtr<UCharacterBrainComponent> BrainComponent;
};
