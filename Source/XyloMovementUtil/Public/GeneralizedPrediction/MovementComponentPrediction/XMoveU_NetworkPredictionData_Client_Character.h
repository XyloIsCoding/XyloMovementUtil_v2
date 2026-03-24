// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XMoveU_SavedMove_Character.h"
#include "GameFramework/CharacterMovementComponent.h"


/**
 * Class that instantiates the FSavedMove_Character for the cmc to use.
 * Use this to override class of FSavedMove_Character
 *
 * Override UCharacterMovementComponent::GetPredictionData_Client to return your subclass of
 * FNetworkPredictionData_Client_Character to change FSavedMove_Character class
 */
class XYLOMOVEMENTUTIL_API FXMoveU_NetworkPredictionData_Client_Character : public FNetworkPredictionData_Client_Character
{
	using Super = FNetworkPredictionData_Client_Character;

public:
	FXMoveU_NetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
	{
		MovementComponent = MakeWeakObjectPtr(&ClientMovement);
	}

	virtual FSavedMovePtr AllocateNewMove() override
	{
		FXMoveU_SavedMovePtr NewMove = MakeShared<FXMoveU_SavedMove_Character>();

		// God forgive me for how ugly this is, but I need it to get the character
		// in FXMoveU_SavedMove_Character::IsImportantMove
		const UCharacterMovementComponent* MoveComp = MovementComponent.Get();
		NewMove->CharacterOwner = MoveComp ? MoveComp->GetCharacterOwner() : nullptr;
		
		return NewMove;
	}

protected:
	TWeakObjectPtr<const UCharacterMovementComponent> MovementComponent;
};
