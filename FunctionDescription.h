#pragma once
#include "DataTypes.h"
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
			FunctionDescription(const std::string& libraryName, Pointer<ITypeInfo2> typeInfo, UINT index);
			FunctionDescription(const FunctionDescription& rhs) = delete;
			~FunctionDescription();

			FunctionDescription& operator=(const FunctionDescription& rhs) = delete;

			std::string GetRootName() const;
			bool IsProperty() const;
			bool IsPropGet() const;
			bool IsPropPut() const;
			bool IsPropPutRef() const;
			std::string GetName() const;
			bool IsDispatchOnly() const;
			Type GetRetval() const;
			std::vector<Parameter> GetParameters() const;
			Parameter CreateParameter(const std::string& name, const ELEMDESC& element) const;
			Function ToFunction(bool supportsDispatch) const;

		private:
			static Parameter ConvertResultToArgument(const Type& type);
		};
	}
}
