//
//  PlayerActor.cpp
//  CaribbeanUntitled (Mac)
//
//  Created by Karl-Johan Bailey on 8/13/26.
//  Copyright © 2026 Epic Games, Inc. All rights reserved.
//


#include "PlayerActor.h"

#include "../Components/ControlComponent.h"

APlayerActor::APlayerActor()
{
    ControlComponent =
        CreateDefaultSubobject<UControlComponent>(
            TEXT("ControlComponent")
        );


}
