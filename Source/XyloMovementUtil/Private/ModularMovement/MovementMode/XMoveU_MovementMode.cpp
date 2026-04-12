// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/MovementMode/XMoveU_MovementMode.h"

#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_MovementMode::UXMoveU_MovementMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UXMoveU_ModularMovementComponent* UXMoveU_MovementMode::GetOwningMoveComp() const
{
	return Cast<UXMoveU_ModularMovementComponent>(GetOuter());
}

AXMoveU_ModularCharacter* UXMoveU_MovementMode::GetOwningCharacter() const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	return MoveComp ? Cast<AXMoveU_ModularCharacter>(MoveComp->GetCharacterOwner()) : nullptr;
}
