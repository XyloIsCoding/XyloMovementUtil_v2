// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/Specialization/XMoveU_PredictionProxy_Float.h"


bool FXMoveU_PredictionProxy_Float::SerializeInputsAndCorrectionStates_Internal(float& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	Ar << Value;
	return true;
}

bool FXMoveU_PredictionProxy_Float::HasPredictionError_Internal(float ClientPredictedValue)
{
	float Tolerance = FMath::Min(ErrorThreshold, 1E-08);
	return FMath::IsNearlyEqual(ProxyVariable->Get(), ClientPredictedValue, Tolerance);
}

bool FXMoveU_PredictionProxy_Float::SerializeCorrectedStates_Internal(float& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	Ar << Value;
	return true;
}

bool FXMoveU_PredictionProxy_Float::CanCombineWithNewFrame_Internal(float OldFrameValue, float NewFrameValue)
{
	if (!CanCombinePredicate.Predicate.IsSet()) return true;
	return CanCombinePredicate.Predicate(OldFrameValue, NewFrameValue);
}

bool FXMoveU_PredictionProxy_Float::HasNonSimulatedChange(float LastPostSimValue, float NewPreSimValue)
{
	return !FMath::IsNearlyEqual(LastPostSimValue, NewPreSimValue);
}

bool FXMoveU_PredictionProxy_Float::IsImportantFrame_Internal(float PreSimValue, float PostSimValue, float LastAckedPreSimValue, float LastAckedPostSimValue)
{
	if (!IsImportantPredicate.Predicate.IsSet()) return false;
	return IsImportantPredicate.Predicate(PreSimValue, PostSimValue, LastAckedPreSimValue, LastAckedPostSimValue);
}

