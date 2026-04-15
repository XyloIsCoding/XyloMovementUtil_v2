// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/LayeredMovementMode/Preset/XMoveU_CrouchLayeredMoveMode.h"

#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_CrouchLayeredMoveMode::UXMoveU_CrouchLayeredMoveMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UXMoveU_CrouchLayeredMoveMode::RequestMode(bool bWantsToEnterMode)
{
	if (UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp())
	{
		MoveComp->bWantsToCrouch = bWantsToEnterMode;
	}
}

bool UXMoveU_CrouchLayeredMoveMode::WantsToBeInMode() const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	return MoveComp && MoveComp->bWantsToCrouch;
}

void UXMoveU_CrouchLayeredMoveMode::SetModeState(bool bIsInMode)
{
	if (AXMoveU_ModularCharacter* Char = GetOwningCharacter())
	{
		Char->bIsCrouched = bIsInMode;
	}
}

bool UXMoveU_CrouchLayeredMoveMode::IsInMode() const
{
	AXMoveU_ModularCharacter* Char = GetOwningCharacter();
	return Char && Char->bIsCrouched;
}

void UXMoveU_CrouchLayeredMoveMode::ReplicateStateToSimProxies()
{
	// Empty implementation because character class already deals with it.
}

bool UXMoveU_CrouchLayeredMoveMode::ShouldEnterMode(float DeltaSeconds) const
{
	return !IsInMode() && WantsToBeInMode() && CanCrouchInCurrentState();
}

bool UXMoveU_CrouchLayeredMoveMode::ShouldLeaveMode(float DeltaSeconds) const
{
	return IsInMode() && (!WantsToBeInMode() || !CanCrouchInCurrentState());
}

bool UXMoveU_CrouchLayeredMoveMode::ShouldForceLeaveMode(float DeltaSeconds) const
{
	return IsInMode() && !CanCrouchInCurrentState();
}

bool UXMoveU_CrouchLayeredMoveMode::CanCrouchInCurrentState() const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	return MoveComp && MoveComp->CanCrouchInCurrentState();
}

void UXMoveU_CrouchLayeredMoveMode::EnterMode(bool bClientSimulation)
{
	if (UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp())
	{
		MoveComp->Crouch(bClientSimulation);
	}
}

void UXMoveU_CrouchLayeredMoveMode::LeaveMode(bool bClientSimulation)
{
	if (UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp())
	{
		MoveComp->UnCrouch(bClientSimulation);
	}
}


