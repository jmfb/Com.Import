#include "TypeLibrary.h"

namespace Com
{
	namespace Import
	{
		TypeLibrary::TypeLibrary(const std::string& fileName)
		{
			auto hr = ::LoadTypeLib(Put(fileName), &typeLibrary);
			CheckError(hr, __FUNCTION__, "LoadTypeLib");

			hr = typeLibrary->GetLibAttr(&attributes);
			CheckError(hr, __FUNCTION__, "GetLibAttr");
		}
		TypeLibrary::TypeLibrary(Pointer<ITypeLib> typeLibrary)
			: typeLibrary(typeLibrary)
		{
			auto hr = typeLibrary->GetLibAttr(&attributes);
			CheckError(hr, __FUNCTION__, "GetLibAttr");
		}
		TypeLibrary::~TypeLibrary()
		{
			if (attributes != nullptr)
				typeLibrary->ReleaseTLibAttr(attributes);
		}

		const GUID& TypeLibrary::GetId() const
		{
			return attributes->guid;
		}

		WORD TypeLibrary::GetMajorVersion() const
		{
			return attributes->wMajorVerNum;
		}

		WORD TypeLibrary::GetMinorVersion() const
		{
			return attributes->wMinorVerNum;
		}

		std::string TypeLibrary::GetName() const
		{
			std::string name;
			auto hr = typeLibrary->GetDocumentation(-1, Get(name), nullptr, nullptr, nullptr);
			CheckError(hr, __FUNCTION__, "GetDocumentation");
			return name;
		}

		UINT TypeLibrary::GetTypeInfoCount() const
		{
			return typeLibrary->GetTypeInfoCount();
		}

		Pointer<ITypeInfo> TypeLibrary::GetTypeInfo(UINT index) const
		{
			Pointer<ITypeInfo> typeInfo;
			auto hr = typeLibrary->GetTypeInfo(index, &typeInfo);
			CheckError(hr, __FUNCTION__, "GetTypeInfo");
			return typeInfo;
		}

		std::string TypeLibrary::QueryPath() const
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
	}
}
