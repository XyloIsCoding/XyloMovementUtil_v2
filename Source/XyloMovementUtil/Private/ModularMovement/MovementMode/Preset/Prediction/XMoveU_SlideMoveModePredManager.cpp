// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/MovementMode/Preset/Prediction/XMoveU_SlideMoveModePredManager.h"

#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/Specialization/XMoveU_PredictionProxy_Float.h"
#include "ModularMovement/MovementMode/Preset/XMoveU_SlideMoveMode.h"

void UXMoveU_SlideMoveModePredManager::OnRegistered(UCharacterMovementComponent* MovementComponent)
{
	Super::OnRegistered(MovementComponent);
	
	UXMoveU_SlideMoveMode* SlideMoveMode = Cast<UXMoveU_SlideMoveMode>(GetOuter());
	if (!ensureMsgf(SlideMoveMode, TEXT("UXMoveU_SlideMoveModePredManager must be owned by a UXMoveU_SlideMoveMode for it to work!")))
	{
		return;
	}

	TSharedPtr<FXMoveU_PredictionProxy_Float> TimeSinceLastSlide = MakeShared<FXMoveU_PredictionProxy_Float>();
	TimeSinceLastSlide->SetName("XMoveU_SlideMoveMode_TimeSinceLastSlide");
	TimeSinceLastSlide->SetProxyVariable<XMoveU::TProxyVar_Object<UXMoveU_SlideMoveMode, float>>({SlideMoveMode, &UXMoveU_SlideMoveMode::TimeSinceLastSlide});
	TimeSinceLastSlide->bAffectedBySimulation = true;
	TimeSinceLastSlide->CorrectionMode = EXMoveU_CorrectionMode::Local;
	RegisterPredictionProxy(TimeSinceLastSlide);
}
