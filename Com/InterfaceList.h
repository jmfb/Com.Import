#pragma once
#include <objbase.h>
#include "DispatchInterfaceSelector.h"
#include <type_traits>

namespace Com
{
	template <typename... Interfaces>
	class InterfaceList
	{
	public:
		bool QueryInterfaceInternal(REFIID riid, void** ppvObject)
		{
			return false;
		}
	};

	template <typename Interface, typename... Interfaces>
	class InterfaceList<Interface, Interfaces...> : public Interface, public InterfaceList<Interfaces...>
	{
	public:
		using PrimaryInterface = Interface;
		using DispatchInterface = typename DispatchInterfaceSelector<Interface, Interfaces...>::Type;
		static constexpr bool SupportsDispatch = !std::is_void<DispatchInterface>::value;

		bool QueryInterfaceInternal(REFIID riid, void** ppvObject)
		{
			if (riid != __uuidof(Interface))
				return InterfaceList<Interfaces...>::QueryInterfaceInternal(riid, ppvObject);
			*ppvObject = static_cast<Interface*>(this);
			return true;
		}
	};
}
