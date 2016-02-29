#pragma once
#include "DataTypes.h"
#include "TypeDescription.h"
#include "ElementDescription.h"
#include "ArgumentNames.h"
#include "Com/Com.h"

namespace Com
{
	namespace Import
	{
		class FunctionDescription
		{
		private:
			std::string libraryName;
			Pointer<ITypeInfo2> typeInfo;
			FUNCDESC* description = nullptr;

		public:
			FunctionDescription(const std::string& libraryName, Pointer<ITypeInfo2> typeInfo, UINT index)
				: libraryName(libraryName), typeInfo(typeInfo)
			{
				auto hr = typeInfo->GetFuncDesc(index, &description);
				CheckError(hr, __FUNCTION__, "GetFuncDesc");
			}
			FunctionDescription(const FunctionDescription& rhs) = delete;
			~FunctionDescription()
			{
				if (description != nullptr)
					typeInfo->ReleaseFuncDesc(description);
			}

			FunctionDescription& operator=(const FunctionDescription& rhs) = delete;

			std::string GetRootName() const
			{
				std::string name;
				auto hr = typeInfo->GetDocumentation(description->memid, Get(name), nullptr, nullptr, nullptr);
				CheckError(hr, __FUNCTION__, "GetDocumentation");
				return name;
			}

			bool IsProperty() const
			{
				return description->invkind != INVOKE_FUNC;
			}

			bool IsPropGet() const
			{
				return description->invkind == INVOKE_PROPERTYGET;
			}

			bool IsPropPut() const
			{
				return description->invkind == INVOKE_PROPERTYPUT;
			}

			bool IsPropPutRef() const
			{
				return description->invkind == INVOKE_PROPERTYPUTREF;
			}

			std::string GetName() const
			{
				if (IsPropGet())
					return "get_" + GetRootName();
				if (IsPropPut())
					return "put_" + GetRootName();
				if (IsPropPutRef())
					return "putref_" + GetRootName();
				return GetRootName();
			}

			bool IsDispatchOnly() const
			{
				return GetName() != "QueryInterface" &&
					description->oVft == 0 &&
					description->funckind == FUNC_DISPATCH;
			}

			Type GetRetval() const
			{
				return TypeDescription{ libraryName, typeInfo, description->elemdescFunc.tdesc }.ToType();
			}

			std::vector<Parameter> GetParameters() const
			{
				std::vector<Parameter> result;
				UINT argumentCount = description->cParams;
				ArgumentNames names{ typeInfo, description->memid, argumentCount };
				for (auto index = 0u; index < argumentCount; ++index)
					result.push_back(CreateParameter(names[index], description->lprgelemdescParam[index]));
				return result;
			}

			Parameter CreateParameter(const std::string& name, const ELEMDESC& element) const
			{
				return ElementDescription{ libraryName, typeInfo, name, element }.ToParameter();
			}

			Function ToFunction(bool supportsDispatch) const
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

		private:
			static Parameter ConvertResultToArgument(const Type& type)
			{
				Parameter argument{ "retval", type, false, true, true };
				++argument.Type.Indirection;
				return argument;
			}
		};
	}
}
