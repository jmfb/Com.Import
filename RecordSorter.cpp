#include "RecordSorter.h"
#include <algorithm>

namespace Com
{
	namespace Import
	{
		void RecordSorter::SortRecords(std::vector<Record>& records)
		{
			auto remainingIterations = records.size() * records.size();
			for (auto index = 0ul; remainingIterations > 0 && index < records.size(); --remainingIterations)
			{
				auto result = TryFindIndexOfLastRecordReferencingOther(records, index);
				if (!result.first)
					++index;
				else
					std::rotate(
						records.begin() + index,
						records.begin() + index + 1,
						records.begin() + result.second + 1);
			}
			std::reverse(records.begin(), records.end());
		}

		std::pair<bool, std::size_t> RecordSorter::TryFindIndexOfLastRecordReferencingOther(std::vector<Record>& records, std::size_t index)
		{
			auto end = records.rend() - index - 1;
			auto iter = std::find_if(
				records.rbegin(),
				end,
				[&](auto& r) { return RecordReferencesOther(r, records[index]); });
			if (iter == end)
				return{ false, 0 };
			return{ true, records.size() - (iter - records.rbegin()) - 1 };
		}

		bool RecordSorter::RecordReferencesOther(const Record& record, const Record& other)
		{
			return std::any_of(
				record.Members.begin(),
				record.Members.end(),
				[&](auto& m){ return other.Name == m.Type.CustomName; });
		}
	}
}
