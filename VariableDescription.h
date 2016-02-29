#pragma once
#include "DataTypes.h"
#include "TypeDescription.h"
#include "ElementDescription.h"
#include "Com/Com.h"

namespace Com
{
	namespace Import
	{
		class VariableDescription
		{
		private:
			std::string libraryName;
			Pointer<ITypeInfo2> typeInfo;
			VARDESC* description = nullptr;

		public:
			VariableDescription(const std::string& libraryName, Pointer<ITypeInfo2> typeInfo, UINT index)
				: libraryName(libraryName), typeInfo(typeInfo)
			{
				auto hr = typeInfo->GetVarDesc(index, &description);
				CheckError(hr, __FUNCTION__, "GetVarDesc");
			}
			VariableDescription(const VariableDescription& rhs) = delete;
			~VariableDescription()
			{
				if (description != nullptr)
					typeInfo->ReleaseVarDesc(description);
			}

			VariableDescription& operator=(const VariableDescription& rhs) = delete;

			std::string GetName() const
			{
				std::string name;
				UINT count = 0;
				auto hr = typeInfo->GetNames(description->memid, Get(name), 1, &count);
				CheckError(hr, __FUNCTION__, "GetNames");
				if (count != 1)
					throw std::runtime_error(__FUNCTION__ ": GetNames returned no name.");
				return name;
			}

			bool IsConstant() const
			{
				return description->varkind == VAR_CONST;
			}

			long GetValue() const
			{
				Variant value;
				auto hr = value.ChangeType(VT_I4, description->lpvarValue);
				CheckError(hr, __FUNCTION__, "ChangeType");
				return value.lVal;
			}

			EnumValue ToEnumValue() const
			{
				if (!IsConstant())
					throw std::runtime_error("Enum member was not a constant.");
				return{ GetName(), GetValue() };
			}

			Parameter ToParameter() const
			{
				return ElementDescription{ libraryName, typeInfo, GetName(), description->elemdescVar }.ToParameter();
			}

			//Type ToType() const
			//{
			//	return TypeDescription{ libraryName, typeInfo, description->elemdescVar.tdesc }.ToType();
			//}
		};
	}
}
