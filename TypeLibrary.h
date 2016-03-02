#pragma once
#include "Com/Com.h"
#include <string>

namespace Com
{
	namespace Import
	{
		class TypeLibrary
		{
		private:
			Pointer<ITypeLib> typeLibrary;
			TLIBATTR* attributes = nullptr;

		public:
			TypeLibrary(const std::string& fileName)
			{
				auto hr = ::LoadTypeLib(Put(fileName), &typeLibrary);
				CheckError(hr, __FUNCTION__, "LoadTypeLib");

				hr = typeLibrary->GetLibAttr(&attributes);
				CheckError(hr, __FUNCTION__, "GetLibAttr");
			}
			TypeLibrary(Pointer<ITypeLib> typeLibrary)
				: typeLibrary(typeLibrary)
			{
				auto hr = typeLibrary->GetLibAttr(&attributes);
				CheckError(hr, __FUNCTION__, "GetLibAttr");
			}
			TypeLibrary(const TypeLibrary& rhs) = delete;
			~TypeLibrary()
			{
				if (attributes != nullptr)
					typeLibrary->ReleaseTLibAttr(attributes);
			}

			TypeLibrary& operator=(const TypeLibrary& rhs) = delete;

			const GUID& GetId() const
			{
				return attributes->guid;
			}

			std::string GetName() const
			{
				std::string name;
				auto hr = typeLibrary->GetDocumentation(-1, Get(name), nullptr, nullptr, nullptr);
				CheckError(hr, __FUNCTION__, "GetDocumentation");
				return name;
			}

			UINT GetTypeInfoCount() const
			{
				return typeLibrary->GetTypeInfoCount();
			}

			Pointer<ITypeInfo> GetTypeInfo(UINT index) const
			{
				Pointer<ITypeInfo> typeInfo;
				auto hr = typeLibrary->GetTypeInfo(index, &typeInfo);
				CheckError(hr, __FUNCTION__, "GetTypeInfo");
				return typeInfo;
			}

			std::string QueryPath() const
			{
				std::string path;
				auto hr = ::QueryPathOfRegTypeLib(
					attributes->guid,
					attributes->wMajorVerNum,
					attributes->wMinorVerNum,
					attributes->lcid,
					Get(path));
				CheckError(hr, __FUNCTION__, "QueryPathOfRegTypeLib");
				return path;
			}
		};
	}
}
