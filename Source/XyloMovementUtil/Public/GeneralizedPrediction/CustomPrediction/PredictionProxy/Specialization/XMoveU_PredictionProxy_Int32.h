// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/XMoveU_SimplePredictionProxyTemplate.h"
#include "UObject/Object.h"
#include "XMoveU_PredictionProxy_Int32.generated.h"



USTRUCT(BlueprintType, DisplayName="CanCombine Predicate (Int)")
struct FXMoveU_CanCombinePredicate_Int32
{
	GENERATED_BODY()

	TFunction<bool(int32 OldFrameValue, int32 NewFrameValue)> Predicate;
};

USTRUCT(BlueprintType, DisplayName="Is Important Predicate (Int)")
struct FXMoveU_IsImportantPredicate_Int32
{
	GENERATED_BODY()

	TFunction<bool(int32 PreSimValue, int32 PostSimValue, int32 LastAckedPreSimValue, int32 LastAckedPostSimValue)> Predicate;
};

/**
 *
 */
USTRUCT(BlueprintType, DisplayName="Prediction Proxy (Int)")
struct XYLOMOVEMENTUTIL_API FXMoveU_PredictionProxy_Int32 :
#if CPP
	public XMoveU::TSimplePredictionProxy<int32, XMoveU::ProxyVar::Traits::ByValue>
#else
	public FXMoveU_SimplePredictionProxy
#endif
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FXMoveU_CanCombinePredicate_Int32 CanCombinePredicate;

	UPROPERTY(BlueprintReadWrite)
	FXMoveU_IsImportantPredicate_Int32 IsImportantPredicate;

protected:
	virtual int32 MakeDefaulted() override { return 0; }
	virtual bool SerializeInputsAndCorrectionStates_Internal(int32& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool HasPredictionError_Internal(int32 ClientPredictedValue) override;
	virtual bool SerializeCorrectedStates_Internal(int32& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool CanCombineWithNewFrame_Internal(int32 OldFrameValue, int32 NewFrameValue) override;
	virtual bool HasNonSimulatedChange(int32 LastPostSimValue, int32 NewPreSimValue) override;
	virtual bool IsImportantFrame_Internal(int32 PreSimValue, int32 PostSimValue, int32 LastAckedPreSimValue, int32 LastAckedPostSimValue) override;
};
