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
			TypeLibrary(const std::string& fileName);
			TypeLibrary(Pointer<ITypeLib> typeLibrary);
			TypeLibrary(const TypeLibrary& rhs) = delete;
			~TypeLibrary();

			TypeLibrary& operator=(const TypeLibrary& rhs) = delete;

			const GUID& GetId() const;
			std::string GetName() const;
			UINT GetTypeInfoCount() const;
			Pointer<ITypeInfo> GetTypeInfo(UINT index) const;
			std::string QueryPath() const;
		};
	}
}
