// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementReplication.h"
#include "GeneralizedPrediction/CustomPrediction/XMoveU_Blackboard.h"

/**
 * Data we receive from the server after all the movement logic has been executed.
 * Should be used to send back to the client everything needed to check the correctness of their movement.
 * It is filled and sent in UCharacterMovementComponent::SendClientAdjustment.
 * 
 * Call UCharacterMovementComponent::SetMoveResponseDataContainer in the cmc constructor to choose what
 * FCharacterMoveResponseDataContainer class to use
 */
struct XYLOMOVEMENTUTIL_API FXMoveU_CharacterMoveResponseDataContainer : FCharacterMoveResponseDataContainer
{
	using Super = FCharacterMoveResponseDataContainer;

	/** Function responsible to fill this container with data from the cmc and the pending adjustment */
	virtual void ServerFillResponseData(const UCharacterMovementComponent& CharacterMovement, const FClientAdjustment& PendingAdjustment) override;

	/** Actual serialization of this struct needed to send it over the network */
	virtual bool Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap) override;

	XMoveU::FBlackboard Blackboard;
};

