// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace XMoveU
{
	namespace ProxyVar::Traits
	{
		template <typename T> struct ByValue { using ReturnType = T; using ReturnConstType = T; using ParamType = T; }; 
		template <typename T> struct ByRef { using ReturnType = T&; using ReturnConstType = const T&; using ParamType = const T&; };
	}
	

	/**
	 *
	 */
	template <typename  T, template <typename> typename Traits>
	struct TProxyVarInterface
	{
		using FTraits = Traits<T>;
		using ReturnType = typename FTraits::ReturnType;
		using ReturnConstType = typename FTraits::ReturnConstType;
		using ParamType = typename FTraits::ParamType;
		
		TProxyVarInterface() {}
		virtual ~TProxyVarInterface() {}
		
		virtual ReturnConstType Get() const = 0;
		virtual void Set(ParamType NewValue) = 0;
	};

	/**
	 *
	 */
	template <
		typename  Class,
		typename  T,
		template <typename> typename Traits = ProxyVar::Traits::ByValue
	> requires TIsDerivedFrom<Class, UObject>::Value
	struct TProxyVar_Object : public TProxyVarInterface<T, Traits>
	{
		using FTraits = Traits<T>;
		using ReturnType = typename FTraits::ReturnType;
		using ReturnConstType = typename FTraits::ReturnConstType;
		using ParamType = typename FTraits::ParamType;

		static TSharedPtr<TProxyVar_Object> Make(TProxyVar_Object InProxyVar)
		{
			return MakeShared<TProxyVar_Object>(InProxyVar);
		}
		
		TProxyVar_Object(Class* InObject, T Class::* InMemberPtr) : Object(InObject), MemberPtr(InMemberPtr) {}
		
		virtual ReturnConstType Get() const override
		{
			check(Object.IsValid());
			return (*Object).*MemberPtr;
		}
		
		ReturnType Get()
		{
			check(Object.IsValid());
			return (*Object).*MemberPtr;
		}
		
		virtual void Set(ParamType NewValue) override
		{
			check(Object.IsValid());
			(*Object).*MemberPtr = NewValue;
		}

	private:
		TWeakObjectPtr<Class> Object;
		T Class::* MemberPtr;
	};


	/**
	 *
	 */
	template <
		typename  T,
		template <typename> typename Traits = ProxyVar::Traits::ByValue
	>
	struct TProxyVar_Lambda : public TProxyVarInterface<T, Traits>
	{
		using FTraits = Traits<T>;
		using ReturnType = typename FTraits::ReturnType;
		using ReturnConstType = typename FTraits::ReturnConstType;
		using ParamType = typename FTraits::ParamType;

		static TSharedPtr<TProxyVar_Lambda> Make(TProxyVar_Lambda InProxyVar)
		{
			return MakeShared<TProxyVar_Lambda>(InProxyVar);
		}
		
		TProxyVar_Lambda(TFunction<ReturnConstType()> InGetter, TFunction<void(ParamType)> InSetter)
		{
			Getter = InGetter;
			Setter = InSetter;
		}

		TFunction<ReturnConstType()> Getter;
		TFunction<void(ParamType)> Setter;
		
		virtual ReturnConstType Get() const override
		{
			return Getter();
		}
		
		virtual void Set(ParamType NewValue) override
		{
			Setter(NewValue);
		}
	};
}