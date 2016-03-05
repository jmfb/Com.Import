#pragma once
#include "DataTypes.h"
#include <iostream>

namespace Com
{
	namespace Import
	{
		enum class FunctionFormat
		{
			AsAbstract
		};

		class FunctionFormatter
		{
		private:
			const Function& value;
			FunctionFormat format;
			std::string prefix;

		public:
			FunctionFormatter(const Function& value, FunctionFormat format, const std::string& prefix);

			std::ostream& Write(std::ostream& out) const;
			friend std::ostream& operator<<(std::ostream& out, const FunctionFormatter& value);

		private:
			void WriteAsAbstract(std::ostream& out) const;
		};

		FunctionFormatter Format(const Function& value, FunctionFormat format, const std::string& prefix = "");
	}
}
