// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/JumpProfile/Preset/XMoveU_ThrusterJump.h"

#include "ModularMovement/XMoveU_JumpStaticLibrary.h"
#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_ThrusterJump::UXMoveU_ThrusterJump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InitialVerticalVelocity = 200.f;
	InitialHorizontalVelocity = 0.f;

	VerticalAcceleration = 5000.f;
	MaxVerticalVelocity = 700.f;
	MinVerticalVelocity = -300.f;

	HorizontalAcceleration = 1000.f;
	MaxHorizontalVelocity = 600.f;

	JumpMaxCount = 10000;
	JumpMaxHoldTime = 10000.f;
}

bool UXMoveU_ThrusterJump::OverrideInitialImpulse() const
{
	AXMoveU_ModularCharacter* Character = GetOwningCharacter();
	if (!Character) return false;
	
	return Character->JumpCurrentCount >= 1;
}

bool UXMoveU_ThrusterJump::JumpInitialImpulse(bool bReplayingMoves, float DeltaTime)
{
	UXMoveU_JumpStaticLibrary::LimitMinVerticalVelocity(GetOwningMovementComponent(), MinVerticalVelocity);
	UXMoveU_JumpStaticLibrary::ApplyJumpImpulse(GetOwningMovementComponent(), GetOwningMovementComponent()->GetCurrentAcceleration() / GetOwningMovementComponent()->GetMaxAcceleration(), InitialVerticalVelocity, InitialHorizontalVelocity, false);
	return true;
}

bool UXMoveU_ThrusterJump::OverrideSustainImpulse() const
{
	AXMoveU_ModularCharacter* Character = GetOwningCharacter();
	if (!Character) return false;
	
	return Character->JumpCurrentCount >= 2;
}

bool UXMoveU_ThrusterJump::JumpSustainImpulse(bool bReplayingMoves, float DeltaTime)
{
	UXMoveU_JumpStaticLibrary::ApplyJumpAcceleration(GetOwningMovementComponent(), GetOwningMovementComponent()->GetCurrentAcceleration() / GetOwningMovementComponent()->GetMaxAcceleration(), VerticalAcceleration, HorizontalAcceleration, MaxVerticalVelocity, MaxHorizontalVelocity, DeltaTime);
	return true;
}
