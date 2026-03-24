// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementReplication.h"
#include "XMoveU_CharacterNetworkMoveData.h"


/**
 * A wrapper for FCharacterNetworkMoveData, that stores three moves: new, old, pending.
 * Use this to override class of FCharacterNetworkMoveData.
 * 
 * Call UCharacterMovementComponent::SetNetworkMoveDataContainer in the cmc constructor to choose what
 * FCharacterNetworkMoveDataContainer class to use
 */
struct XYLOMOVEMENTUTIL_API FXMoveU_CharacterNetworkMoveDataContainer : public FCharacterNetworkMoveDataContainer
{
	using Super = FCharacterNetworkMoveDataContainer;

public:
	FXMoveU_CharacterNetworkMoveDataContainer()
	{
		NewMoveData = &MoveData[0];
		PendingMoveData = &MoveData[1];
		OldMoveData = &MoveData[2];
	}
 
private:
	FXMoveU_CharacterNetworkMoveData MoveData[3];
};
