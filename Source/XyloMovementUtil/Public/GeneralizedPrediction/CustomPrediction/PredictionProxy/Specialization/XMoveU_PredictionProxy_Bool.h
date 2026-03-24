// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/XMoveU_SimplePredictionProxyTemplate.h"
#include "UObject/Object.h"
#include "XMoveU_PredictionProxy_Bool.generated.h"


USTRUCT(BlueprintType, DisplayName="Can Combine Predicate (Bool)")
struct FXMoveU_CanCombinePredicate_Bool
{
	GENERATED_BODY()

	TFunction<bool(bool OldFrameValue, bool NewFrameValue)> Predicate;
};

USTRUCT(BlueprintType, DisplayName="Is Important Predicate (Bool)")
struct FXMoveU_IsImportantPredicate_Bool
{
	GENERATED_BODY()

	TFunction<bool(bool PreSimValue, bool PostSimValue, bool LastAckedPreSimValue, bool LastAckedPostSimValue)> Predicate;
};

/**
 *
 */
USTRUCT(BlueprintType, DisplayName="Prediction Proxy (Bool)")
struct XYLOMOVEMENTUTIL_API FXMoveU_PredictionProxy_Bool :
#if CPP
	public XMoveU::TSimplePredictionProxy<bool, XMoveU::ProxyVar::Traits::ByValue>
#else
	public FXMoveU_SimplePredictionProxy
#endif
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FXMoveU_CanCombinePredicate_Bool CanCombinePredicate;

	UPROPERTY(BlueprintReadWrite)
	FXMoveU_IsImportantPredicate_Bool IsImportantPredicate;

protected:
	virtual bool MakeDefaulted() override { return false; }
	virtual bool SerializeInputsAndCorrectionStates_Internal(bool& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool HasPredictionError_Internal(bool ClientPredictedValue) override;
	virtual bool SerializeCorrectedStates_Internal(bool& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool CanCombineWithNewFrame_Internal(bool OldFrameValue, bool NewFrameValue) override;
	virtual bool HasNonSimulatedChange(bool LastPostSimValue, bool NewPreSimValue) override;
	virtual bool IsImportantFrame_Internal(bool PreSimValue, bool PostSimValue, bool LastAckedPreSimValue, bool LastAckedPostSimValue) override;
};
