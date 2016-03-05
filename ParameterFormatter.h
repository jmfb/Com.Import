#pragma once
#include "DataTypes.h"
#include <iostream>

namespace Com
{
	namespace Import
	{
		class ParameterFormatter
		{
		private:
			const Parameter& value;

		public:
			ParameterFormatter(const Parameter& value);

			std::ostream& Write(std::ostream& out) const;
			friend std::ostream& operator<<(std::ostream& out, const ParameterFormatter& value);
		};

		ParameterFormatter Format(const Parameter& value);
	}
}
