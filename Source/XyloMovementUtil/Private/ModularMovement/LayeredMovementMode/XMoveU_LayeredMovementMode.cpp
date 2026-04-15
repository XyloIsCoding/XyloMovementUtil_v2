// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/LayeredMovementMode/XMoveU_LayeredMovementMode.h"

#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_LayeredMovementMode::UXMoveU_LayeredMovementMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCancelRequestAfterTransitionCheck = false;
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXMoveU_LayeredMovementMode
 */

void UXMoveU_LayeredMovementMode::OnRegistered(uint32 InModeIndex)
{
	ModeIndex = InModeIndex;
	bRegistered = true;
}

void UXMoveU_LayeredMovementMode::RequestMode(bool bWantsToEnterMode)
{
	bModeRequested = bWantsToEnterMode;
}

bool UXMoveU_LayeredMovementMode::WantsToBeInMode() const
{
	return bModeRequested;
}

void UXMoveU_LayeredMovementMode::SetModeState(bool bIsInMode)
{
	checkf(bRegistered, TEXT("UXMoveU_LayeredMovementMode must be registered to a UXMoveU_ModularMovementComponent"))
	
	if (AXMoveU_ModularCharacter* Character = GetOwningCharacter())
	{
		if (bIsInMode)
		{
			// 0100010 States
			// 0000100 1 << ModeIndex
			// 0100110 States | (1 << ModeIndex)
			Character->LayeredMovementModeStates |= (1 << ModeIndex);  
		}
		else
		{
			// 0100110 States
			// 0000100 1 << ModeIndex
			// 1111011 ~(1 << ModeIndex)
			// 0100010 States & ~(1 << ModeIndex)
			Character->LayeredMovementModeStates &= ~(1 << ModeIndex);
		}
	}
}

bool UXMoveU_LayeredMovementMode::IsInMode() const
{
	checkf(bRegistered, TEXT("UXMoveU_LayeredMovementMode must be registered to a UXMoveU_ModularMovementComponent"))
	
	if (AXMoveU_ModularCharacter* Character = GetOwningCharacter())
	{
		// true:
		// 0100110 States
		// 0000100 1 << ModeIndex
		// 0000100 States & (1 << ModeIndex)
		// false:
		// 0100010 States
		// 0000100 1 << ModeIndex
		// 0000000 States & (1 << ModeIndex)
		return ((Character->LayeredMovementModeStates & (1 << ModeIndex)) != 0);
	}
	return false;
}

void UXMoveU_LayeredMovementMode::ReplicateStateToSimProxies()
{
	if (UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp())
	{
		if (IsInMode())
		{
			RequestMode(true);
			EnterMode(true);
		}
		else
		{
			RequestMode(false);
			LeaveMode(true);
		}
		MoveComp->bNetworkUpdateReceived = true;
	}
}

/*====================================================================================================================*/
// LayeredMovementModeInterface

void UXMoveU_LayeredMovementMode::EnterMode(bool bClientSimulation)
{
	SetModeState(true);
	OnEnteredMode();
}

void UXMoveU_LayeredMovementMode::LeaveMode(bool bClientSimulation)
{
	SetModeState(false);
	OnLeftMode();
}

// ~LayeredMovementModeInterface
/*====================================================================================================================*/
