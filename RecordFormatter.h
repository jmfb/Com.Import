#pragma once
#include "DataTypes.h"
#include <iostream>

namespace Com
{
	namespace Import
	{
		class RecordFormatter
		{
		private:
			const Record& value;

		public:
			RecordFormatter(const Record& value);

			std::ostream& Write(std::ostream& out) const;
			friend std::ostream& operator<<(std::ostream& out, const RecordFormatter& value);
		};

		RecordFormatter Format(const Record& value);
	}
}
