// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneralizedPrediction/MovementComponentPrediction/XMoveU_CharacterNetworkMoveData.h"

#include "GameFramework/Character.h"
#include "GeneralizedPrediction/XMoveU_PredictionMovementComponent.h"
#include "GeneralizedPrediction/CustomPrediction/XMoveU_PredictionManager.h"
#include "GeneralizedPrediction/MovementComponentPrediction/XMoveU_SavedMove_Character.h"

void FXMoveU_CharacterNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(ClientMove, MoveType);

	UCharacterMovementComponent* MovementComponent = ClientMove.CharacterOwner->GetCharacterMovement();
	const FXMoveU_SavedMove_Character& XMoveU_CurrentMove = static_cast<const FXMoveU_SavedMove_Character&>(ClientMove);
	
	// Call UXMoveU_PredictionManager::ClientFillNetworkMoveData
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionManagers(MovementComponent,
	[&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		PredictionManager->ClientFillNetworkMoveData(*this, XMoveU_CurrentMove, MoveType);
	});
	
	// Call FXMoveU_PredictionProxy::CollectCorrectedStates
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionProxies(MovementComponent,
	[&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		PredictionProxy.CollectInputsAndCorrectionStates(XMoveU_CurrentMove.Blackboard, Blackboard);
	});
}

bool FXMoveU_CharacterNetworkMoveData::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType)
{
	if (!Super::Serialize(CharacterMovement, Ar, PackageMap, MoveType))
	{
		// Super::Serialize returns '!Ar.IsError()'
		return false;
	}

	bool bOutSuccess = true;

	// Call UXMoveU_PredictionManager::SerializeNetworkMoveData
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionManagers(&CharacterMovement,
	[&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		bOutSuccess &= PredictionManager->SerializeNetworkMoveData(*this, &CharacterMovement, Ar, PackageMap, MoveType);
	});
	
	// Call FXMoveU_PredictionProxy::SerializeInputsAndCorrectionStates
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionProxies(&CharacterMovement,
	[&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		bOutSuccess &= PredictionProxy.SerializeInputsAndCorrectionStates(Blackboard, Ar, PackageMap);
	});

	return bOutSuccess;
}
