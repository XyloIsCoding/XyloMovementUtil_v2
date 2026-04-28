// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/JumpProfile/Preset/XMoveU_WallJump.h"

#include "XyloMovementUtil.h"
#include "ModularMovement/XMoveU_JumpStaticLibrary.h"
#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"
#include "ModularMovement/MovementMode/XMoveU_MovementMode.h"
#include "ModularMovement/MovementMode/Preset/XMoveU_WallRunMoveMode.h"

bool UXMoveU_WallJump::OverrideInitialImpulse() const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMovementComponent();
	UXMoveU_MovementMode* CurrentMoveMode = MoveComp ? MoveComp->GetCurrentCustomMovementMode() : nullptr;
	return CurrentMoveMode && CurrentMoveMode->GetClass()->IsChildOf<UXMoveU_WallRunMoveMode>();
}

bool UXMoveU_WallJump::JumpInitialImpulse(bool bReplayingMoves, float DeltaTime)
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMovementComponent();
	UXMoveU_WallRunMoveMode* WallRunMode = Cast<UXMoveU_WallRunMoveMode>(MoveComp ? MoveComp->GetCurrentCustomMovementMode() : nullptr);
	if (!WallRunMode)
	{
		UE_LOG(LogXyloMovementUtil, Warning, TEXT("Cannot use UXMoveU_WallJump without being in a movement mode derived from UXMoveU_WallRunMoveMode"))
		return false;
	}
	
	float PushVelocity = WallRunMode->IsClimbing() ? WallPushVelocityClimbing : WallPushVelocity;
	UXMoveU_JumpStaticLibrary::ApplyJumpImpulse(MoveComp, WallRunMode->CurrentWall.LastWallNormal, 0.f, PushVelocity, false);

	if (WallRunMode->CurrentWall.WallHit.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Wall jump with wall"))
		float OldJumpHorizontalVelocity = MoveComp->JumpHorizontalVelocity;
		MoveComp->JumpHorizontalVelocity = 0.f;
		MoveComp->JumpInitialImpulse(GetOwningCharacter()->bClientUpdating, DeltaTime);
		MoveComp->JumpHorizontalVelocity = OldJumpHorizontalVelocity;
	}
	return true;
}
