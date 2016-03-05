#pragma once
#include "DataTypes.h"
#include <iostream>

namespace Com
{
	namespace Import
	{
		class IdentifierFormatter
		{
		private:
			const Identifier& value;

		public:
			IdentifierFormatter(const Identifier& value);

			std::ostream& Write(std::ostream& out) const;
			friend std::ostream& operator<<(std::ostream& out, const IdentifierFormatter& value);
		};

		IdentifierFormatter Format(const Identifier& value);
	}
}
