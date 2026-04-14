// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/LayeredMovementMode/XMoveU_LayeredMovementMode.h"

#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_LayeredMovementMode::UXMoveU_LayeredMovementMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UXMoveU_ModularMovementComponent* UXMoveU_LayeredMovementMode::GetOwningMoveComp() const
{
	return Cast<UXMoveU_ModularMovementComponent>(GetOuter());
}

AXMoveU_ModularCharacter* UXMoveU_LayeredMovementMode::GetOwningCharacter() const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	return MoveComp ? Cast<AXMoveU_ModularCharacter>(MoveComp->GetCharacterOwner()) : nullptr;
}
