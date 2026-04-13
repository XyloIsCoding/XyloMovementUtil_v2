// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/JumpProfile/Preset/XMoveU_GlideJump.h"

#include "ModularMovement/XMoveU_JumpStaticLibrary.h"
#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_GlideJump::UXMoveU_GlideJump()
{
	InitialVelocity = 400.f;
	
	GravityCompensation = 0.8f;

	HorizontalAcceleration = 1000.f;
	MaxHorizontalVelocity = 600.f;
	
	JumpMaxCount = 10000;
	JumpMaxHoldTime = 10000.f;
}

bool UXMoveU_GlideJump::OverrideInitialImpulse() const
{
	AXMoveU_ModularCharacter* Character = GetOwningCharacter();
	if (!Character) return false;
	
	return Character->JumpCurrentCount >= 1;
}

bool UXMoveU_GlideJump::JumpInitialImpulse(bool bReplayingMoves, float DeltaTime)
{
	AXMoveU_ModularCharacter* Character = GetOwningCharacter();
	if (!Character || Character->JumpCurrentCount != 1) return false;
	
	GetOwningMovementComponent()->Velocity += GetOwningMovementComponent()->Velocity.GetSafeNormal() * InitialVelocity;
	return true;
}

bool UXMoveU_GlideJump::OverrideSustainImpulse() const
{
	AXMoveU_ModularCharacter* Character = GetOwningCharacter();
	if (!Character) return false;
	
	return Character->JumpCurrentCount >= 2;
}

bool UXMoveU_GlideJump::JumpSustainImpulse(bool bReplayingMoves, float DeltaTime)
{
	UXMoveU_JumpStaticLibrary::ApplyJumpAcceleration(GetOwningMovementComponent(), GetOwningMovementComponent()->GetCurrentAcceleration() / GetOwningMovementComponent()->GetMaxAcceleration(), -GetOwningMovementComponent()->GetGravityZ() * GravityCompensation, HorizontalAcceleration, UE_BIG_NUMBER, MaxHorizontalVelocity, DeltaTime);
	return true;
}
