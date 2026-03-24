// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/XMoveU_SimplePredictionProxyTemplate.h"
#include "XMoveU_PredictionProxy_Float.generated.h"



USTRUCT(BlueprintType, DisplayName="Can Combine Predicate (Float)")
struct FXMoveU_CanCombinePredicate_Float
{
	GENERATED_BODY()

	TFunction<bool(float OldFrameValue, float NewFrameValue)> Predicate;
};

USTRUCT(BlueprintType, DisplayName="Is Important Predicate (Float)")
struct FXMoveU_IsImportantPredicate_Float
{
	GENERATED_BODY()

	TFunction<bool(float PreSimValue, float PostSimValue, float LastAckedPreSimValue, float LastAckedPostSimValue)> Predicate;
};

/**
 * example of usages:
 *	@code
 *	FXMoveU_PredictionProxy_Float Test1;
 *	Test1.SetName("Hello1");
 *	Test1.SetProxyVariable(XMoveU::TProxyVar_Object<ACharacter, float, XMoveU::ProxyVar::Traits::ByValue>::Make({CharacterOwner, &ACharacter::JumpMaxHoldTime}));
 *	
 *	FXMoveU_PredictionProxy_Float Test2;
 *	Test2.SetName("Hello2");
 *	Test2.SetProxyVariable<XMoveU::TProxyVar_Lambda<float, XMoveU::ProxyVar::Traits::ByValue>>({[]()->float{ return 0.f; }, [](float a){}});
 *  @endcode
 */
USTRUCT(BlueprintType, DisplayName="Prediction Proxy (Float)")
struct XYLOMOVEMENTUTIL_API FXMoveU_PredictionProxy_Float :
#if CPP
	public XMoveU::TSimplePredictionProxy<float, XMoveU::ProxyVar::Traits::ByValue>
#else
	public FXMoveU_SimplePredictionProxy
#endif
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayAfter="bCheckForError"))
	float ErrorThreshold = 0.f;

	UPROPERTY(BlueprintReadWrite)
	FXMoveU_CanCombinePredicate_Float CanCombinePredicate;

	UPROPERTY(BlueprintReadWrite)
	FXMoveU_IsImportantPredicate_Float IsImportantPredicate;

protected:
	virtual float MakeDefaulted() override { return 0.f; }
	virtual bool SerializeInputsAndCorrectionStates_Internal(float& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool HasPredictionError_Internal(float ClientPredictedValue) override;
	virtual bool SerializeCorrectedStates_Internal(float& Value, FArchive& Ar, UPackageMap* PackageMap) override;
	virtual bool CanCombineWithNewFrame_Internal(float OldFrameValue, float NewFrameValue) override;
	virtual bool HasNonSimulatedChange(float LastPostSimValue, float NewPreSimValue) override;
	virtual bool IsImportantFrame_Internal(float PreSimValue, float PostSimValue, float LastAckedPreSimValue, float LastAckedPostSimValue) override;
};

