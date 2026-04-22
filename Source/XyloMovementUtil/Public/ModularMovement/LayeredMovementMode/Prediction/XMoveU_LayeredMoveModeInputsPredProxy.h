// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/XMoveU_SimplePredictionProxyTemplate.h"
#include "UObject/Object.h"
#include "XMoveU_LayeredMoveModeInputsPredProxy.generated.h"


class UXMoveU_ModularMovementComponent;

/**
 *
 */
USTRUCT()
struct XYLOMOVEMENTUTIL_API FXMoveU_LayeredMoveModeInputsPredProxy :
#if CPP
	public XMoveU::TSimplePredictionProxy<uint32, XMoveU::ProxyVar::Traits::ByValue>
#else
	public FXMoveU_SimplePredictionProxy
#endif
{
	GENERATED_BODY()

public:
	FXMoveU_LayeredMoveModeInputsPredProxy()
	{
		bIsInput = true;
		bCheckForError = false;
		CorrectionMode = EXMoveU_CorrectionMode::None;
		bAffectedBySimulation = true;
		bRestoreAfterRollback = true;
		bIsPositionRelated = false;
	}

public:
	void Initialize(FName Name, UXMoveU_ModularMovementComponent* MovementComponent);
protected:
	int32 LayeredMoveModesCount = 0;

protected:
	virtual uint32 MakeDefaulted() override { return 0; }
	virtual bool SerializeInputsAndCorrectionStates_Internal(uint32& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool HasPredictionError_Internal(const uint32& ClientPredictedValue) override;
	virtual bool SerializeCorrectedStates_Internal(uint32& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool CanCombineWithNewFrame_Internal(const uint32& OldFrameValue, const uint32& NewFrameValue) override;
	virtual bool HasNonSimulatedChange(const uint32& LastPostSimValue, const uint32& NewPreSimValue) override;
	virtual bool IsImportantFrame_Internal(const uint32& PreSimValue, const uint32& PostSimValue, const uint32& LastAckedPreSimValue, const uint32& LastAckedPostSimValue) override;
};
