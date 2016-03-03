#include "FunctionDescription.h"
#include "TypeDescription.h"
#include "ElementDescription.h"
#include "ArgumentNames.h"

namespace Com
{
	namespace Import
	{
		FunctionDescription::FunctionDescription(
			const std::string& libraryName,
			Pointer<ITypeInfo2> typeInfo,
			UINT index)
			: libraryName(libraryName),
			typeInfo(typeInfo)
		{
			auto hr = typeInfo->GetFuncDesc(index, &description);
			CheckError(hr, __FUNCTION__, "GetFuncDesc");
		}

		FunctionDescription::~FunctionDescription()
		{
			if (description != nullptr)
				typeInfo->ReleaseFuncDesc(description);
		}

		std::string FunctionDescription::GetRootName() const
		{
			std::string name;
			auto hr = typeInfo->GetDocumentation(description->memid, Get(name), nullptr, nullptr, nullptr);
			CheckError(hr, __FUNCTION__, "GetDocumentation");
			return name;
		}

		bool FunctionDescription::IsProperty() const
		{
			return description->invkind != INVOKE_FUNC;
		}

		bool FunctionDescription::IsPropGet() const
		{
			return description->invkind == INVOKE_PROPERTYGET;
		}

		bool FunctionDescription::IsPropPut() const
		{
			return description->invkind == INVOKE_PROPERTYPUT;
		}

		bool FunctionDescription::IsPropPutRef() const
		{
			return description->invkind == INVOKE_PROPERTYPUTREF;
		}

		std::string FunctionDescription::GetName() const
		{
			if (IsPropGet())
				return "get_" + GetRootName();
			if (IsPropPut())
				return "put_" + GetRootName();
			if (IsPropPutRef())
				return "putref_" + GetRootName();
			return GetRootName();
		}

		bool FunctionDescription::IsDispatchOnly() const
		{
			return GetName() != "QueryInterface" &&
				description->oVft == 0 &&
				description->funckind == FUNC_DISPATCH;
		}

		Type FunctionDescription::GetRetval() const
		{
			return TypeDescription{ libraryName, typeInfo, description->elemdescFunc.tdesc }.ToType();
		}

		std::vector<Parameter> FunctionDescription::GetParameters() const
		{
			std::vector<Parameter> result;
			UINT argumentCount = description->cParams;
			ArgumentNames names{ typeInfo, description->memid, argumentCount };
			for (auto index = 0u; index < argumentCount; ++index)
				result.push_back(CreateParameter(names[index], description->lprgelemdescParam[index]));
			return result;
		}

		Parameter FunctionDescription::CreateParameter(const std::string& name, const ELEMDESC& element) const
		{
			return ElementDescription{ libraryName, typeInfo, name, element }.ToParameter();
		}

		Function FunctionDescription::ToFunction(bool supportsDispatch) const
		{
			Function value
			{
				static_cast<unsigned long>(description->oVft),
				IsDispatchOnly(),
				description->memid,
				GetName(),
				GetRetval(),
				GetParameters(),
				GetRootName(),
				IsProperty(),
				IsPropGet(),
				IsPropPut(),
				IsPropPutRef()
			};
			if (!supportsDispatch)
				return value;
			if (value.Retval.TypeEnum != TypeEnum::Void)
				value.ArgList.push_back(ConvertResultToArgument(value.Retval));
			value.Retval = { 0, TypeEnum::Hresult, "", false, 0 };
			return value;
		}

		Parameter FunctionDescription::ConvertResultToArgument(const Type& type)
		{
			Parameter argument{ "retval", type, false, true, true };
			++argument.Type.Indirection;
			return argument;
		}
	}
}
