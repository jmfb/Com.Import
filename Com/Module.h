#pragma once
#include <Windows.h>
#include <oleauto.h>
#include <atomic>

namespace Com
{
	class Module
	{
	public:
		static Module& GetInstance()
		{
			static Module instance;
			return instance;
		}

		void Initialize(HMODULE module)
		{
			this->module = module;
		}

		HRESULT LoadTypeLibrary(ITypeLib** typeLibrary)
		{
			wchar_t moduleName[MAX_PATH];
			auto result = ::GetModuleFileNameW(module, moduleName, MAX_PATH);
			if (result == 0 || result == MAX_PATH)
				return E_FAIL;
			return ::LoadTypeLib(moduleName, typeLibrary);
		}

		void AddRef()
		{
			++referenceCount;
		}

		void Release()
		{
			--referenceCount;
		}

		bool CanUnload()
		{
			return referenceCount == 0;
		}

	private:
		std::atomic<int> referenceCount = 0;
		HMODULE module = nullptr;
	};
}
