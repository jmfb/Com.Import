#pragma once
#include <atlbase.h>

namespace Com
{
	template <typename Interface>
	using Pointer = ATL::CComPtr<Interface>;

	template <typename Interface>
	using QueryPointer = ATL::CComQIPtr<Interface>;

	using String = ATL::CComBSTR;

	using Variant = ATL::CComVariant;
}
