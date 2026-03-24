// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementReplication.h"
#include "GeneralizedPrediction/CustomPrediction/XMoveU_Blackboard.h"


/**
 * Data sent from client to server at the end of the movement tick. Only data stored in SavedMove can be sent.
 * Should contain inputs and everything the server needs in UCharacterMovementComponent::ServerCheckClientError
 * to validate the client's move.
 * 
 * To make your cmc use a custom FCharacterNetworkMoveData, you need to create a FCharacterNetworkMoveDataContainer
 * subclass that uses your struct.
 */
struct XYLOMOVEMENTUTIL_API FXMoveU_CharacterNetworkMoveData : public FCharacterNetworkMoveData
{
	using Super = FCharacterNetworkMoveData;

public:
	FXMoveU_CharacterNetworkMoveData() {}

	/** Function responsible to fill this struct with data from the saved move
	 * interesting to note that it is also called by UCharacterMovementComponent::ClientUpdatePositionAfterServerUpdate */
	virtual void ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType) override;

	/** Actual serialization of this struct needed to send it over the network */
	virtual bool Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType) override;

	XMoveU::FBlackboard Blackboard;
};
