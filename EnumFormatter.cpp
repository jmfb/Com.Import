#include "EnumFormatter.h"
#include "HexFormatter.h"
#include <algorithm>

namespace Com
{
	namespace Import
	{
		EnumFormatter::EnumFormatter(const Enum& value)
			: value(value)
		{
		}

		std::ostream& EnumFormatter::Write(std::ostream& out) const
		{
			out << "	enum class " << value.Name;
			if (std::any_of(value.Values.begin(), value.Values.end(), &ShouldDisplayAsHex))
				out << " : unsigned";
			out << std::endl
				<< "	{" << std::endl;
			auto first = true;
			for (auto& member : value.Values)
			{
				if (!first)
					out << "," << std::endl;
				first = false;
				WriteMember(out, member);
			}
			out << std::endl
				<< "	};" << std::endl;
			return out;
		}

		std::ostream& operator<<(std::ostream& out, const EnumFormatter& value)
		{
			return value.Write(out);
		}

		bool EnumFormatter::ShouldDisplayAsHex(const EnumValue& member)
		{
			return (member.Value & 0xf0000000) == 0x80000000;
		}

		void EnumFormatter::WriteMember(std::ostream& out, const EnumValue& member)
		{
			out << "		" << member.Name << " = ";
			if (ShouldDisplayAsHex(member))
				out << Hex(member.Value);
			else
				out << member.Value;
		}

		EnumFormatter Format(const Enum& value)
		{
			return{ value };
		}
	}
}
