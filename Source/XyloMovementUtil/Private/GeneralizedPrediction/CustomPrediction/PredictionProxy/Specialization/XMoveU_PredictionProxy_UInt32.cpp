// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/Specialization/XMoveU_PredictionProxy_UInt32.h"

bool FXMoveU_PredictionProxy_UInt32::SerializeInputsAndCorrectionStates_Internal(uint32& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	Ar.SerializeBits(&Value, SerializationBits);
	return true;
}

bool FXMoveU_PredictionProxy_UInt32::HasPredictionError_Internal(const uint32& ClientPredictedValue)
{
	if (!ProxyVariable->IsValid()) { return false; }
	return ClientPredictedValue != ProxyVariable->Get();
}

bool FXMoveU_PredictionProxy_UInt32::SerializeCorrectedStates_Internal(uint32& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	Ar.SerializeBits(&Value, SerializationBits);
	return true;
}

bool FXMoveU_PredictionProxy_UInt32::CanCombineWithNewFrame_Internal(const uint32& OldFrameValue, const uint32& NewFrameValue)
{
	if (!CanCombinePredicate.Predicate.IsSet()) return true;
	return CanCombinePredicate.Predicate(OldFrameValue, NewFrameValue); 
}

bool FXMoveU_PredictionProxy_UInt32::HasNonSimulatedChange(const uint32& LastPostSimValue, const uint32& NewPreSimValue)
{
	return LastPostSimValue != NewPreSimValue;
}

bool FXMoveU_PredictionProxy_UInt32::IsImportantFrame_Internal(const uint32& PreSimValue, const uint32& PostSimValue, const uint32& LastAckedPreSimValue, const uint32& LastAckedPostSimValue)
{
	if (!IsImportantPredicate.Predicate.IsSet()) return false;
	return IsImportantPredicate.Predicate(PreSimValue, PostSimValue, LastAckedPreSimValue, LastAckedPostSimValue);
}
