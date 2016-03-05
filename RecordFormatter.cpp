#include "RecordFormatter.h"
#include "GuidFormatter.h"
#include "TypeFormatter.h"

namespace Com
{
	namespace Import
	{
		RecordFormatter::RecordFormatter(const Record& value)
			: value(value)
		{
		}

		std::ostream& RecordFormatter::Write(std::ostream& out) const
		{
			out << "	#pragma pack(push, " << value.Alignment << ")" << std::endl
				<< "	struct " << Format(value.Guid, GuidFormat::AsAttribute) << " " << value.Name << std::endl
				<< "	{" << std::endl;
			for (auto& member : value.Members)
				out << "		" << Format(member.Type, TypeFormat::AsNative)
					<< " " << member.Name
					<< Format(member.Type, TypeFormat::AsSuffix)
					<< ";" << std::endl;
			out << "	};" << std::endl
				<< "	#pragma pack(pop)" << std::endl;
			return out;
		}

		std::ostream& operator<<(std::ostream& out, const RecordFormatter& value)
		{
			return value.Write(out);
		}

		RecordFormatter Format(const Record& value)
		{
			return{ value };
		}
	}
}
