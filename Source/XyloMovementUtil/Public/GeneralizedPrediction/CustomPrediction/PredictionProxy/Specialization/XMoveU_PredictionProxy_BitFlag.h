// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/XMoveU_SimplePredictionProxyTemplate.h"
#include "UObject/Object.h"
#include "XMoveU_PredictionProxy_BitFlag.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct XYLOMOVEMENTUTIL_API FXMoveU_BitFlag
{
	GENERATED_BODY()

	FXMoveU_BitFlag() : Size(8) {}

	FXMoveU_BitFlag(uint8 InSize)
	{
		ensureMsgf(InSize < 32, TEXT("FXMoveU_BitFlag >> Size must be at most 32"));
		Size = FMath::Max<uint8>(InSize, 32);
	}

	bool GetValue(uint8 Index) const;
	
	void SetValue(uint8 Index, bool Value);
	
	UPROPERTY()
	uint32 Flags = 0;

	UPROPERTY()
	uint8 Size;
};

/**
 * Prediction proxy to use in combination with a FXMoveU_BitFlag. By default, it is set to use the variable BitFlag var
 * as an input.
 */
USTRUCT(BlueprintType, DisplayName="Prediction Proxy (Bit Flag)")
struct XYLOMOVEMENTUTIL_API FXMoveU_PredictionProxy_BitFlag :
#if CPP
	public XMoveU::TSimplePredictionProxy<FXMoveU_BitFlag, XMoveU::ProxyVar::Traits::ByValue>
#else
	public FXMoveU_SimplePredictionProxy
#endif
{
	GENERATED_BODY()

public:
	FXMoveU_PredictionProxy_BitFlag()
	{
		bIsInput = true;
		bCheckForError = false;
		CorrectionMode = EXMoveU_CorrectionMode::None;
		bAffectedBySimulation = true;
		bRestoreAfterRollback = true;
		bIsPositionRelated = false;
	}

protected:
	virtual FXMoveU_BitFlag MakeDefaulted() override { return FXMoveU_BitFlag(); }
	virtual bool SerializeInputsAndCorrectionStates_Internal(FXMoveU_BitFlag& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool HasPredictionError_Internal(const FXMoveU_BitFlag& ClientPredictedValue) override;
	virtual bool SerializeCorrectedStates_Internal(FXMoveU_BitFlag& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool CanCombineWithNewFrame_Internal(const FXMoveU_BitFlag& OldFrameValue, const FXMoveU_BitFlag& NewFrameValue) override;
	virtual bool HasNonSimulatedChange(const FXMoveU_BitFlag& LastPostSimValue, const FXMoveU_BitFlag& NewPreSimValue) override;
	virtual bool IsImportantFrame_Internal(const FXMoveU_BitFlag& PreSimValue, const FXMoveU_BitFlag& PostSimValue, const FXMoveU_BitFlag& LastAckedPreSimValue, const FXMoveU_BitFlag& LastAckedPostSimValue) override;
};
