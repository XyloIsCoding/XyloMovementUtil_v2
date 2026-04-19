// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/LayeredMovementMode/Preset/Prediction/XMoveU_DashLayeredMoveModePredManager.h"

#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/Specialization/XMoveU_PredictionProxy_Float.h"
#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/Specialization/XMoveU_PredictionProxy_Vector.h"
#include "ModularMovement/LayeredMovementMode/Preset/XMoveU_DashLayeredMoveMode.h"

void UXMoveU_DashLayeredMoveModePredManager::OnRegistered(UCharacterMovementComponent* MovementComponent)
{
	Super::OnRegistered(MovementComponent);
	
	UXMoveU_DashLayeredMoveMode* DashLayeredMoveMode = Cast<UXMoveU_DashLayeredMoveMode>(GetOuter());
	if (!ensureMsgf(DashLayeredMoveMode, TEXT("UXMoveU_DashLayeredMoveModePredManager must be owned by a UXMoveU_DashLayeredMoveMode for it to work!")))
	{
		return;
	}

	TSharedPtr<FXMoveU_PredictionProxy_Float> TimeSinceDash = MakeShared<FXMoveU_PredictionProxy_Float>();
	TimeSinceDash->SetName("XMoveU_DashLayeredMoveMode_TimeSinceDash");
	TimeSinceDash->SetProxyVariable<XMoveU::TProxyVar_Object<UXMoveU_DashLayeredMoveMode, float>>({DashLayeredMoveMode, &UXMoveU_DashLayeredMoveMode::TimeSinceDash});
	TimeSinceDash->bAffectedBySimulation = true;
	TimeSinceDash->CorrectionMode = EXMoveU_CorrectionMode::Local;
	RegisterPredictionProxy(TimeSinceDash);

	TSharedPtr<FXMoveU_PredictionProxy_Float> DashCharge = MakeShared<FXMoveU_PredictionProxy_Float>();
	DashCharge->SetName("XMoveU_DashLayeredMoveMode_DashCharge");
	DashCharge->SetProxyVariable<XMoveU::TProxyVar_Object<UXMoveU_DashLayeredMoveMode, float>>({DashLayeredMoveMode, &UXMoveU_DashLayeredMoveMode::DashCharge});
	DashCharge->bAffectedBySimulation = true;
	DashCharge->CorrectionMode = EXMoveU_CorrectionMode::Local;
	RegisterPredictionProxy(DashCharge);
}
