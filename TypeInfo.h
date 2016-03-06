#pragma once
#include "DataTypes.h"
#include <Com/Com.h>
#include <string>

namespace Com
{
	namespace Import
	{
		class TypeInfo
		{
		private:
			std::string libraryName;
			Pointer<ITypeInfo2> typeInfo;
			TYPEATTR* attributes = nullptr;

		public:
			TypeInfo(Pointer<ITypeInfo> value);
			TypeInfo(const TypeInfo& rhs) = delete;
			~TypeInfo();

			TypeInfo& operator=(const TypeInfo& rhs) = delete;

			const GUID& GetId() const;
			std::string GetName() const;
			const std::string& GetLibraryName() const;
			Pointer<ITypeLib> GetLibrary() const;
			TYPEKIND GetTypeKind() const;
			Enum ToEnum() const;
			Alias ToAlias() const;
			Coclass ToCoclass() const;
			Interface GetInterface(UINT index) const;
			std::string GetInterfaceName(UINT index) const;
			Record ToRecord() const;
			Interface ToInterface() const;

		private:
			void TryUpdateBaseInterface(Interface& value) const;
			static bool FunctionIsLessThan(const Function& lhs, const Function& rhs);
		};
	}
}
