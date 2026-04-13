// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/LayeredMode/XMoveU_LayeredMode.h"

#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_LayeredMode::UXMoveU_LayeredMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UXMoveU_ModularMovementComponent* UXMoveU_LayeredMode::GetOwningMoveComp() const
{
	return Cast<UXMoveU_ModularMovementComponent>(GetOuter());
}

AXMoveU_ModularCharacter* UXMoveU_LayeredMode::GetOwningCharacter() const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	return MoveComp ? Cast<AXMoveU_ModularCharacter>(MoveComp->GetCharacterOwner()) : nullptr;
}
