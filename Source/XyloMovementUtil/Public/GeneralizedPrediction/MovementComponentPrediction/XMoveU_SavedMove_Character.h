// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GeneralizedPrediction/CustomPrediction/XMoveU_Blackboard.h"


typedef TSharedPtr<class FXMoveU_SavedMove_Character> FXMoveU_SavedMovePtr;

/**
 * Locally stored data that describes the movement happened over a movement tick.
 * Stores inputs, and results of the move.
 *
 * The main reason of existence of this class is to locally replay movements after a server correction.
 * Let's say we have high ping, so after we move it takes 5 frames to receive the response from the server for that
 * move (correction or acknowledgment). Once the response arrives, we might find out that the move we performed 5
 * frames ago, finished at the wrong position by a meter. The character movement component instead of just snapping
 * you back to the newly received location, it also replays the moves that happened locally after this one, so that
 * the snapping is not as visible.
 */
class XYLOMOVEMENTUTIL_API FXMoveU_SavedMove_Character : public FSavedMove_Character
{
	using Super = FSavedMove_Character;
	
public:
	FXMoveU_SavedMove_Character() {}

	virtual ~FXMoveU_SavedMove_Character() override {}

	/** Clear saved move properties, so it can be re-used. */
	virtual void Clear() override;

	/** Returns true if this move is an "important" move that should be sent again if not acked by the server.
	 * <p> Note: Input changes should make this return true. */
	virtual bool IsImportantMove(const FSavedMovePtr& LastAckedMove) const override;
	
	/** Returns true if this move can be combined with NewMove for replication without changing any behavior 
	 * <p> Call Context: Called in ReplicateMoveToServer right after SetMoveFor and before PerformMovement
	 * <p> Note: Returns true if nothing important changed between this move and the start of the new one */
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

	/** To combine this move with an older move we need to revert the character state to the old move.
	 * A better name for this function would be RevertOldMoveChanges.
	 * <p> Call Context: Called in ReplicateMoveToServer if CanCombineWith returns true
	 * <p> Note: Since nothing changed since PendingMove, we merge PendingMove and NewMove, and simulate
	 * both together, so we need to revert the Character / CMC state to the one of PendingMove */
	virtual void CombineWith(const FSavedMove_Character* OldMove, ACharacter* InCharacter, APlayerController* PC, const FVector& OldStartLocation) override;

	/** Called to set up this saved move (when initially created) to make a predictive correction.
	 * <p> Call Context: Called in ReplicateMoveToServer before CanCombineWith and PerformMovement.
	 * <p> Note: Here we set only the values that will NOT change during PerformMovement, otherwise they are not
	 * going to get updated after CombineWith. */
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

	/** Set the properties describing the position, etc. of the moved pawn at the start of the move.
	 * <p> Call Context: Called in SetMoveFor and in ReplicateMoveToServer if we called CombineWith
	 * <p> Note: Here we set the values in this SavedMove that will change during PerformMovement, since after
	 * CombineWith we need to reset this Move to the state of PendingMove */
	virtual void SetInitialPosition(ACharacter* C) override;

	/** Called before ClientUpdatePosition uses this SavedMove to make a predictive correction.
	 * This function sets the character and movement component state to the one of this Move, so the cmc can resimulate
	 * the tick after this move.
	 * <p> Call Context: Called in ClientUpdatePositionAfterServerUpdate before MoveAutonomous
	 * <p> Note: Does not need to deal with variables that drive compressed flags cause MoveAutonomous already calls
	 * UpdateFromCompressedFlags. Other inputs should be rolled back here.
	 * <p> Note 2: Should not restore any variable that is sent back with FCharacterMoveResponseDataContainer else it
	 * will override the server correction, causing more desync */
	virtual void PrepMoveFor(ACharacter* C) override;

	/** Set the properties describing the final position and state of the moved pawn.
	 * <p> Call Context: Called in ReplicateMoveToServer after PerformMovement and
	 * in ClientUpdatePositionAfterServerUpdate after MoveAutonomous
	 * <p> Note: if PostUpdateMode == PostUpdate_Record we need to set bForceNoCombine to true if there is any initial
	 * value that changed during the movement in a way that would prevent this move from being able to be combined.
	 * this way the network data from this move is instantly sent to the server instead of waiting for the next frame. */
	virtual void PostUpdate(ACharacter* C, EPostUpdateMode PostUpdateMode) override;

public:
	XMoveU::FBlackboard Blackboard;
};