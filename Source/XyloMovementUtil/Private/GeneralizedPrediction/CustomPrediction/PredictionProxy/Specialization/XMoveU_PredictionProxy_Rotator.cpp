// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/Specialization/XMoveU_PredictionProxy_Rotator.h"

bool FXMoveU_PredictionProxy_Rotator::SerializeInputsAndCorrectionStates_Internal(FRotator& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	bool bOutSuccess;
	return Value.NetSerialize(Ar, PackageMap, bOutSuccess);
}

bool FXMoveU_PredictionProxy_Rotator::HasPredictionError_Internal(const FRotator& ClientPredictedValue)
{
	float Tolerance = FMath::Min(ErrorThreshold, 0.0001);
	return ClientPredictedValue.Equals(ProxyVariable->Get(), Tolerance);
}

bool FXMoveU_PredictionProxy_Rotator::SerializeCorrectedStates_Internal(FRotator& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	bool bOutSuccess;
	return Value.NetSerialize(Ar, PackageMap, bOutSuccess);
}

bool FXMoveU_PredictionProxy_Rotator::CanCombineWithNewFrame_Internal(const FRotator& OldFrameValue, const FRotator& NewFrameValue)
{
	if (!CanCombinePredicate.Predicate.IsSet()) return true;
	return CanCombinePredicate.Predicate(OldFrameValue, NewFrameValue); 
}

bool FXMoveU_PredictionProxy_Rotator::HasNonSimulatedChange(const FRotator& LastPostSimValue, const FRotator& NewPreSimValue)
{
	return !LastPostSimValue.Equals(NewPreSimValue);
}

bool FXMoveU_PredictionProxy_Rotator::IsImportantFrame_Internal(const FRotator& PreSimValue, const FRotator& PostSimValue, const FRotator& LastAckedPreSimValue, const FRotator& LastAckedPostSimValue)
{
	if (!IsImportantPredicate.Predicate.IsSet()) return false;
	return IsImportantPredicate.Predicate(PreSimValue, PostSimValue, LastAckedPreSimValue, LastAckedPostSimValue);
}
