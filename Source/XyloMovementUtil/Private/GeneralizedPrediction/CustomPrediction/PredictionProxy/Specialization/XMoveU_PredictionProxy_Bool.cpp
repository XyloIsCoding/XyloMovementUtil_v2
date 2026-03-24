// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/Specialization/XMoveU_PredictionProxy_Bool.h"

bool FXMoveU_PredictionProxy_Bool::SerializeInputsAndCorrectionStates_Internal(bool& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	Ar << Value;
	return true;
}

bool FXMoveU_PredictionProxy_Bool::HasPredictionError_Internal(bool ClientPredictedValue)
{
	return ClientPredictedValue != ProxyVariable->Get();
}

bool FXMoveU_PredictionProxy_Bool::SerializeCorrectedStates_Internal(bool& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	Ar << Value;
	return true;
}

bool FXMoveU_PredictionProxy_Bool::CanCombineWithNewFrame_Internal(bool OldFrameValue, bool NewFrameValue)
{
	if (!CanCombinePredicate.Predicate.IsSet()) return true;
	return CanCombinePredicate.Predicate(OldFrameValue, NewFrameValue); 
}

bool FXMoveU_PredictionProxy_Bool::HasNonSimulatedChange(bool LastPostSimValue, bool NewPreSimValue)
{
	return LastPostSimValue != NewPreSimValue;
}

bool FXMoveU_PredictionProxy_Bool::IsImportantFrame_Internal(bool PreSimValue, bool PostSimValue, bool LastAckedPreSimValue, bool LastAckedPostSimValue)
{
	if (!IsImportantPredicate.Predicate.IsSet()) return false;
	return IsImportantPredicate.Predicate(PreSimValue, PostSimValue, LastAckedPreSimValue, LastAckedPostSimValue);
}
