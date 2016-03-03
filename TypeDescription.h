#pragma once
#include "DataTypes.h"
#include "Com/Com.h"

namespace Com
{
	namespace Import
	{
		class TypeDescription
		{
		private:
			std::string libraryName;
			Pointer<ITypeInfo2> typeInfo;
			const TYPEDESC& description;

		public:
			TypeDescription(const std::string& libraryName, Pointer<ITypeInfo2> typeInfo, const TYPEDESC& description);
			TypeDescription(const TypeDescription& rhs) = delete;
			~TypeDescription() = default;

			TypeDescription& operator=(const TypeDescription& rhs) = delete;

			Type ToType() const;

		private:
			Type DetermineType(const TYPEDESC& typeDescription, int indirection) const;
			Type ToArrayType(const ARRAYDESC& arrayDescription, int indirection) const;
			static TypeEnum ToArrayTypeEnum(VARTYPE vt);
			static TypeEnum ToBasicTypeEnum(VARTYPE vt);
			Type ToUserDefinedType(HREFTYPE handle, int indirection) const;
			std::string GetUserDefinedTypeName(Pointer<ITypeInfo> customType) const;
			static TYPEKIND GetTypeKind(Pointer<ITypeInfo> customType);
			std::string GetDefaultInterfaceName(Pointer<ITypeInfo> customType) const;
		};
	}
}
