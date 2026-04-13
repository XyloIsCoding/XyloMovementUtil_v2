// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/XMoveU_JumpStaticLibrary.h"

#include "GameFramework/CharacterMovementComponent.h"


void UXMoveU_JumpStaticLibrary::ApplyJumpImpulse(UCharacterMovementComponent* MoveComp, const FVector& ScaledDirection, float VerticalVelocity, float HorizontalVelocity, bool bOverrideVerticalVelocity, bool bClampHorizontalVelocity, float MaxHorizontalVelocity)
{
	FVector InitialVelocity = MoveComp->Velocity;
	FVector InputDirection = ScaledDirection;
	if (MoveComp->HasCustomGravity())
	{
		InitialVelocity = MoveComp->RotateWorldToGravity(InitialVelocity);
		InputDirection = MoveComp->RotateWorldToGravity(InputDirection);
	}
	
	float NewVelocityZ = InitialVelocity.Z;
	if (VerticalVelocity != 0.f)
	{
		if (bOverrideVerticalVelocity)
		{
			NewVelocityZ = FMath::Max<FVector::FReal>(InitialVelocity.Z, VerticalVelocity);
		}
		else
		{
			NewVelocityZ += VerticalVelocity;
		}
	}

	// If it was requested to clamp the velocity, only increase velocity if less than the threshold.
	FVector NewHorizontalVelocity { InitialVelocity.X, InitialVelocity.Y, 0.f };
	if (HorizontalVelocity != 0.f && (!bClampHorizontalVelocity || (InitialVelocity.SizeSquared2D() < MaxHorizontalVelocity * MaxHorizontalVelocity)))
	{
		// If we increased Horizontal Velocity, make sure we clamp the result.
		NewHorizontalVelocity = NewHorizontalVelocity + InputDirection * HorizontalVelocity;
		if (bClampHorizontalVelocity && NewHorizontalVelocity.SizeSquared2D() > MaxHorizontalVelocity * MaxHorizontalVelocity)
		{
			NewHorizontalVelocity = NewHorizontalVelocity.GetSafeNormal2D() * MaxHorizontalVelocity;
		}
	}

	// Combine results
	FVector FinalVelocity = NewHorizontalVelocity + FVector::UpVector * NewVelocityZ;

	// Apply velocity
	if (MoveComp->HasCustomGravity())
	{
		MoveComp->Velocity = MoveComp->RotateGravityToWorld(FinalVelocity);
	}
	else
	{
		MoveComp->Velocity = FinalVelocity;
	}
}

void UXMoveU_JumpStaticLibrary::ApplyJumpAcceleration(UCharacterMovementComponent* MoveComp, const FVector& ScaledDirection, float VerticalAcceleration, float HorizontalAcceleration, float MaxVerticalVelocity, float MaxHorizontalVelocity, float DeltaTime)
{
	FVector InitialVelocity = MoveComp->Velocity;
	FVector InputDirection = ScaledDirection;
	if (MoveComp->HasCustomGravity())
	{
		InitialVelocity = MoveComp->RotateWorldToGravity(InitialVelocity);
		InputDirection = MoveComp->RotateWorldToGravity(InputDirection);
	}
	
	// Only increase Z velocity if less than threshold and Acceleration was requested
	float NewVelocityZ = InitialVelocity.Z;
	if (VerticalAcceleration != 0.f && (InitialVelocity.Z < MaxVerticalVelocity))
	{
		// If we increased Z Velocity, make sure we clamp the result.
		NewVelocityZ = FMath::Min<FVector::FReal>(MaxVerticalVelocity, NewVelocityZ + VerticalAcceleration * DeltaTime);
	}

	// Only increase Horizontal velocity if less than threshold and Acceleration was requested
	FVector NewHorizontalVelocity { InitialVelocity.X, InitialVelocity.Y, 0.f };
	if (HorizontalAcceleration != 0.f && (InitialVelocity.SizeSquared2D() < MaxHorizontalVelocity * MaxHorizontalVelocity))
	{
		// If we increased Horizontal Velocity, make sure we clamp the result.
		NewHorizontalVelocity = NewHorizontalVelocity + InputDirection * HorizontalAcceleration * DeltaTime;
		if (NewHorizontalVelocity.SizeSquared2D() > MaxHorizontalVelocity * MaxHorizontalVelocity)
		{
			NewHorizontalVelocity = NewHorizontalVelocity.GetSafeNormal2D() * MaxHorizontalVelocity;
		}
	}

	// Combine results
	FVector FinalVelocity = NewHorizontalVelocity + FVector::UpVector * NewVelocityZ;

	// Apply velocity
	if (MoveComp->HasCustomGravity())
	{
		MoveComp->Velocity = MoveComp->RotateGravityToWorld(FinalVelocity);
	}
	else
	{
		MoveComp->Velocity = FinalVelocity;
	}
}

void UXMoveU_JumpStaticLibrary::LimitMinVerticalVelocity(UCharacterMovementComponent* MoveComp, float MinVerticalVelocity)
{
	// We use FMath::Max because we want resulting velocity to be greater than MinVerticalVelocity
	
	if (MoveComp->HasCustomGravity())
	{
		const FVector::FReal ClampedZ = FMath::Max<FVector::FReal>(MoveComp->GetGravitySpaceZ(MoveComp->Velocity), MinVerticalVelocity);
		MoveComp->SetGravitySpaceZ(MoveComp->Velocity, ClampedZ);
	}
	else
	{
		MoveComp->Velocity.Z = FMath::Max<FVector::FReal>(MoveComp->Velocity.Z, MinVerticalVelocity);
	}
}

void UXMoveU_JumpStaticLibrary::LimitMaxVerticalVelocity(UCharacterMovementComponent* MoveComp, float MaxVerticalVelocity)
{
	// We use FMath::Min because we want resulting velocity to be less than MaxVerticalVelocity
	
	if (MoveComp->HasCustomGravity())
	{
		const FVector::FReal ClampedZ = FMath::Min<FVector::FReal>(MoveComp->GetGravitySpaceZ(MoveComp->Velocity), MaxVerticalVelocity);
		MoveComp->SetGravitySpaceZ(MoveComp->Velocity, ClampedZ);
	}
	else
	{
		MoveComp->Velocity.Z = FMath::Min<FVector::FReal>(MoveComp->Velocity.Z, MaxVerticalVelocity);
	}
}

void UXMoveU_JumpStaticLibrary::ClampVerticalVelocity(UCharacterMovementComponent* MoveComp, float MinVerticalVelocity, float MaxVerticalVelocity)
{
	if (MoveComp->HasCustomGravity())
	{
		const FVector::FReal ClampedZ = FMath::Clamp<FVector::FReal>(MoveComp->GetGravitySpaceZ(MoveComp->Velocity), MinVerticalVelocity, MaxVerticalVelocity);
		MoveComp->SetGravitySpaceZ(MoveComp->Velocity, ClampedZ);
	}
	else
	{
		MoveComp->Velocity.Z = FMath::Clamp<FVector::FReal>(MoveComp->Velocity.Z, MinVerticalVelocity, MaxVerticalVelocity);
	}
}
