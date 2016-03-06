#pragma once
#include "DataTypes.h"
#include <iostream>

namespace Com
{
	namespace Import
	{
		enum class ParameterFormat
		{
			AsName,
			AsNative,
			AsWrapper,
			AsWrapperArgument,
			AsCoclassReturnValue,
			AsCoclassArgument
		};

		class ParameterFormatter
		{
		private:
			const Parameter& value;
			ParameterFormat format;

		public:
			ParameterFormatter(const Parameter& value, ParameterFormat format);

			std::ostream& Write(std::ostream& out) const;
			friend std::ostream& operator<<(std::ostream& out, const ParameterFormatter& value);

		private:
			void WriteAsName(std::ostream& out) const;
			void WriteAsNative(std::ostream& out) const;
			void WriteAsWrapper(std::ostream& out) const;
			void WriteAsWrapperArgument(std::ostream& out) const;
			void WriteAsCoclassReturnValue(std::ostream& out) const;
			void WriteAsCoclassArgument(std::ostream& out) const;
		};

		ParameterFormatter Format(const Parameter& value, ParameterFormat format);
	}
}
