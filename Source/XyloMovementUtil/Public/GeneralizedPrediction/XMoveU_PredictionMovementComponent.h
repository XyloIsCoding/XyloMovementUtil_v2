// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomPrediction/PredictionProxy/XMoveU_PredictionProxy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MovementComponentPrediction/XMoveU_CharacterMoveResponseDataContainer.h"
#include "MovementComponentPrediction/XMoveU_CharacterNetworkMoveDataContainer.h"
#include "XMoveU_PredictionMovementComponent.generated.h"

class UXMoveU_PredictionManager;

namespace CharacterMovementCVars
{
	float Get_ClientAuthorityThresholdOnBaseChange();
	float Get_MaxFallingCorrectionLeash();
	float Get_MaxFallingCorrectionLeashBuffer();
	int32 Get_NetUseBaseRelativeVelocity();
	int32 Get_NetShowCorrections();
	float Get_NetCorrectionLifetime();
}


DECLARE_MULTICAST_DELEGATE(FXMoveU_MoveReplayFinishedSignature)

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class XYLOMOVEMENTUTIL_API UXMoveU_PredictionMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UXMoveU_PredictionMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
private:
	FXMoveU_CharacterMoveResponseDataContainer MoveResponseDataContainer;
	FXMoveU_CharacterNetworkMoveDataContainer NetworkMoveDataContainer;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UCharacterMovementComponent Interface
	 */

public:
	virtual void BeginPlay() override;
	
public:
	virtual void ServerMove_PerformMovement(const FCharacterNetworkMoveData& MoveData) override;
protected:
	virtual void OnClientCorrectionReceived(class FNetworkPredictionData_Client_Character& ClientData, float TimeStamp, FVector NewLocation, FVector NewVelocity, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode, FVector ServerGravityDirection) override;
	virtual bool ClientUpdatePositionAfterServerUpdate() override;
protected:
	virtual void ServerMoveHandleClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) override;

/*--------------------------------------------------------------------------------------------------------------------*/
	// ServerMoveHandleClientError Helpers

protected:
	/** Deals with mitigating corrections when the client if falling / during landing or lift-offs. */
	virtual bool IsFallingWithinAcceptableError(const FVector& ClientWorldLocation, uint8 ClientMovementMode, bool& bUseLastBase, FVector& ServerWorldLocation, UPrimitiveComponent* ServerMovementBase, FVector& RelativeLocation, FVector& RelativeVelocity);
	/** Prepares ServerData to send a correction to client. */
	virtual void ServerPrepareCorrection(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, const FVector& ServerWorldLocation, bool bUseLastBase, UPrimitiveComponent* ServerMovementBase, FName ServerBaseBoneName, const FVector& RelativeLocation, const FVector& RelativeVelocity);
	/** Adjusts server position to client one if we are using client authoritative movement. */
	virtual void ServerUseAuthoritativePosition(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);

	virtual bool ShouldDeferServerCorrectionsWhenFalling();
	virtual FVector CalculateClientWorldLocation(const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName);
	virtual void TryUseServerBaseForClientBase(UPrimitiveComponent*& ClientMovementBase, FName& ClientBaseBoneName, uint8 ClientMovementMode);

	// ~ServerMoveHandleClientError Helpers
/*--------------------------------------------------------------------------------------------------------------------*/
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UXMoveU_PredictionMovementComponent
	 */

public:
	FXMoveU_MoveReplayFinishedSignature MoveReplayFinishedDelegate;
protected:
	virtual void ServerGetClientInputs(const FCharacterNetworkMoveData& MoveData);
	virtual void CacheStateBeforeReplay();
	virtual void RestoreStateAfterReplay();
	
	/** We use this function instead of ServerCheckClientError because that one is position focused, and is ignored
	 * under certain conditions, like transitioning on and off moving platforms. */
	virtual bool ServerCheckGenericClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, bool bIgnorePositionErrors);

/*====================================================================================================================*/
	// CustomPrediction

public:
	UFUNCTION(Category="Pawn|Components|CharacterMovement|Networking", BlueprintCallable)
	virtual void RegisterPredictionManager(UXMoveU_PredictionManager* NewPredictionManager);
	
public:
	static void ExecuteOnPredictionManagers(const UCharacterMovementComponent* MoveComp, TFunctionRef<void(UXMoveU_PredictionManager*, bool&)> InFunction);
	static void ExecuteOnPredictionProxies(const UCharacterMovementComponent* MoveComp, TFunctionRef<void(FXMoveU_PredictionProxy&, bool&)> InFunction);

protected:
	/** Override to add Prediction Managers from other sources. */
	virtual void GetPredictionManagers(TArray<UXMoveU_PredictionManager*>& OutPredictionManagers) const;

protected:
	UPROPERTY()
	TArray<TObjectPtr<UXMoveU_PredictionManager>> PredictionManagers;

	UPROPERTY(Category="Character Movement (Networking)", EditDefaultsOnly, Instanced, BlueprintReadOnly)
	TObjectPtr<UXMoveU_PredictionManager> DefaultPredictionManager;

	// ~CustomPrediction
/*====================================================================================================================*/

/*====================================================================================================================*/
	// CustomAccessors

protected:
	void SetLastServerMovementBase(UPrimitiveComponent* InMovementBase);
	
	// ~CustomAccessors
/*====================================================================================================================*/
	
};
