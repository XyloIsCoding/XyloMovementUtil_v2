// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/JumpProfile/Preset/XMoveU_ImpulseJump.h"

#include "ModularMovement/XMoveU_JumpStaticLibrary.h"
#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_ImpulseJump::UXMoveU_ImpulseJump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	VerticalVelocity = 1400.f;
	HorizontalVelocity = 600.f;
	
	JumpMaxCount = 2;
	JumpMaxHoldTime = 0.f;
}

bool UXMoveU_ImpulseJump::OverrideInitialImpulse() const
{
	AXMoveU_ModularCharacter* Character = GetOwningCharacter();
	if (!Character) return false;
	
	return Character->JumpCurrentCount >= 1;
}

bool UXMoveU_ImpulseJump::JumpInitialImpulse(bool bReplayingMoves, float DeltaTime)
{
	UXMoveU_JumpStaticLibrary::ApplyJumpImpulse(GetOwningMovementComponent(), GetOwningMovementComponent()->GetCurrentAcceleration() / GetOwningMovementComponent()->GetMaxAcceleration(), VerticalVelocity, HorizontalVelocity, true);
	return true;
}
