#include "ParameterFormatter.h"
#include "TypeFormatter.h"

namespace Com
{
	namespace Import
	{
		ParameterFormatter::ParameterFormatter(const Parameter& value)
			: value(value)
		{
		}

		std::ostream& ParameterFormatter::Write(std::ostream& out) const
		{
			out << Format(value.Type, TypeFormat::AsWrapper);
			if (value.Out)
				out << "&";
			out << " " << value.Name;
			return out;
		}

		std::ostream& operator<<(std::ostream& out, const ParameterFormatter& value)
		{
			return value.Write(out);
		}

		ParameterFormatter Format(const Parameter& value)
		{
			return{ value };
		}
	}
}
