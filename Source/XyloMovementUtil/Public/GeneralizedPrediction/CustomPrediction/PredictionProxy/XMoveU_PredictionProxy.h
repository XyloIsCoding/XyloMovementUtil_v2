// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GeneralizedPrediction/CustomPrediction/XMoveU_Blackboard.h"
#include "XMoveU_PredictionProxy.generated.h"


UENUM()
enum class EXMoveU_CorrectionContext : uint8
{
	OnReceive,
	PreRollback,
};


/**
 * PredictionProxies are a way to make variables part of the UCharacterMovementComponent prediction system without
 * having to override the movement component or the other structs related to client side prediction and server
 * reconciliation (e.g. SavedMove, NetworkMoveData, ResponseDataContainer).
 * This base class provides the public interface for PredictionProxies, which is used by the classes mentioned above,
 * in order to deal with the prediction of each variable in a centralized place, instead of splitting the logic across
 * multiple structs and classes.
 */
USTRUCT(BlueprintType)
struct XYLOMOVEMENTUTIL_API FXMoveU_PredictionProxy
{
	GENERATED_BODY()
	
	FXMoveU_PredictionProxy() {}
	virtual ~FXMoveU_PredictionProxy() {}

public:
	virtual void CollectInputsAndCorrectionStates(const XMoveU::FBlackboard& CurrentPredictionFrame, XMoveU::FBlackboard& NetworkFrame) {}
	virtual bool SerializeInputsAndCorrectionStates(XMoveU::FBlackboard& NetworkFrame, FArchive& Ar, UPackageMap* PackageMap) { return true; }
	/** Called at the beginning of UCharacterMovementComponent::ServerMove_PerformMovement to allow applying client generated inputs to the server simulation. */
	virtual void ApplyClientInputs(const XMoveU::FBlackboard& ClientNetworkFrame) {}
	/** Called by UCharacterMovementComponent::ServerCheckClientError */
	virtual bool HasPredictionError(const XMoveU::FBlackboard& ClientNetworkFrame) { return false; }
	/** Override to return true if the tracked variable tracks a location state.
	 * Error in position related variables can be ignored while transitioning from falling to walking, in order to
	 * prevent corrections when transitioning to and from moving platforms. */
	virtual bool IsPositionRelated() const { return false; }
	
	virtual void CollectCorrectedStates(XMoveU::FBlackboard& AuthoritativeFrame) {}
	virtual bool SerializeCorrectedStates(XMoveU::FBlackboard& AuthoritativeFrame, FArchive& Ar, UPackageMap* PackageMap) { return true; }
	/** Called by UCharacterMovementComponent::OnClientCorrectionReceived and at the beginning of UCharacterMovementComponent::ClientUpdatePositionAfterServerUpdate.
	 * Context param is used to differentiate the two calls. */
	virtual void ApplyCorrectedState(const XMoveU::FBlackboard& AuthoritativeFrame, const XMoveU::FBlackboard* TargetPredictionFrame, EXMoveU_CorrectionContext Context) {}

	/** Called instead of SerializeCorrectedStates if the move was valid. Should be used with caution, since data
	 * serialized is sent to client with every move acknowledgment. */
	virtual bool SerializeAcknowledgedStates(XMoveU::FBlackboard& AuthoritativeFrame, FArchive& Ar, UPackageMap* PackageMap) { return true; }
	/** Called by UCharacterMovementComponent::ClientAckGoodMove_Implementation. */
	virtual void CheckAcknowledgedState(const XMoveU::FBlackboard& AuthoritativeFrame, const XMoveU::FBlackboard* TargetPredictionFrame) {}
	
	/** Called by FXMoveU_SavedMove_Character::Clear */
	virtual void Reset(XMoveU::FBlackboard& PredictionFrame) {}
	/** Called by FXMoveU_SavedMove_Character::SetMoveFor */
	virtual void CollectCurrentState(XMoveU::FBlackboard& PredictionFrame) {}
	/** Called by FXMoveU_SavedMove_Character::SetInitialPosition */
	virtual void CollectDynamicState(XMoveU::FBlackboard& PredictionFrame) {}
	/** Called by FXMoveU_SavedMove_Character::SetMoveFor */
	virtual bool BlockCombinePreSimulation(XMoveU::FBlackboard& PredictionFrame) { return false; }
	/** Called by FXMoveU_SavedMove_Character::PostUpdate */
	virtual void CollectFinalState(XMoveU::FBlackboard& PredictionFrame, FSavedMove_Character::EPostUpdateMode PostUpdateMode) {}
	/** Called by FXMoveU_SavedMove_Character::PostUpdate */
	virtual bool BlockCombinePostSimulation(XMoveU::FBlackboard& PredictionFrame) { return false; }

	/** Called by FXMoveU_SavedMove_Character::IsImportantMove */
	virtual bool IsImportantFrame(const XMoveU::FBlackboard& PredictionFrame, const XMoveU::FBlackboard& LastAckedPredictionFrame) { return false; }
	/** Called by FXMoveU_SavedMove_Character::CanCombineWith */
	virtual bool CanCombineWithNewFrame(const XMoveU::FBlackboard& PredictionFrame, const XMoveU::FBlackboard& NewPredictionFrame) { return true; }
	/** Called by FXMoveU_SavedMove_Character::CombineWith */
	virtual void RevertFrameChanges(const XMoveU::FBlackboard& OldPredictionFrame) {}

	/** Called at the beginning of UCharacterMovementComponent::ClientUpdatePositionAfterServerUpdate to cache the
	 * value the client had right before the rollback. Note that this is called AFTER ApplyCorrectedState is called. */
	virtual void CacheStatePreRollback() {}
	/** Called by FXMoveU_SavedMove_Character::PrepMoveFor */
	virtual void RollbackToFrame(const XMoveU::FBlackboard& PredictionFrame) {}
	/** Called at the end of UCharacterMovementComponent::ClientUpdatePositionAfterServerUpdate to allow restoring the
	 * value the client had before the rollback. */
	virtual void RestoreStatePostRollback() {}
};
