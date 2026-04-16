// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/LayeredMovementMode/Preset/XMoveU_SlamLayeredMoveMode.h"

#include "ModularMovement/XMoveU_JumpStaticLibrary.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_SlamLayeredMoveMode::UXMoveU_SlamLayeredMoveMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCancelRequestAfterTransitionCheck = true;
	
	SlamGravityMultiplier = 4.f;
	SlamHorizontalVelocity = 500.f;
	SlamMinHeight = 200.f;
}

bool UXMoveU_SlamLayeredMoveMode::ShouldEnterMode(float DeltaSeconds) const
{
	return !IsInMode() && WantsToBeInMode() && CanSlamInCurrentState();
}

bool UXMoveU_SlamLayeredMoveMode::ShouldForceLeaveMode(float DeltaSeconds) const
{
	return IsInMode() && !CanSlamInCurrentState(true);
}

bool UXMoveU_SlamLayeredMoveMode::CanSlamInCurrentState(bool bIgnoreMinHeight) const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	if (!MoveComp || !MoveComp->UpdatedComponent || MoveComp->UpdatedComponent->IsSimulatingPhysics())
	{
		return false;
	}

	return MoveComp->IsFalling() && (bIgnoreMinHeight || MoveComp->GetGroundInfo().GroundDistance > SlamMinHeight);
}

void UXMoveU_SlamLayeredMoveMode::OnEnteredMode()
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	MoveComp->Velocity = FVector::ZeroVector;
}

void UXMoveU_SlamLayeredMoveMode::UpdateMode(float DeltaSeconds)
{
	if (IsInMode())
	{
		UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
		UXMoveU_JumpStaticLibrary::ApplyJumpAcceleration(MoveComp, FVector::ZeroVector, MoveComp->GetGravityZ() * SlamGravityMultiplier, 0.f, 0.f, 0.f, DeltaSeconds);
		UXMoveU_JumpStaticLibrary::ApplyJumpImpulse(MoveComp, MoveComp->GetForwardVector(), 0.f, SlamHorizontalVelocity, false, false, true, SlamHorizontalVelocity);
	}
}
