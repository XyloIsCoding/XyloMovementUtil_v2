// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/XMoveU_ModularMovementPredManager.h"

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
}
