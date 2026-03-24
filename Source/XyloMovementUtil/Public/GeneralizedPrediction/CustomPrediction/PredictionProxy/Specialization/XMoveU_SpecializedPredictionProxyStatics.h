// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XMoveU_PredictionProxy_Bool.h"
#include "XMoveU_PredictionProxy_Float.h"
#include "XMoveU_PredictionProxy_Int32.h"
#include "XMoveU_PredictionProxy_Rotator.h"
#include "XMoveU_PredictionProxy_Vector.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XMoveU_SpecializedPredictionProxyStatics.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Specialized Prediction Proxy Statics")
class XYLOMOVEMENTUTIL_API UXMoveU_SpecializedPredictionProxyStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName="Combine If Equal (Bool)")
	static FXMoveU_CanCombinePredicate_Bool CombineIfEqual_Bool();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName="Combine If Equal (Float)")
	static FXMoveU_CanCombinePredicate_Float CombineIfEqual_Float(float Threshold);

	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName="Combine If Equal (Int)")
	static FXMoveU_CanCombinePredicate_Int32 CombineIfEqual_Int();

	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName="Combine If Angle Range (Rotator)")
	static FXMoveU_CanCombinePredicate_Rotator CombineIfAngleRange_Rotator(float MaxAngle);

	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName="Combine If Angle Range (Vector)")
	static FXMoveU_CanCombinePredicate_Vector CombineIfAngleRange_Vector(float MaxAngle);

	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName="Combine If Equal Magnitude (Vector)")
	static FXMoveU_CanCombinePredicate_Vector CombineIfEqualMagnitude(float Threshold);
};
