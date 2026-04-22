// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/Specialization/XMoveU_PredictionProxy_BitFlag.h"

bool FXMoveU_BitFlag::GetValue(uint8 Index) const
{
	if (!ensureMsgf(Index < Size, TEXT("FXMoveU_BitFlag::GetValue >> Index %i exceds BitFlag size %i"), Index, Size))
	{
		return false;
	}
	
	return ((Flags & (1 << Index)) != 0);
}

void FXMoveU_BitFlag::SetValue(uint8 Index, bool Value)
{
	if (!ensureMsgf(Index < Size, TEXT("FXMoveU_BitFlag::SetValue >> Index %i exceds BitFlag size %i"), Index, Size))
	{
		return;
	}
	
	if (Value)
	{
		// 0100010 Flags
		// 0000100 1 << Index
		// 0100110 Flags | (1 << Index)
		Flags |= (1 << Index);  
	}
	else
	{
		// 0100110 Flags
		// 0000100 1 << Index
		// 1111011 ~(1 << Index)
		// 0100010 Flags & ~(1 << Index)
		Flags &= ~(1 << Index);
	}
}

bool FXMoveU_PredictionProxy_BitFlag::SerializeInputsAndCorrectionStates_Internal(FXMoveU_BitFlag& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	bool bIsZero = Value.Flags == 0;
	Ar.SerializeBits(&bIsZero, 1);

	if (!bIsZero)
	{
		Ar.SerializeBits(&Value.Flags, Value.Size);
	}
	return true;
}

bool FXMoveU_PredictionProxy_BitFlag::HasPredictionError_Internal(const FXMoveU_BitFlag& ClientPredictedValue)
{
	if (!ProxyVariable->IsValid()) { return false; }
	return ClientPredictedValue.Flags != ProxyVariable->Get().Flags; 
}

bool FXMoveU_PredictionProxy_BitFlag::SerializeCorrectedStates_Internal(FXMoveU_BitFlag& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	bool bIsZero = Value.Flags == 0;
	Ar.SerializeBits(&bIsZero, 1);

	if (!bIsZero)
	{
		Ar.SerializeBits(&Value.Flags, Value.Size);
	}
	return true;
}

bool FXMoveU_PredictionProxy_BitFlag::CanCombineWithNewFrame_Internal(const FXMoveU_BitFlag& OldFrameValue, const FXMoveU_BitFlag& NewFrameValue)
{
	return OldFrameValue.Flags == NewFrameValue.Flags;
}

bool FXMoveU_PredictionProxy_BitFlag::HasNonSimulatedChange(const FXMoveU_BitFlag& LastPostSimValue, const FXMoveU_BitFlag& NewPreSimValue)
{
	return LastPostSimValue.Flags != NewPreSimValue.Flags;
}

bool FXMoveU_PredictionProxy_BitFlag::IsImportantFrame_Internal(const FXMoveU_BitFlag& PreSimValue, const FXMoveU_BitFlag& PostSimValue, const FXMoveU_BitFlag& LastAckedPreSimValue, const FXMoveU_BitFlag& LastAckedPostSimValue)
{
	return PreSimValue.Flags != LastAckedPreSimValue.Flags;
}
