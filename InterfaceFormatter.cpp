#include "InterfaceFormatter.h"
#include "GuidFormatter.h"
#include "FunctionFormatter.h"

namespace Com
{
	namespace Import
	{
		InterfaceFormatter::InterfaceFormatter(const Interface& value, InterfaceFormat format, const std::string& prefix)
			: value(value), format(format), prefix(prefix)
		{
		}

		std::ostream& InterfaceFormatter::Write(std::ostream& out) const
		{
			switch (format)
			{
			case InterfaceFormat::AsForwardDeclaration:
				WriteAsForwardDeclaration(out);
				break;
			case InterfaceFormat::AsNative:
				WriteAsNative(out);
				break;
			}
			return out;
		}

		std::ostream& operator<<(std::ostream& out, const InterfaceFormatter& value)
		{
			return value.Write(out);
		}

		void InterfaceFormatter::WriteAsForwardDeclaration(std::ostream& out) const
		{
			out << "	class " << Format(value.Iid, GuidFormat::AsAttribute)
				<< " " << value.Name << ";" << std::endl;
		}

		void InterfaceFormatter::WriteAsNative(std::ostream& out) const
		{
			out << "	class " << Format(value.Iid, GuidFormat::AsAttribute) << " " << value.Name << " : public " << value.Base << std::endl
				<< "	{" << std::endl
				<< "	public:" << std::endl;
			auto nextPlaceholderId = 1;
			auto nextValidOffset = value.VtblOffset;
			for (auto& function : value.Functions)
			{
				if (function.VtblOffset == 0 && function.IsDispatchOnly)
				{
					out << Format(function, FunctionFormat::AsAbstract, prefix);
					continue;
				}

				if (function.VtblOffset < nextValidOffset)
					continue;

				while (nextValidOffset < function.VtblOffset)
				{
					out << "		virtual HRESULT __stdcall _VtblGapPlaceholder" << nextPlaceholderId << "() { return E_NOTIMPL; }" << std::endl;
					++nextPlaceholderId;
					nextValidOffset += 4;
				}

				out << Format(function, FunctionFormat::AsAbstract, prefix);
				nextValidOffset += 4;
			}
			out << "	};" << std::endl;
		}

		InterfaceFormatter Format(const Interface& value, InterfaceFormat format, const std::string& prefix)
		{
			return{ value, format, prefix };
		}
	}
}