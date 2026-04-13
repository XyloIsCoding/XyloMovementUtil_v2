// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XMoveU_JumpStaticLibrary.generated.h"

class UCharacterMovementComponent;
/**
 * 
 */
UCLASS()
class XYLOMOVEMENTUTIL_API UXMoveU_JumpStaticLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "JumpStaticLibrary", BlueprintCallable)
	static void ApplyJumpImpulse(UCharacterMovementComponent* MoveComp, const FVector& ScaledDirection, float VerticalVelocity, float HorizontalVelocity, bool bOverrideVerticalVelocity, bool bClampHorizontalVelocity = false, float MaxHorizontalVelocity = 0.f);

	UFUNCTION(Category = "JumpStaticLibrary", BlueprintCallable)
	static void ApplyJumpAcceleration(UCharacterMovementComponent* MoveComp, const FVector& ScaledDirection, float VerticalAcceleration, float HorizontalAcceleration, float MaxVerticalVelocity, float MaxHorizontalVelocity, float DeltaTime);
	
	UFUNCTION(Category = "JumpStaticLibrary", BlueprintCallable)
	static void LimitMinVerticalVelocity(UCharacterMovementComponent* MoveComp, float MinVerticalVelocity);

	UFUNCTION(Category = "JumpStaticLibrary", BlueprintCallable)
	static void LimitMaxVerticalVelocity(UCharacterMovementComponent* MoveComp, float MaxVerticalVelocity);

	UFUNCTION(Category = "JumpStaticLibrary", BlueprintCallable)
	static void ClampVerticalVelocity(UCharacterMovementComponent* MoveComp, float MinVerticalVelocity, float MaxVerticalVelocity);
};
