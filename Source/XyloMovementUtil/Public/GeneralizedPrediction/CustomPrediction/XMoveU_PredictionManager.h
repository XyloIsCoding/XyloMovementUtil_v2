// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "PredictionProxy/XMoveU_PredictionProxy.h"
#include "GeneralizedPrediction/MovementComponentPrediction/XMoveU_SavedMove_Character.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"
#include "UObject/Object.h"
#include "XMoveU_PredictionManager.generated.h"

class FXMoveU_NetworkPredictionData_Client_Character;
struct FXMoveU_CharacterNetworkMoveData;
struct FXMoveU_CharacterMoveResponseDataContainer;

/**
 * This is the main protagonist of the generalised prediction system. Once registered to a movement component, it
 * presents a complete interface to CMC's prediction system.
 * It also allows to register prediction proxy structs, which are used to handle the whole prediction cycle of a
 * variable atomically.
 */
UCLASS(DisplayName="Prediction Manager", BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew, Abstract)
class XYLOMOVEMENTUTIL_API UXMoveU_PredictionManager : public UObject
{
	GENERATED_BODY()

public:
	UXMoveU_PredictionManager(const FObjectInitializer& ObjectInitializer);
	
public:
	/** Called when movement component registered this PredictionManager.
	 * This is a good spot to create and register PredictionProxies */
	virtual void OnRegistered(UXMoveU_ModularMovementComponent* MovementComponent) {}

/*====================================================================================================================*/
	// PredictionProxies
	
public:
	void RegisterPredictionProxy(const TSharedPtr<FXMoveU_PredictionProxy>& NewProxy);
	void ForEachProxy(TFunctionRef<void(FXMoveU_PredictionProxy& Proxy, bool& bStopExecution)> InFunction, bool& bOutExecutionStopped);
protected:
	TArray<TSharedPtr<FXMoveU_PredictionProxy>> PredictionProxies;

	// ~PredictionProxies
/*====================================================================================================================*/

/*====================================================================================================================*/
	// MangerInterface
	
public:
	// UCharacterMovementComponent Interface
	virtual void ServerGetClientInputs(UCharacterMovementComponent* MoveComp, const FXMoveU_CharacterNetworkMoveData& MoveData) {}
	virtual bool ServerCheckClientError(UCharacterMovementComponent* MoveComp, float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, bool bIgnorePositionErrors) { return false; }
	virtual void OnClientCorrectionReceived(UCharacterMovementComponent* MoveComp, FXMoveU_NetworkPredictionData_Client_Character& ClientData, float TimeStamp, FVector NewLocation, FVector NewVelocity, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode, FVector ServerGravityDirection) {}
	virtual void CorrectClientPreRollback(UCharacterMovementComponent* MoveComp, const FXMoveU_CharacterMoveResponseDataContainer& MoveResponse, FXMoveU_NetworkPredictionData_Client_Character& ClientData) {}
	virtual void CacheStateBeforeReplay(UCharacterMovementComponent* MoveComp) {}
	virtual void RestoreStateAfterReplay(UCharacterMovementComponent* MoveComp) {}
	// ~UCharacterMovementComponent Interface

	// FSavedMove_Character Interface
	virtual void Clear(FXMoveU_SavedMove_Character& ThisMove) {}
	virtual bool IsImportantMove(const FXMoveU_SavedMove_Character& ThisMove, const FXMoveU_SavedMove_Character* LastAckedMove) const { return false; }
	virtual bool CanCombineWith(const FXMoveU_SavedMove_Character& ThisMove, const FXMoveU_SavedMove_Character& NewMove, ACharacter* InCharacter, float MaxDelta) const { return true; }
	virtual void CombineWith(FXMoveU_SavedMove_Character& ThisMove, const FXMoveU_SavedMove_Character* OldMove, ACharacter* InCharacter, APlayerController* PC, const FVector& OldStartLocation) {}
	virtual void SetMoveFor(FXMoveU_SavedMove_Character& ThisMove, ACharacter* InCharacter, float InDeltaTime, FVector const& NewAccel, FXMoveU_NetworkPredictionData_Client_Character& ClientData) {}
	virtual void SetInitialPosition(FXMoveU_SavedMove_Character& ThisMove, ACharacter* InCharacter) {}
	virtual void PrepMoveFor(FXMoveU_SavedMove_Character& ThisMove, ACharacter* InCharacter) {}
	virtual void PostUpdate(FXMoveU_SavedMove_Character& ThisMove, ACharacter* InCharacter, FSavedMove_Character::EPostUpdateMode PostUpdateMode) {}
	// ~FSavedMove_Character

	// FCharacterNetworkMoveData Interface
	virtual void ClientFillNetworkMoveData(FXMoveU_CharacterNetworkMoveData& ThisNetworkMove, const FXMoveU_SavedMove_Character& ClientMove, FCharacterNetworkMoveData::ENetworkMoveType MoveType) {}
	virtual bool SerializeNetworkMoveData(FXMoveU_CharacterNetworkMoveData& ThisNetworkMove, UCharacterMovementComponent* CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, FCharacterNetworkMoveData::ENetworkMoveType MoveType) { return true; }
	// ~FCharacterNetworkMoveData Interface

	// FCharacterMoveResponseDataContainer Interface
	virtual void ServerFillResponseData(FXMoveU_CharacterMoveResponseDataContainer& ThisResponse, const UCharacterMovementComponent* CharacterMovement, const FClientAdjustment& PendingAdjustment) {}
	virtual bool SerializeResponseData(FXMoveU_CharacterMoveResponseDataContainer& ThisResponse, UCharacterMovementComponent* CharacterMovement, FArchive& Ar, UPackageMap* PackageMap) { return true; }
	// ~FCharacterMoveResponseDataContainer Interface
	
	// ~MangerInterface
/*====================================================================================================================*/
	
};
