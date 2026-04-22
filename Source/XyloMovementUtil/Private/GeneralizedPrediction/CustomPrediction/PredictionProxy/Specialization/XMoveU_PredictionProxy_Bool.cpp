// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/Specialization/XMoveU_PredictionProxy_Bool.h"

bool FXMoveU_PredictionProxy_Bool::SerializeInputsAndCorrectionStates_Internal(bool& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	Ar << Value;
	return true;
}

bool FXMoveU_PredictionProxy_Bool::HasPredictionError_Internal(const bool& ClientPredictedValue)
{
	if (!ProxyVariable->IsValid()) { return false; }
	return ClientPredictedValue != ProxyVariable->Get();
}

bool FXMoveU_PredictionProxy_Bool::SerializeCorrectedStates_Internal(bool& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	Ar << Value;
	return true;
}

bool FXMoveU_PredictionProxy_Bool::CanCombineWithNewFrame_Internal(const bool& OldFrameValue, const bool& NewFrameValue)
{
	if (!CanCombinePredicate.Predicate.IsSet()) return true;
	return CanCombinePredicate.Predicate(OldFrameValue, NewFrameValue); 
}

bool FXMoveU_PredictionProxy_Bool::HasNonSimulatedChange(const bool& LastPostSimValue, const bool& NewPreSimValue)
{
	return LastPostSimValue != NewPreSimValue;
}

bool FXMoveU_PredictionProxy_Bool::IsImportantFrame_Internal(const bool& PreSimValue, const bool& PostSimValue, const bool& LastAckedPreSimValue, const bool& LastAckedPostSimValue)
{
	if (!IsImportantPredicate.Predicate.IsSet()) return false;
	return IsImportantPredicate.Predicate(PreSimValue, PostSimValue, LastAckedPreSimValue, LastAckedPostSimValue);
}
