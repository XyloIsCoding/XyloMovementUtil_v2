// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/Specialization/XMoveU_SpecializedPredictionProxyStatics.h"

#include "Kismet/KismetMathLibrary.h"

FXMoveU_CanCombinePredicate_Bool UXMoveU_SpecializedPredictionProxyStatics::CombineIfEqual_Bool()
{
	return FXMoveU_CanCombinePredicate_Bool([](bool OldFrameValue, bool NewFrameValue)
	{
		return OldFrameValue == NewFrameValue;
	});
}

FXMoveU_CanCombinePredicate_Float UXMoveU_SpecializedPredictionProxyStatics::CombineIfEqual_Float(float Threshold)
{
	return FXMoveU_CanCombinePredicate_Float([Threshold](float OldFrameValue, float NewFrameValue)
	{
		return FMath::IsNearlyEqual(OldFrameValue, NewFrameValue, FMath::Max(Threshold, 1E-08));
	});
}

FXMoveU_CanCombinePredicate_Int32 UXMoveU_SpecializedPredictionProxyStatics::CombineIfEqual_Int()
{
	return FXMoveU_CanCombinePredicate_Int32([](int32 OldFrameValue, int32 NewFrameValue)
	{
		return OldFrameValue == NewFrameValue;
	});
}

FXMoveU_CanCombinePredicate_Rotator UXMoveU_SpecializedPredictionProxyStatics::CombineIfAngleRange_Rotator(float MaxAngle)
{
	float MinCos = FMath::Cos(MaxAngle);
	return FXMoveU_CanCombinePredicate_Rotator([MinCos](const FRotator& OldFrameValue, const FRotator& NewFrameValue)
	{
		float Cosine = OldFrameValue.Vector() | NewFrameValue.Vector();
		return Cosine > MinCos;
	});
}

FXMoveU_CanCombinePredicate_Vector UXMoveU_SpecializedPredictionProxyStatics::CombineIfAngleRange_Vector(float MaxAngle)
{
	float MinCos = FMath::Cos(MaxAngle);
	return FXMoveU_CanCombinePredicate_Vector([MinCos](const FVector& OldFrameValue, const FVector& NewFrameValue)
	{
		float Cosine = OldFrameValue.GetSafeNormal() | NewFrameValue.GetSafeNormal();
		return Cosine > MinCos;
	});
}

FXMoveU_CanCombinePredicate_Vector UXMoveU_SpecializedPredictionProxyStatics::CombineIfEqualMagnitude(float Threshold)
{
	return FXMoveU_CanCombinePredicate_Vector([Threshold](const FVector& OldFrameValue, const FVector& NewFrameValue)
	{
		return FMath::Abs(OldFrameValue.Size() - NewFrameValue.Size()) < Threshold;
	});
}
