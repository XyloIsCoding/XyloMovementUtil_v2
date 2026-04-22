// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/LayeredMovementMode/Prediction/XMoveU_LayeredMoveModeInputsPredProxy.h"

#include "XyloMovementUtil.h"
#include "GeneralizedPrediction/CustomPrediction/PredictionProxy/Specialization/XMoveU_PredictionProxy_Float.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

void FXMoveU_LayeredMoveModeInputsPredProxy::Initialize(FName Name, UXMoveU_ModularMovementComponent* MovementComponent)
{
	SetName(Name);
	SetProxyVariable(XMoveU::TProxyVar_Lambda<uint32>(
		MovementComponent,
		[MovementComponent]()->uint32
		{
			return MovementComponent->GetLayeredMovementModesRequests();
		},
		[MovementComponent](uint32 Inputs)
		{
			MovementComponent->SetLayeredMovementModesRequests(Inputs);
		}
	));

	LayeredMoveModesCount = MovementComponent->GetLayeredMovementModes().Num();
}

bool FXMoveU_LayeredMoveModeInputsPredProxy::SerializeInputsAndCorrectionStates_Internal(uint32& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	Ar.SerializeBits(&Value, LayeredMoveModesCount);
	// UE_LOG(LogTemp, Warning, TEXT("LayeredMoveModeInputs: %i (%s)"), Value, *(Ar.IsSaving() ? FString(TEXT("Client")) : FString(TEXT("Server"))))
	return true;
}

bool FXMoveU_LayeredMoveModeInputsPredProxy::HasPredictionError_Internal(const uint32& ClientPredictedValue)
{
	UE_LOG(LogXyloMovementUtil, Warning, TEXT("FXMoveU_LayeredMoveModeInputsPredProxy should not be used for error checking"))
	return false;
}

bool FXMoveU_LayeredMoveModeInputsPredProxy::SerializeCorrectedStates_Internal(uint32& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	UE_LOG(LogXyloMovementUtil, Warning, TEXT("FXMoveU_LayeredMoveModeStatesPredProxy should not be sent to client as correction"))
	return true;
}

bool FXMoveU_LayeredMoveModeInputsPredProxy::CanCombineWithNewFrame_Internal(const uint32& OldFrameValue, const uint32& NewFrameValue)
{
	return OldFrameValue == NewFrameValue;
}

bool FXMoveU_LayeredMoveModeInputsPredProxy::HasNonSimulatedChange(const uint32& LastPostSimValue, const uint32& NewPreSimValue)
{
	return LastPostSimValue != NewPreSimValue;
}

bool FXMoveU_LayeredMoveModeInputsPredProxy::IsImportantFrame_Internal(const uint32& PreSimValue, const uint32& PostSimValue, const uint32& LastAckedPreSimValue, const uint32& LastAckedPostSimValue)
{
	// Input change makes this move important.
	return PreSimValue != LastAckedPreSimValue;
}
