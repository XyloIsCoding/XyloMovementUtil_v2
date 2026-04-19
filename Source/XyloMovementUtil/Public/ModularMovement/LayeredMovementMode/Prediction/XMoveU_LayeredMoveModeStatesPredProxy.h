// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/XMoveU_SimplePredictionProxyTemplate.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"
#include "UObject/Object.h"
#include "XMoveU_LayeredMoveModeStatesPredProxy.generated.h"


class AXMoveU_ModularCharacter;

/**
 *
 */
USTRUCT()
struct XYLOMOVEMENTUTIL_API FXMoveU_LayeredMoveModeStatesPredProxy :
#if CPP
	public XMoveU::TSimplePredictionProxy<uint32, XMoveU::ProxyVar::Traits::ByValue>
#else
	public FXMoveU_SimplePredictionProxy
#endif
{
	GENERATED_BODY()

public:
	FXMoveU_LayeredMoveModeStatesPredProxy()
	{
		bIsInput = false;
		bCheckForError = false;
		CorrectionMode = EXMoveU_CorrectionMode::Authoritative;
		bAffectedBySimulation = true;
		bRestoreAfterRollback = false;
		bIsPositionRelated = false;
	}

public:
	void Initialize(FName Name, UXMoveU_ModularMovementComponent* MovementComponent);
protected:
	int32 LayeredMoveModesCount = 0;

protected:
	virtual uint32 MakeDefaulted() override { return 0; }
	virtual bool SerializeInputsAndCorrectionStates_Internal(uint32& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool HasPredictionError_Internal(uint32 ClientPredictedValue) override;
	virtual bool SerializeCorrectedStates_Internal(uint32& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool CanCombineWithNewFrame_Internal(uint32 OldFrameValue, uint32 NewFrameValue) override;
	virtual bool HasNonSimulatedChange(uint32 LastPostSimValue, uint32 NewPreSimValue) override;
	virtual bool IsImportantFrame_Internal(uint32 PreSimValue, uint32 PostSimValue, uint32 LastAckedPreSimValue, uint32 LastAckedPostSimValue) override;
};
