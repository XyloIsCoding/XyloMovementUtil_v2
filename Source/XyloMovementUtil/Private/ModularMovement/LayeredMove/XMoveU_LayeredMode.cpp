// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/LayeredMove/XMoveU_LayeredMove.h"

#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_LayeredMove::UXMoveU_LayeredMove(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UXMoveU_ModularMovementComponent* UXMoveU_LayeredMove::GetOwningMoveComp() const
{
	return Cast<UXMoveU_ModularMovementComponent>(GetOuter());
}

AXMoveU_ModularCharacter* UXMoveU_LayeredMove::GetOwningCharacter() const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	return MoveComp ? Cast<AXMoveU_ModularCharacter>(MoveComp->GetCharacterOwner()) : nullptr;
}
