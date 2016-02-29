#pragma once
#include <type_traits>

namespace Com
{
	template <typename... Interfaces>
	class DispatchInterfaceSelector
	{
	public:
		using Type = void;
	};

	template <typename Interface, typename... Interfaces>
	class DispatchInterfaceSelector<Interface, Interfaces...>
	{
	public:
		using Type = typename std::conditional<
			std::is_base_of<IDispatch, Interface>::value,
			Interface,
			typename DispatchInterfaceSelector<Interfaces...>::Type
		>::type;
	};
}
