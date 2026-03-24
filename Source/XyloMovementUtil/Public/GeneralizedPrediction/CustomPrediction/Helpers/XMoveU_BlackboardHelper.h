// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/CustomPrediction/XMoveU_Blackboard.h"

namespace XMoveU
{
	/**
	 * 
	 */
	template <typename T>
	struct TBlackboardHelper
	{
		TBlackboardHelper() {}
		TBlackboardHelper(FName InVarName) : VarName(InVarName) {}
		
		void Transfer(const FBlackboard& Source, FBlackboard& Target)
		{
			checkf(!VarName.IsNone(), TEXT("XMoveU::TBlackboardHelper::Transfer >> VarName must be set!"))
			Set(Target, Source.GetChecked<T>(VarName));
		}
		
		const T& Get(const FBlackboard& Source) const
		{
			checkf(!VarName.IsNone(), TEXT("XMoveU::TBlackboardHelper::Get >> VarName must be set!"))
			return Source.GetChecked<T>(VarName);
		}
		
		T& Get(FBlackboard& Source)
		{
			checkf(!VarName.IsNone(), TEXT("XMoveU::TBlackboardHelper::Get >> VarName must be set!"))
			return Source.GetChecked<T>(VarName);
		}
		
		void Set(FBlackboard& Target, const T& NewValue)
		{
			checkf(!VarName.IsNone(), TEXT("XMoveU::TBlackboardHelper::Set >> VarName must be set!"))
			if (T* Current = Target.Find<T>(VarName))
			{
				*Current = NewValue;
				return;
			}
			Target.Set(VarName, NewValue);
		}

		FName VarName = NAME_None;
	};
}

