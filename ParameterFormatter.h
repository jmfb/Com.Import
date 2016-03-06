#pragma once
#include "DataTypes.h"
#include <iostream>

namespace Com
{
	namespace Import
	{
		enum class ParameterFormat
		{
			AsNative,
			AsWrapper,
			AsWrapperReturnValue,
			AsWrapperArgument
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
			void WriteAsNative(std::ostream& out) const;
			void WriteAsWrapper(std::ostream& out) const;
			void WriteAsWrapperReturnValue(std::ostream& out) const;
			void WriteAsWrapperArgument(std::ostream& out) const;
		};

		ParameterFormatter Format(const Parameter& value, ParameterFormat format);
	}
}
