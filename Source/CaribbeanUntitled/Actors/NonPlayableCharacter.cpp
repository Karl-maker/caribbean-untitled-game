#include "NonPlayableCharacter.h"


ANonPlayableCharacter::ANonPlayableCharacter()
{
    BrainComponent =
        CreateDefaultSubobject<UCharacterBrainComponent>(
            TEXT("BrainComponent")
        );

    if (BrainComponent)
    {
        // ========================================================
        // DEFAULT NPC PERSONALITY
        // ========================================================

        BrainComponent->Personality.Bravery = 0.5f;
        BrainComponent->Personality.Curiosity = 0.5f;
        BrainComponent->Personality.Aggression = 0.5f;
        BrainComponent->Personality.Sociability = 0.5f;

        // ========================================================
        // DEFAULT STATE
        // ========================================================

        BrainComponent->SetFeeling(EBrainFeeling::Neutral);
        BrainComponent->SetIntent(EBrainIntent::Idle, 0.0f);
    }
    
    CharacterPerceptionComponent =
        CreateDefaultSubobject<UCharacterPerceptionComponent>(
            TEXT("PerceptionComponent")
        );
}
