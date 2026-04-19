// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/CustomPrediction/XMoveU_PredictionManager.h"
#include "XMoveU_ModularMovementPredManager.generated.h"

/**
 * 
 */
UCLASS()
class XYLOMOVEMENTUTIL_API UXMoveU_ModularMovementPredManager : public UXMoveU_PredictionManager
{
	GENERATED_BODY()

public:
	virtual void OnRegistered(UXMoveU_ModularMovementComponent* MovementComponent) override;
};
