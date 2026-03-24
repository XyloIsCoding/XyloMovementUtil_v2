// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/Specialization/XMoveU_PredictionProxy_Vector.h"

bool FXMoveU_PredictionProxy_Vector::SerializeInputsAndCorrectionStates_Internal(FVector& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	bool bOutSuccess;
	return Value.NetSerialize(Ar, PackageMap, bOutSuccess);
}

bool FXMoveU_PredictionProxy_Vector::HasPredictionError_Internal(const FVector& ClientPredictedValue)
{
	float Tolerance = FMath::Min(ErrorThreshold, 0.0001);
	return ClientPredictedValue.Equals(ProxyVariable->Get(), Tolerance);
}

bool FXMoveU_PredictionProxy_Vector::SerializeCorrectedStates_Internal(FVector& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	bool bOutSuccess;
	return Value.NetSerialize(Ar, PackageMap, bOutSuccess);
}

bool FXMoveU_PredictionProxy_Vector::CanCombineWithNewFrame_Internal(const FVector& OldFrameValue, const FVector& NewFrameValue)
{
	if (!CanCombinePredicate.Predicate.IsSet()) return true;
	return CanCombinePredicate.Predicate(OldFrameValue, NewFrameValue); 
}

bool FXMoveU_PredictionProxy_Vector::HasNonSimulatedChange(const FVector& LastPostSimValue, const FVector& NewPreSimValue)
{
	return !LastPostSimValue.Equals(NewPreSimValue);
}

bool FXMoveU_PredictionProxy_Vector::IsImportantFrame_Internal(const FVector& PreSimValue, const FVector& PostSimValue, const FVector& LastAckedPreSimValue, const FVector& LastAckedPostSimValue)
{
	if (!IsImportantPredicate.Predicate.IsSet()) return false;
	return IsImportantPredicate.Predicate(PreSimValue, PostSimValue, LastAckedPreSimValue, LastAckedPostSimValue);
}
