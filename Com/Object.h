#pragma once
#include <objbase.h>
#include <atomic>
#include "Module.h"
#include "Dispatch.h"
#include "InterfaceList.h"

namespace Com
{
	template <typename Type, const CLSID* Clsid, typename... Interfaces>
	class Object : public Dispatch<InterfaceList<Interfaces...>>, public ISupportErrorInfo
	{
	protected:
		std::atomic<int> referenceCount = 0;

	public:
		Object()
		{
			Module::GetInstance().AddRef();
		}
		Object(const Object<Type, Clsid, Interfaces...>& rhs) = delete;
		virtual ~Object()
		{
			Module::GetInstance().Release();
		}

		Object<Type, Clsid, Interfaces...>& operator=(const Object<Type, Clsid, Interfaces...>& rhs) = delete;

		HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) final
		{
			if (ppvObject == nullptr)
				return E_POINTER;
			if (riid == IID_IUnknown)
				*ppvObject = static_cast<IUnknown*>(static_cast<PrimaryInterface*>(this));
			else if (SupportsDispatch && riid == IID_IDispatch)
				*ppvObject = GetDispatchInterface();
			else if (riid == IID_ISupportErrorInfo)
				*ppvObject = static_cast<ISupportErrorInfo*>(this);
			else if (!QueryInterfaceInternal(riid, ppvObject))
				return E_NOINTERFACE;
			++referenceCount;
			return S_OK;
		}

		ULONG __stdcall AddRef() final
		{
			return ++referenceCount;
		}

		ULONG __stdcall Release() final
		{
			auto result = --referenceCount;
			if (result == 0)
			{
				OnFinalRelease();
				delete this;
			}
			return result;
		}

		HRESULT __stdcall InterfaceSupportsErrorInfo(REFIID riid)
		{
			return S_OK;
		}

		virtual HRESULT OnFinalConstruct()
		{
			return S_OK;
		}

		virtual void OnFinalRelease()
		{
		}

		static HRESULT TryCreateInstance(REFIID riid, void** ppvObject)
		{
			if (ppvObject == nullptr)
				return E_POINTER;
			auto object = new Type();
			auto hr = object->OnFinalConstruct();
			if (SUCCEEDED(hr))
				hr = object->QueryInterface(riid, ppvObject);
			if (FAILED(hr))
				delete object;
			return hr;
		}

		static bool Is(REFCLSID rclsid)
		{
			return Clsid != nullptr && rclsid == *Clsid;
		}
	};
}
