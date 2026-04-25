// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/LayeredMovementMode/Preset/XMoveU_SprintLayeredMoveMode.h"

#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_SprintLayeredMoveMode::UXMoveU_SprintLayeredMoveMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxWalkSpeedSprinting = 570.f;
	MinSprintingAngleCosine = 0.5f;
}

void UXMoveU_SprintLayeredMoveMode::ModifyMaxSpeed(float& OutMaxSpeed) const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	if (MoveComp && MoveComp->IsWalking())
	{
		OutMaxSpeed = MaxWalkSpeedSprinting;
	}
}

bool UXMoveU_SprintLayeredMoveMode::ShouldEnterMode(float DeltaSeconds)
{
	return !IsInMode() && WantsToBeInMode() && CanSprintInCurrentState();
}

bool UXMoveU_SprintLayeredMoveMode::ShouldLeaveMode(float DeltaSeconds)
{
	return IsInMode() && (!WantsToBeInMode() || !CanSprintInCurrentState());
}

bool UXMoveU_SprintLayeredMoveMode::ShouldForceLeaveMode(float DeltaSeconds)
{
	return IsInMode() && !CanSprintInCurrentState();
}

bool UXMoveU_SprintLayeredMoveMode::CanSprintInCurrentState() const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	if (!MoveComp || !MoveComp->UpdatedComponent || MoveComp->UpdatedComponent->IsSimulatingPhysics())
	{
		return false;
	}
	
	if ((MoveComp->GetCurrentAcceleration().GetSafeNormal() | MoveComp->UpdatedComponent->GetForwardVector()) < MinSprintingAngleCosine)
	{
		return false;
	}
	return (MoveComp->IsFalling() || MoveComp->IsMovingOnGround()) && !MoveComp->IsCrouching();
}
