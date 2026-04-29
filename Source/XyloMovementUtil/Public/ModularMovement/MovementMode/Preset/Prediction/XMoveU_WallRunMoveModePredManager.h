// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/CustomPrediction/XMoveU_PredictionManager.h"
#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/XMoveU_SimplePredictionProxyTemplate.h"
#include "ModularMovement/MovementMode/Preset/XMoveU_WallRunMoveMode.h"
#include "XMoveU_WallRunMoveModePredManager.generated.h"



/**
 * 
 */
USTRUCT(BlueprintType, DisplayName="Prediction Proxy (WallData)")
struct XYLOMOVEMENTUTIL_API FXMoveU_PredictionProxy_WallData :
#if CPP
	public XMoveU::TSimplePredictionProxy<FXMoveU_WallData, XMoveU::ProxyVar::Traits::ByValue>
#else
	public FXMoveU_SimplePredictionProxy
#endif
{
	GENERATED_BODY()

protected:
	virtual FXMoveU_WallData MakeDefaulted() override { return FXMoveU_WallData(); }
};


/**
 * 
 */
UCLASS()
class XYLOMOVEMENTUTIL_API UXMoveU_WallRunMoveModePredManager : public UXMoveU_PredictionManager
{
	GENERATED_BODY()

public:
	virtual void OnRegistered(UCharacterMovementComponent* MovementComponent) override;
};
