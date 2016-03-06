#include "LibraryFormatter.h"
#include "EnumFormatter.h"
#include "InterfaceFormatter.h"
#include "AliasFormatter.h"
#include "RecordFormatter.h"
#include "IdentifierFormatter.h"
#include "CoclassFormatter.h"

namespace Com
{
	namespace Import
	{
		LibraryFormatter::LibraryFormatter(const Library& value, LibraryFormat format, bool implement)
			: value(value), format(format), implement(implement)
		{
		}

		std::ostream& LibraryFormatter::Write(std::ostream& out) const
		{
			switch (format)
			{
			case LibraryFormat::AsImport:
				WriteAsImport(out);
				break;
			}
			return out;
		}

		std::ostream& operator<<(std::ostream& out, const LibraryFormatter& value)
		{
			return value.Write(out);
		}

		void LibraryFormatter::WriteAsImport(std::ostream& out) const
		{
			out << "#pragma once" << std::endl
				<< "#include \"Com/Com.h\"" << std::endl;
			for (auto& reference : value.References)
				out << "#include \"" << reference << "\"" << std::endl;
			out << "#pragma pack(push, 8)" << std::endl
				<< "namespace " << value.Name << std::endl
				<< "{" << std::endl;
			for (auto& enumeration : value.Enums)
				out << Format(enumeration);
			for (auto& iface : value.Interfaces)
				out << Format(iface, InterfaceFormat::AsForwardDeclaration)
					<< Format(iface, InterfaceFormat::AsWrapperForwardDeclaration);
			for (auto& alias : value.Aliases)
				out << Format(alias);
			for (auto& record : value.Records)
				out << Format(record);
			for (auto& iface : value.Interfaces)
				out << Format(iface, InterfaceFormat::AsNative, implement ? "raw_" : "");
			for (auto& identifier : value.Identifiers)
				out << Format(identifier);
			for (auto& iface : value.Interfaces)
				out << Format(iface, InterfaceFormat::AsWrapper);
			for (auto& iface : value.Interfaces)
				out << Format(iface, InterfaceFormat::AsWrapperFunctions, implement ? "raw_" : "");
			if (implement)
				for (auto& coclass : value.Coclasses)
					out << Format(coclass, CoclassFormat::AsBase);
			out << "}" << std::endl;
			out << "namespace Com" << std::endl
				<< "{" << std::endl;
			for (auto& iface : value.Interfaces)
				out << Format(iface, InterfaceFormat::AsTypeInfoSpecialization, "", value.Name);
			out << "}" << std::endl;
			out << "#pragma pack(pop)" << std::endl;
		}

		LibraryFormatter Format(const Library& library, LibraryFormat format, bool implement)
		{
			return{ library, format, implement };
		}
	}
}
