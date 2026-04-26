// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/XMoveU_ModularMovementPredManager.h"

#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/Specialization/XMoveU_PredictionProxy_Float.h"
#include "ModularMovement/LayeredMovementMode/Prediction/XMoveU_LayeredMoveModeInputsPredProxy.h"
#include "ModularMovement/LayeredMovementMode/Prediction/XMoveU_LayeredMoveModeStatesPredProxy.h"

void UXMoveU_ModularMovementPredManager::OnRegistered(UCharacterMovementComponent* MovementComponent)
{
	Super::OnRegistered(MovementComponent);

	UXMoveU_ModularMovementComponent* ModularMovementComponent = Cast<UXMoveU_ModularMovementComponent>(MovementComponent);

	TSharedPtr<FXMoveU_LayeredMoveModeInputsPredProxy> LayeredMoveModeInputs = MakeShared<FXMoveU_LayeredMoveModeInputsPredProxy>();
	LayeredMoveModeInputs->Initialize("XMoveU_LayeredMoveModeInputs", ModularMovementComponent);
	RegisterPredictionProxy(LayeredMoveModeInputs);

	TSharedPtr<FXMoveU_LayeredMoveModeStatesPredProxy> LayeredMoveModeStates = MakeShared<FXMoveU_LayeredMoveModeStatesPredProxy>();
	LayeredMoveModeStates->Initialize("XMoveU_LayeredMoveModeStates", ModularMovementComponent);
	RegisterPredictionProxy(LayeredMoveModeStates);

	TSharedPtr<FXMoveU_PredictionProxy_Float> CoyoteTimeDurationLeft = MakeShared<FXMoveU_PredictionProxy_Float>();
	CoyoteTimeDurationLeft->SetName("XMoveU_CoyoteTimeDurationLeft");
	CoyoteTimeDurationLeft->SetProxyVariable(XMoveU::TProxyVar_Object(ModularMovementComponent, &UXMoveU_ModularMovementComponent::CoyoteTimeDurationLeft));
	CoyoteTimeDurationLeft->bAffectedBySimulation = true;
	CoyoteTimeDurationLeft->CorrectionMode = EXMoveU_CorrectionMode::Local;
	RegisterPredictionProxy(CoyoteTimeDurationLeft);
}
