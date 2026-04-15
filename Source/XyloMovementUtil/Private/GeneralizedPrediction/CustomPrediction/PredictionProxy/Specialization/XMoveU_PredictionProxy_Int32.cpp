// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/Specialization/XMoveU_PredictionProxy_Int32.h"

bool FXMoveU_PredictionProxy_Int32::SerializeInputsAndCorrectionStates_Internal(int32& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	Ar << Value;
	return true;
}

bool FXMoveU_PredictionProxy_Int32::HasPredictionError_Internal(int32 ClientPredictedValue)
{
	if (!ProxyVariable->IsValid()) { return false; }
	return ClientPredictedValue != ProxyVariable->Get();
}

bool FXMoveU_PredictionProxy_Int32::SerializeCorrectedStates_Internal(int32& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	Ar << Value;
	return true;
}

bool FXMoveU_PredictionProxy_Int32::CanCombineWithNewFrame_Internal(int32 OldFrameValue, int32 NewFrameValue)
{
	if (!CanCombinePredicate.Predicate.IsSet()) return true;
	return CanCombinePredicate.Predicate(OldFrameValue, NewFrameValue); 
}

bool FXMoveU_PredictionProxy_Int32::HasNonSimulatedChange(int32 LastPostSimValue, int32 NewPreSimValue)
{
	return LastPostSimValue != NewPreSimValue;
}

bool FXMoveU_PredictionProxy_Int32::IsImportantFrame_Internal(int32 PreSimValue, int32 PostSimValue, int32 LastAckedPreSimValue, int32 LastAckedPostSimValue)
{
	if (!IsImportantPredicate.Predicate.IsSet()) return false;
	return IsImportantPredicate.Predicate(PreSimValue, PostSimValue, LastAckedPreSimValue, LastAckedPostSimValue);
}
