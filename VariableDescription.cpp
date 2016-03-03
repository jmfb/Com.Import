#include "VariableDescription.h"
#include "TypeDescription.h"
#include "ElementDescription.h"

namespace Com
{
	namespace Import
	{
		VariableDescription::VariableDescription(const std::string& libraryName, Pointer<ITypeInfo2> typeInfo, UINT index)
			: libraryName(libraryName), typeInfo(typeInfo)
		{
			auto hr = typeInfo->GetVarDesc(index, &description);
			CheckError(hr, __FUNCTION__, "GetVarDesc");
		}
		
		VariableDescription::~VariableDescription()
		{
			if (description != nullptr)
				typeInfo->ReleaseVarDesc(description);
		}

		std::string VariableDescription::GetName() const
		{
			std::string name;
			UINT count = 0;
			auto hr = typeInfo->GetNames(description->memid, Get(name), 1, &count);
			CheckError(hr, __FUNCTION__, "GetNames");
			if (count != 1)
				throw std::runtime_error(__FUNCTION__ ": GetNames returned no name.");
			return name;
		}

		long VariableDescription::GetValue() const
		{
			Variant value;
			auto hr = value.ChangeType(VT_I4, description->lpvarValue);
			CheckError(hr, __FUNCTION__, "ChangeType");
			return value.lVal;
		}

		EnumValue VariableDescription::ToEnumValue() const
		{
			if (description->varkind != VAR_CONST)
				throw std::runtime_error("Enum member was not a constant.");
			return{ GetName(), GetValue() };
		}

		Parameter VariableDescription::ToParameter() const
		{
			return ElementDescription{ libraryName, typeInfo, GetName(), description->elemdescVar }.ToParameter();
		}
	}
}
