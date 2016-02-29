#pragma once
#include <objbase.h>
#include "Object.h"
#include "Module.h"

namespace Com
{
	template <typename Type>
	class ClassFactory : public Object<ClassFactory<Type>, nullptr, IClassFactory>
	{
	public:
		HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) final
		{
			if (pUnkOuter != nullptr)
				return CLASS_E_NOAGGREGATION;
			return Type::TryCreateInstance(riid, ppvObject);
		}

		HRESULT __stdcall LockServer(BOOL fLock) final
		{
			if (fLock)
				Module::GetInstance().AddRef();
			else
				Module::GetInstance().Release();
			return S_OK;
		}
	};
}
