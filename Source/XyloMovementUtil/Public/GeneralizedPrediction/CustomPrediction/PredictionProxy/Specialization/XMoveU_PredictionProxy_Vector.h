// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/XMoveU_SimplePredictionProxyTemplate.h"
#include "UObject/Object.h"
#include "XMoveU_PredictionProxy_Vector.generated.h"



USTRUCT(BlueprintType, DisplayName="Can Combine Predicate (Vector)")
struct FXMoveU_CanCombinePredicate_Vector
{
	GENERATED_BODY()

	TFunction<bool(const FVector& OldFrameValue, const FVector& NewFrameValue)> Predicate;
};

USTRUCT(BlueprintType, DisplayName="Is Important Predicate (Vector)")
struct FXMoveU_IsImportantPredicate_Vector
{
	GENERATED_BODY()

	TFunction<bool(const FVector& PreSimValue, const FVector& PostSimValue, const FVector& LastAckedPreSimValue, const FVector& LastAckedPostSimValue)> Predicate;
};

/**
 * 
 */
USTRUCT(BlueprintType, DisplayName="Prediction Proxy (Vector)")
struct XYLOMOVEMENTUTIL_API FXMoveU_PredictionProxy_Vector :
#if CPP
	public XMoveU::TSimplePredictionProxy<FVector, XMoveU::ProxyVar::Traits::ByRef>
#else
	public FXMoveU_SimplePredictionProxy
#endif
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayAfter="bCheckForError"))
	float ErrorThreshold = 0.f;

	UPROPERTY(BlueprintReadWrite)
	FXMoveU_CanCombinePredicate_Vector CanCombinePredicate;

	UPROPERTY(BlueprintReadWrite)
	FXMoveU_IsImportantPredicate_Vector IsImportantPredicate;

protected:
	virtual FVector MakeDefaulted() override { return FVector::ZeroVector; }
	virtual bool SerializeInputsAndCorrectionStates_Internal(FVector& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool HasPredictionError_Internal(const FVector& ClientPredictedValue) override;
	virtual bool SerializeCorrectedStates_Internal(FVector& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool CanCombineWithNewFrame_Internal(const FVector& OldFrameValue, const FVector& NewFrameValue) override;
	virtual bool HasNonSimulatedChange(const FVector& LastPostSimValue, const FVector& NewPreSimValue) override;
	virtual bool IsImportantFrame_Internal(const FVector& PreSimValue, const FVector& PostSimValue, const FVector& LastAckedPreSimValue, const FVector& LastAckedPostSimValue) override;
};
