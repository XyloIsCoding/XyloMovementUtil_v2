// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/MovementMode/Preset/Prediction/XMoveU_WallRunMoveModePredManager.h"

#include "GeneralizedPrediction/CustomPrediction/Helpers/XMoveU_ProxyVar.h"
#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/XMoveU_SimplePredictionProxy.h"
#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/Specialization/XMoveU_PredictionProxy_Float.h"
#include "ModularMovement/MovementMode/Preset/XMoveU_WallRunMoveMode.h"

void UXMoveU_WallRunMoveModePredManager::OnRegistered(UCharacterMovementComponent* MovementComponent)
{
	Super::OnRegistered(MovementComponent);

	UXMoveU_WallRunMoveMode* WallRunMoveMode = Cast<UXMoveU_WallRunMoveMode>(GetOuter());
	if (!ensureMsgf(WallRunMoveMode, TEXT("UXMoveU_WallRunMoveModePredManager must be owned by a UXMoveU_WallRunMoveMode for it to work!")))
	{
		return;
	}

	TSharedPtr<FXMoveU_PredictionProxy_Float> WallRunReentryLockTimeRemaining = MakeShared<FXMoveU_PredictionProxy_Float>();
	WallRunReentryLockTimeRemaining->SetName("XMoveU_WallRunMoveMode_WallRunReentryLockTimeRemaining");
	WallRunReentryLockTimeRemaining->SetProxyVariable(XMoveU::TProxyVar_Object(WallRunMoveMode, &UXMoveU_WallRunMoveMode::WallRunReentryLockTimeRemaining));
	WallRunReentryLockTimeRemaining->bAffectedBySimulation = true;
	WallRunReentryLockTimeRemaining->CorrectionMode = EXMoveU_CorrectionMode::Local;
	RegisterPredictionProxy(WallRunReentryLockTimeRemaining);

	// Just register it to allow move combining to work properly.
	TSharedPtr<FXMoveU_PredictionProxy_WallData> CurrentWall = MakeShared<FXMoveU_PredictionProxy_WallData>();
	CurrentWall->SetName("XMoveU_WallRunMoveMode_CurrentWall");
	CurrentWall->SetProxyVariable(XMoveU::TProxyVar_Object(WallRunMoveMode, &UXMoveU_WallRunMoveMode::CurrentWall));
	CurrentWall->bAffectedBySimulation = true;
	CurrentWall->CorrectionMode = EXMoveU_CorrectionMode::None;
	RegisterPredictionProxy(CurrentWall);
}
