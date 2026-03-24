// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/XMoveU_ModularCharacter.h"

#include "ModularMovement/XMoveU_ModularMovementComponent.h"


AXMoveU_ModularCharacter::AXMoveU_ModularCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UXMoveU_ModularMovementComponent>(ACharacter::CharacterMovementComponentName))
{
}
