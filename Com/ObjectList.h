#pragma once
#include <objbase.h>
#include "ClassFactory.h"

namespace Com
{
	template <typename... Types>
	class ObjectList
	{
	public:
		static HRESULT Create(REFCLSID rclsid, REFIID riid, void** ppv)
		{
			return CLASS_E_CLASSNOTAVAILABLE;
		}
	};

	template <typename Type, typename... Types>
	class ObjectList<Type, Types...>
	{
	public:
		static HRESULT Create(REFCLSID rclsid, REFIID riid, void** ppv)
		{
			return Type::Is(rclsid) ?
				ClassFactory<Type>::TryCreateInstance(riid, ppv) :
				ObjectList<Types...>::Create(rclsid, riid, ppv);
		}
	};
}
