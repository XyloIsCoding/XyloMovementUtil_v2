// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/XMoveU_ModularCharacter.h"

#include "ModularMovement/XMoveU_ModularMovementComponent.h"
#include "Net/UnrealNetwork.h"


// Caches CVars from base class and provides getters
// @XMoveU - @AfterUpdatingEngine: check that CVars did not change.
namespace CharacterCVars
{
	static IConsoleVariable* CVar_UseLegacyDoJump = IConsoleManager::Get().FindConsoleVariable(TEXT("p.UseLegacyDoJump"));
	
	int32 Get_UseLegacyDoJump()
	{
		return CharacterCVars::CVar_UseLegacyDoJump->GetInt();
	}
}


AXMoveU_ModularCharacter::AXMoveU_ModularCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UXMoveU_ModularMovementComponent>(ACharacter::CharacterMovementComponentName))
{
}

void AXMoveU_ModularCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AXMoveU_ModularCharacter, LayeredMovementModeStates, COND_SimulatedOnly);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
/*
 * ACharacter Interface
 */

void AXMoveU_ModularCharacter::CheckJumpInput(float DeltaTime)
{
	// We only call super if we are using the default jump input check.
	// The custom sync version is called in UpdateCharacterStateBeforeMovement.
	if (bUseDefaultCheckJumpInput)
	{
		Super::CheckJumpInput(DeltaTime);
	}
}

bool AXMoveU_ModularCharacter::CanJumpInternal_Implementation() const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetCharacterMovement<UXMoveU_ModularMovementComponent>();
	bool bCanJumpWhileCrouched = MoveComp ? MoveComp->CanJumpWhileCrouched() : false;

	// @XMoveU - @SameAsSuper: but conditionally allowing crouch jumping and calling JumpIsAllowed instead of
	// JumpIsAllowedInternal to allow overriding the function.
	return (!IsCrouched() || bCanJumpWhileCrouched) && JumpIsAllowed();
}

void AXMoveU_ModularCharacter::MoveBlockedBy(const FHitResult& Impact)
{
	Super::MoveBlockedBy(Impact);
	MoveBlockedByDelegate.Broadcast(Impact);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         
/*
 * AXMoveU_ModularCharacter
 */
	
/*====================================================================================================================*/
// JumpExtension

void AXMoveU_ModularCharacter::CheckJumpInputSynced(float DeltaTime)
{
	// @XMoveU - @CopiedFromSuper
	JumpCurrentCountPreJump = JumpCurrentCount;

	// @XMoveU - @Change
	UXMoveU_ModularMovementComponent* MoveComp = GetCharacterMovement<UXMoveU_ModularMovementComponent>();
	if (MoveComp && bPressedJump && JumpKeyHoldTime == 0.f)
	{
		// Called one time per input press
		if (MoveComp->TryJumpOverride(DeltaTime))
		{
			StopJumping();
			return;
		}
	}
	// ~@XMoveU - @Change
	
	if (MoveComp)
	{
		if (bPressedJump)
		{
			// If this is the first jump and we're already falling,
			// then increment the JumpCount to compensate.
			const bool bFirstJump = JumpCurrentCount == 0;
			if (bFirstJump && MoveComp->ShouldSkipFirstJump()) // @XMoveU - @Change: using ShouldSkipFirstJump instead of IsFalling
			{
				JumpCurrentCount++;
			}

			// @XMoveU - @AfterUpdatingEngine: Check if legacy DoJump is still a thing.
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
			const bool bDidJump = CanJump() && (CharacterCVars::Get_UseLegacyDoJump())? GetCharacterMovement()->DoJump(bClientUpdating) : GetCharacterMovement()->DoJump(bClientUpdating, DeltaTime);
			PRAGMA_ENABLE_DEPRECATION_WARNINGS
			if (bDidJump)
			{
				// Transition from not (actively) jumping to jumping.
				if (!bWasJumping)
				{
					JumpCurrentCount++;
					JumpForceTimeRemaining = GetJumpMaxHoldTime();

					MoveComp->OnJumped(); // @XMoveU - @Change
					OnJumped();
				}
			}

			bWasJumping = bDidJump;
		}
	}
	// ~@XMoveU - @CopiedFromSuper
}

bool AXMoveU_ModularCharacter::JumpIsAllowed() const
{
	// @XMoveU - @CopiedFromSuper
	// Ensure that the CharacterMovement state is valid
	bool bJumpIsAllowed = GetCharacterMovement()->CanAttemptJump();

	if (bJumpIsAllowed)
	{
		// Ensure JumpHoldTime and JumpCount are valid.
		if (!bWasJumping || GetJumpMaxHoldTime() <= 0.0f)
		{
			UXMoveU_ModularMovementComponent* MoveComp = GetCharacterMovement<UXMoveU_ModularMovementComponent>();
			if (JumpCurrentCount == 0 && MoveComp->ShouldSkipFirstJump()) // @XMoveU - @Change: using ShouldSkipFirstJump instead of IsFalling
			{
				bJumpIsAllowed = JumpCurrentCount + 1 < JumpMaxCount;
			}
			else
			{
				bJumpIsAllowed = JumpCurrentCount < JumpMaxCount;
			}
		}
		else
		{
			// Only consider JumpKeyHoldTime as long as:
			// A) The jump limit hasn't been met OR
			// B) The jump limit has been met AND we were already jumping
			const bool bJumpKeyHeld = (bPressedJump && JumpKeyHoldTime < GetJumpMaxHoldTime());
			bJumpIsAllowed = bJumpKeyHeld &&
				((JumpCurrentCount < JumpMaxCount) || (bWasJumping && JumpCurrentCount == JumpMaxCount));
		}
	}

	return bJumpIsAllowed;
	// ~@XMoveU - @CopiedFromSuper
}

// ~JumpExtension
/*====================================================================================================================*/

/*====================================================================================================================*/
// LayeredMovementModes
	
void AXMoveU_ModularCharacter::OnRep_LayeredMovementModeStates(uint32 OldStates)
{
	if (UXMoveU_ModularMovementComponent* MoveComp = GetCharacterMovement<UXMoveU_ModularMovementComponent>())
	{
		MoveComp->ReplicateLayeredMovementModeStatesToSimProxies(OldStates);
	}
}

// ~LayeredMovementModes
/*====================================================================================================================*/
