#pragma once
#include "DataTypes.h"
#include <iostream>

namespace Com
{
	namespace Import
	{
		class AliasFormatter
		{
		private:
			const Alias& value;

		public:
			AliasFormatter(const Alias& value);

			std::ostream& Write(std::ostream& out) const;
			friend std::ostream& operator<<(std::ostream& out, const AliasFormatter& value);
		};

		AliasFormatter Format(const Alias& value);
	}
}
