// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"

namespace XMoveU
{
	/**
	 * 
	 */
	class XYLOMOVEMENTUTIL_API FBlackboard
	{
	public:
		FBlackboard() {}
		FBlackboard(const FBlackboard&) = delete;
		FBlackboard& operator=(const FBlackboard&) = delete;
		
		/*====================================================================================================================*/
		// BlackboardObject
		
	private:
		class BlackboardObject
		{
			// untyped base
			struct ObjectContainerBase
			{
			};

			// typed container
			template<typename T>
			struct ObjectContainer : ObjectContainerBase
			{
				ObjectContainer(const T& t) : Object(t) {}

				const T& Get() const { return Object; }
				T& Get() { return Object; }

			private:
				T Object;
			};


		public:
			template<typename T>
			BlackboardObject(const T& obj) : ContainerPtr(MakeShared<ObjectContainer<T>>(obj)) {}

			template<typename T>
			const T& Get() const
			{
				const ObjectContainer<T>* TypedContainer = static_cast<ObjectContainer<T>*>(ContainerPtr.Get());
				return TypedContainer->Get();
			}

			template<typename T>
			T& Get()
			{
				ObjectContainer<T>* TypedContainer = static_cast<ObjectContainer<T>*>(ContainerPtr.Get());
				return TypedContainer->Get();
			}

		private:
			TSharedPtr<ObjectContainerBase> ContainerPtr;
		};

		// ~BlackboardObject
		/*====================================================================================================================*/
	 
	public:

		/** Attempt to retrieve an object from the blackboard. If found, OutFoundValue will be set.
		 * Returns true/false to indicate whether it was found. */
		template<typename T>
	   bool TryGet(FName ObjName, T& OutFoundValue) const
		{
			if (const TUniquePtr<BlackboardObject>* ExistingObject = ObjectsByName.Find(ObjName))
			{
				OutFoundValue = ExistingObject->Get()->Get<T>();
				return true;
			}

			return false;
		}

		template<typename T>
		const T* Find(FName ObjName) const
		{
			if (const TUniquePtr<BlackboardObject>* ExistingObject = ObjectsByName.Find(ObjName))
			{
				return &ExistingObject->Get()->Get<T>();
			}

			return nullptr;
		}

		template<typename T>
		T* Find(FName ObjName)
		{
			if (TUniquePtr<BlackboardObject>* ExistingObject = ObjectsByName.Find(ObjName))
			{
				return &ExistingObject->Get()->Get<T>();
			}

			return nullptr;
		}
		
		template<typename T>
		const T& GetChecked(FName ObjName) const
		{
			const TUniquePtr<BlackboardObject>* ExistingObject = ObjectsByName.Find(ObjName);
			checkf(ExistingObject, TEXT("XMoveU::FBlackboard::GetChecked >> Object not found"))
			return ExistingObject->Get()->Get<T>();
		}

		template<typename T>
		T& GetChecked(FName ObjName)
		{
			TUniquePtr<BlackboardObject>* ExistingObject = ObjectsByName.Find(ObjName);
			checkf(ExistingObject, TEXT("XMoveU::FBlackboard::GetChecked >> Object not found"))
			return ExistingObject->Get()->Get<T>();
		}

		/** Returns true/false to indicate if an object is stored with that name */
		bool Contains(FName ObjName) const
		{
			return ObjectsByName.Contains(ObjName);
		}

		/** Store object by a named key, overwriting any existing object */
		template<typename T>
		T& Set(FName ObjName, T Obj)
		{
			TUniquePtr<BlackboardObject>& ExistingObject = ObjectsByName.Emplace(ObjName, MakeUnique<BlackboardObject>(Obj));
			return ExistingObject.Get()->Get<T>();
		}

		void Clear()
		{
			ObjectsByName.Empty();
		}

	private:
		TMap<FName, TUniquePtr<BlackboardObject>> ObjectsByName;
	};
}