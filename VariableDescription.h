#pragma once
#include "DataTypes.h"
#include <Com/Com.h>

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
			VariableDescription(const std::string& libraryName, Pointer<ITypeInfo2> typeInfo, UINT index);
			VariableDescription(const VariableDescription& rhs) = delete;
			~VariableDescription();

			VariableDescription& operator=(const VariableDescription& rhs) = delete;

			std::string GetName() const;
			long GetValue() const;
			EnumValue ToEnumValue() const;
			Parameter ToParameter() const;
		};
	}
}
