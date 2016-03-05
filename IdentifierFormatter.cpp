#include "IdentifierFormatter.h"
#include "GuidFormatter.h"

namespace Com
{
	namespace Import
	{
		IdentifierFormatter::IdentifierFormatter(const Identifier& value)
			: value(value)
		{
		}

		std::ostream& IdentifierFormatter::Write(std::ostream& out) const
		{
			return out << "	extern const ::GUID __declspec(selectany) " << value.Name
				<< " = " << Format(value.Guid, GuidFormat::AsInitializer) << ";" << std::endl;
		}

		std::ostream& operator<<(std::ostream& out, const IdentifierFormatter& value)
		{
			return value.Write(out);
		}

		IdentifierFormatter Format(const Identifier& value)
		{
			return{ value };
		}
	}
}
