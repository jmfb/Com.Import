#pragma once
#include "DataTypes.h"
#include <utility>
#include <vector>

namespace Com
{
	namespace Import
	{
		class RecordSorter
		{
		public:
			static void SortRecords(std::vector<Record>& records);

		private:
			static std::pair<bool, std::size_t> TryFindIndexOfLastRecordReferencingOther(std::vector<Record>& records, std::size_t index);
			static bool RecordReferencesOther(const Record& record, const Record& other);
		};
	}
}
