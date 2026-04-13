// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "GeneralizedPrediction/MovementComponentPrediction/XMoveU_CharacterMoveResponseDataContainer.h"

#include "GeneralizedPrediction/XMoveU_PredictionMovementComponent.h"
#include "GeneralizedPrediction/CustomPrediction/XMoveU_PredictionManager.h"

void FXMoveU_CharacterMoveResponseDataContainer::ServerFillResponseData(const UCharacterMovementComponent& CharacterMovement, const FClientAdjustment& PendingAdjustment)
{
	Super::ServerFillResponseData(CharacterMovement, PendingAdjustment);

	// Call UXMoveU_PredictionManager::ServerFillResponseData
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionManagers(&CharacterMovement,
	[&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		PredictionManager->ServerFillResponseData(*this, &CharacterMovement, PendingAdjustment);
	});
	
	// Call FXMoveU_PredictionProxy::CollectCorrectedStates
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionProxies(&CharacterMovement,
	[&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		PredictionProxy.CollectCorrectedStates(Blackboard);
	});
}

bool FXMoveU_CharacterMoveResponseDataContainer::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap)
{
	if (!Super::Serialize(CharacterMovement, Ar, PackageMap))
	{
		// Super::Serialize returns '!Ar.IsError()'
		return false;
	}

	bool bOutSuccess = true;

	// Call UXMoveU_PredictionManager::SerializeResponseData
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionManagers(&CharacterMovement,
	[&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		bOutSuccess &= PredictionManager->SerializeResponseData(*this, &CharacterMovement, Ar, PackageMap);
	});
	
	if (IsCorrection())
	{
		// Call FXMoveU_PredictionProxy::SerializeCorrectedStates
		UXMoveU_PredictionMovementComponent::ExecuteOnPredictionProxies(&CharacterMovement,
		[&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
		{
			bOutSuccess &= PredictionProxy.SerializeCorrectedStates(Blackboard, Ar, PackageMap);
		});
	}

	return bOutSuccess;
}
