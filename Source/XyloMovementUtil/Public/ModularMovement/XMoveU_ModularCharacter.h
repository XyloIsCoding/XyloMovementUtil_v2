// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/XMoveU_PredictionCharacter.h"
#include "XMoveU_ModularCharacter.generated.h"


namespace CharacterCVars
{
	int32 Get_UseLegacyDoJump();
}


DECLARE_MULTICAST_DELEGATE_OneParam(FXMoveU_MoveBlockedBySignature, const FHitResult& /* Impact*/)


/**
 *
 */
UCLASS()
class XYLOMOVEMENTUTIL_API AXMoveU_ModularCharacter : public AXMoveU_PredictionCharacter
{
	GENERATED_BODY()

public:
	AXMoveU_ModularCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/*
	 * ACharacter Interface
	 */

public:
	virtual void CheckJumpInput(float DeltaTime) override;
protected:
	virtual bool CanJumpInternal_Implementation() const override;

public:
	virtual void MoveBlockedBy(const FHitResult& Impact) override;
    	 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         
	/*
	 * AXMoveU_ModularCharacter
	 */

/*====================================================================================================================*/
	// ImprovedInterface

public:
	FXMoveU_MoveBlockedBySignature MoveBlockedByDelegate;
	
	// ~ImprovedInterface
/*====================================================================================================================*/
	
/*====================================================================================================================*/
	// JumpExtension
	
public:
	/** Like CheckJumpInput, but called from UpdateCharacterStateBeforeMovement */
	virtual void CheckJumpInputSynced(float DeltaTime);
protected:
	/** Virtual version of JumpIsAllowedInternal (called instead of it) */
	virtual bool JumpIsAllowed() const;

public:
	bool ShouldUseDefaultCheckJumpInput() const { return bUseDefaultCheckJumpInput; }
protected:
	/** If true uses CheckJumpInput, otherwise CheckJumpInputSynced is used. The latter has the same logic as the first,
	 * but it is called in UpdateCharacterStateBeforeMovement */
	UPROPERTY(Category = "Character", EditAnywhere)
	bool bUseDefaultCheckJumpInput = false;

	// ~JumpExtension
/*====================================================================================================================*/

/*====================================================================================================================*/
	// LayeredMovementModes
	
public:
	UPROPERTY(ReplicatedUsing="OnRep_LayeredMovementModeStates")
	uint32 LayeredMovementModeStates;

	UFUNCTION()
	virtual void OnRep_LayeredMovementModeStates(uint32 OldStates);

	// ~LayeredMovementModes
/*====================================================================================================================*/
	
};
