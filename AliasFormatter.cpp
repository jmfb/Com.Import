#include "AliasFormatter.h"

namespace Com
{
	namespace Import
	{
		AliasFormatter::AliasFormatter(const Alias& value)
			: value(value)
		{
		}

		std::ostream& AliasFormatter::Write(std::ostream& out) const
		{
			return out << "	using " << value.NewName << " = " << value.OldName << ";" << std::endl;
		}

		std::ostream& operator<<(std::ostream& out, const AliasFormatter& value)
		{
			return value.Write(out);
		}

		AliasFormatter Format(const Alias& value)
		{
			return{ value };
		}
	}
}
