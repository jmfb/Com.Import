#include "FunctionFormatter.h"
#include "TypeFormatter.h"

namespace Com
{
	namespace Import
	{
		FunctionFormatter::FunctionFormatter(const Function& value, FunctionFormat format, const std::string& prefix)
			: value(value), format(format), prefix(prefix)
		{
		}

		std::ostream& FunctionFormatter::Write(std::ostream& out) const
		{
			switch (format)
			{
			case FunctionFormat::AsAbstract:
				WriteAsAbstract(out);
				break;
			}
			return out;
		}

		std::ostream& operator<<(std::ostream& out, const FunctionFormatter& value)
		{
			return value.Write(out);
		}

		void FunctionFormatter::WriteAsAbstract(std::ostream& out) const
		{
			out << "		";
			if (value.IsDispatchOnly)
				out << "//";
			out << "virtual " << Format(value.Retval, TypeFormat::AsNative)
				<< " __stdcall " << prefix << value.Name << "(";
			auto first = true;
			for (auto& argument : value.ArgList)
			{
				if (!first)
					out << ", ";
				first = false;
				out << Format(argument.Type, TypeFormat::AsNative);
				out << " " << argument.Name;
			}
			out << ") = 0;" << std::endl;
		}

		FunctionFormatter Format(const Function& value, FunctionFormat format, const std::string& prefix)
		{
			return{ value, format, prefix };
		}
	}
}
