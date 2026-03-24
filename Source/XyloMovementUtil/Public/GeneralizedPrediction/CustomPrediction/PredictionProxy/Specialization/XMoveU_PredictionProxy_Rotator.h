// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/XMoveU_SimplePredictionProxyTemplate.h"
#include "UObject/Object.h"
#include "XMoveU_PredictionProxy_Rotator.generated.h"



USTRUCT(BlueprintType, DisplayName="Can Combine Predicate (Rotator)")
struct FXMoveU_CanCombinePredicate_Rotator
{
	GENERATED_BODY()

	TFunction<bool(const FRotator& OldFrameValue, const FRotator& NewFrameValue)> Predicate;
};

USTRUCT(BlueprintType, DisplayName="Is Important Predicate (Rotator)")
struct FXMoveU_IsImportantPredicate_Rotator
{
	GENERATED_BODY()

	TFunction<bool(const FRotator& PreSimValue, const FRotator& PostSimValue, const FRotator& LastAckedPreSimValue, const FRotator& LastAckedPostSimValue)> Predicate;
};

/**
 * 
 */
USTRUCT(BlueprintType, DisplayName="Prediction Proxy (Rotator)")
struct XYLOMOVEMENTUTIL_API FXMoveU_PredictionProxy_Rotator :
#if CPP
	public XMoveU::TSimplePredictionProxy<FRotator, XMoveU::ProxyVar::Traits::ByRef>
#else
	public FXMoveU_SimplePredictionProxy
#endif
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayAfter="bCheckForError"))
	float ErrorThreshold = 0.f;

	UPROPERTY(BlueprintReadWrite)
	FXMoveU_CanCombinePredicate_Rotator CanCombinePredicate;

	UPROPERTY(BlueprintReadWrite)
	FXMoveU_IsImportantPredicate_Rotator IsImportantPredicate;

protected:
	virtual FRotator MakeDefaulted() override { return FRotator::ZeroRotator; }
	virtual bool SerializeInputsAndCorrectionStates_Internal(FRotator& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool HasPredictionError_Internal(const FRotator& ClientPredictedValue) override;
	virtual bool SerializeCorrectedStates_Internal(FRotator& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool CanCombineWithNewFrame_Internal(const FRotator& OldFrameValue, const FRotator& NewFrameValue) override;
	virtual bool HasNonSimulatedChange(const FRotator& LastPostSimValue, const FRotator& NewPreSimValue) override;
	virtual bool IsImportantFrame_Internal(const FRotator& PreSimValue, const FRotator& PostSimValue, const FRotator& LastAckedPreSimValue, const FRotator& LastAckedPostSimValue) override;
};