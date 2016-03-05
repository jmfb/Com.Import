#pragma once
#include "DataTypes.h"
#include <iostream>

namespace Com
{
	namespace Import
	{
		class EnumFormatter
		{
		private:
			const Enum& value;

		public:
			EnumFormatter(const Enum& value);

			std::ostream& Write(std::ostream& out) const;
			friend std::ostream& operator<<(std::ostream& out, const EnumFormatter& value);

		private:
			static bool ShouldDisplayAsHex(const EnumValue& member);
			static void WriteMember(std::ostream& out, const EnumValue& member);
		};

		EnumFormatter Format(const Enum& value);
	}
}
