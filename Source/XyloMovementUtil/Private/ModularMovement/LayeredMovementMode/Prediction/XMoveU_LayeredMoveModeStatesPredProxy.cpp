// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/LayeredMovementMode/Prediction/XMoveU_LayeredMoveModeStatesPredProxy.h"

#include "XyloMovementUtil.h"
#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

void FXMoveU_LayeredMoveModeStatesPredProxy::Initialize(FName Name, UXMoveU_ModularMovementComponent* MovementComponent)
{
	AXMoveU_ModularCharacter* Character = Cast<AXMoveU_ModularCharacter>(MovementComponent->GetCharacterOwner());
	
	SetName(Name);
	SetProxyVariable<XMoveU::TProxyVar_Object<AXMoveU_ModularCharacter, uint32, XMoveU::ProxyVar::Traits::ByValue>>({
		Character,
		&AXMoveU_ModularCharacter::LayeredMovementModeStates
	});

	LayeredMoveModesCount = MovementComponent->GetLayeredMovementModes().Num();
}

bool FXMoveU_LayeredMoveModeStatesPredProxy::SerializeInputsAndCorrectionStates_Internal(uint32& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	// This function should not be called, since we do not need to check for these states in order to figure out
	// if we need a correction. Generally if the layered move mode differs, the end location is already different.
	UE_LOG(LogXyloMovementUtil, Warning, TEXT("FXMoveU_LayeredMoveModeStatesPredProxy should not be sent to server to check for errors"))
	return true;
}

bool FXMoveU_LayeredMoveModeStatesPredProxy::HasPredictionError_Internal(uint32 ClientPredictedValue)
{
	UE_LOG(LogXyloMovementUtil, Warning, TEXT("FXMoveU_LayeredMoveModeStatesPredProxy should not be used for error checking"))
	return false;
}

bool FXMoveU_LayeredMoveModeStatesPredProxy::SerializeCorrectedStates_Internal(uint32& Value, FArchive& Ar, UPackageMap* PackageMap)
{
	Ar.SerializeBits(&Value, LayeredMoveModesCount);
	return true;
}

bool FXMoveU_LayeredMoveModeStatesPredProxy::CanCombineWithNewFrame_Internal(uint32 OldFrameValue, uint32 NewFrameValue)
{
	return OldFrameValue == NewFrameValue;
}

bool FXMoveU_LayeredMoveModeStatesPredProxy::HasNonSimulatedChange(uint32 LastPostSimValue, uint32 NewPreSimValue)
{
	return LastPostSimValue != NewPreSimValue;
}

bool FXMoveU_LayeredMoveModeStatesPredProxy::IsImportantFrame_Internal(uint32 PreSimValue, uint32 PostSimValue, uint32 LastAckedPreSimValue, uint32 LastAckedPostSimValue)
{
	return false;
}
