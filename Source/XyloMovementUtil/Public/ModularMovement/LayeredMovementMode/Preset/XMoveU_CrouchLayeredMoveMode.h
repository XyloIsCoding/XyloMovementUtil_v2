// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularMovement/LayeredMovementMode/XMoveU_LayeredMovementMode.h"
#include "XMoveU_CrouchLayeredMoveMode.generated.h"

/**
 * This layered mode just calls the native CharacterMovementComponent functions for crouching.
 * The reason behind the existence of this mode is to be able to choose when crouch logic is checked and executed in
 * relation to other layered movement modes.
 *
 * If this is used, we need to make sure that we are not checking for crouch transitions inside cmc
 * (UpdateCharacterStateBeforeMovement and UpdateCharacterStateAfterMovement).
 */
UCLASS()
class XYLOMOVEMENTUTIL_API UXMoveU_CrouchLayeredMoveMode : public UXMoveU_LayeredMovementMode
{
	GENERATED_BODY()

public:
	UXMoveU_CrouchLayeredMoveMode(const FObjectInitializer& ObjectInitializer);

public:
	virtual void RequestMode(bool bWantsToEnterMode) override;
	virtual bool WantsToBeInMode() const override;
	virtual void SetModeState(bool bIsInMode) override;
	virtual bool IsInMode() const override;
	virtual void ReplicateStateToSimProxies() override;

public:
	virtual bool ShouldEnterMode(float DeltaSeconds) const override;
	virtual bool ShouldLeaveMode(float DeltaSeconds) const override;
	virtual bool ShouldForceLeaveMode(float DeltaSeconds) const override;
protected:
	virtual bool CanCrouchInCurrentState() const;

protected:
	virtual void EnterMode(bool bClientSimulation) override;
	virtual void LeaveMode(bool bClientSimulation) override;
	
};
