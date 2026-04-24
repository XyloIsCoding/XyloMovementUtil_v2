// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/XMoveU_SimplePredictionProxyTemplate.h"
#include "XMoveU_PredictionProxy_UInt32.generated.h"



USTRUCT(BlueprintType, DisplayName="CanCombine Predicate (Unsigned Int)")
struct FXMoveU_CanCombinePredicate_UInt32
{
	GENERATED_BODY()

	TFunction<bool(uint32 OldFrameValue, uint32 NewFrameValue)> Predicate;
};

USTRUCT(BlueprintType, DisplayName="Is Important Predicate (Unsigned Int)")
struct FXMoveU_IsImportantPredicate_UInt32
{
	GENERATED_BODY()

	TFunction<bool(uint32 PreSimValue, uint32 PostSimValue, uint32 LastAckedPreSimValue, uint32 LastAckedPostSimValue)> Predicate;
};

/**
 *
 */
USTRUCT(BlueprintType, DisplayName="Prediction Proxy (Unsigned Int)")
struct XYLOMOVEMENTUTIL_API FXMoveU_PredictionProxy_UInt32 :
#if CPP
	public XMoveU::TSimplePredictionProxy<uint32, XMoveU::ProxyVar::Traits::ByValue>
#else
	public FXMoveU_SimplePredictionProxy
#endif
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FXMoveU_CanCombinePredicate_UInt32 CanCombinePredicate;

	UPROPERTY(BlueprintReadWrite)
	FXMoveU_IsImportantPredicate_UInt32 IsImportantPredicate;
	
	UPROPERTY(BlueprintReadWrite)
	uint8 SerializationBits = 32;

protected:
	virtual uint32 MakeDefaulted() override { return 0; }
	virtual bool SerializeInputsAndCorrectionStates_Internal(uint32& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool HasPredictionError_Internal(const uint32& ClientPredictedValue) override;
	virtual bool SerializeCorrectedStates_Internal(uint32& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool CanCombineWithNewFrame_Internal(const uint32& OldFrameValue, const uint32& NewFrameValue) override;
	virtual bool HasNonSimulatedChange(const uint32& LastPostSimValue, const uint32& NewPreSimValue) override;
	virtual bool IsImportantFrame_Internal(const uint32& PreSimValue, const uint32& PostSimValue, const uint32& LastAckedPreSimValue, const uint32& LastAckedPostSimValue) override;
};
