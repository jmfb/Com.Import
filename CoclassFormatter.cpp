#include "CoclassFormatter.h"
#include "InterfaceFormatter.h"

namespace Com
{
	namespace Import
	{
		CoclassFormatter::CoclassFormatter(
			const Coclass& value,
			CoclassFormat format,
			const std::string& libraryName,
			const std::string& outputName)
			: value(value), format(format), libraryName(libraryName), outputName(outputName)
		{
		}

		std::ostream& CoclassFormatter::Write(std::ostream& out) const
		{
			switch (format)
			{
			case CoclassFormat::AsBase:
				WriteAsBase(out);
				break;
			case CoclassFormat::AsObjectHeader:
				WriteAsObjectHeader(out);
				break;
			case CoclassFormat::AsObjectSource:
				WriteAsObjectSource(out);
				break;
			}
			return out;
		}

		std::ostream& operator<<(std::ostream& out, const CoclassFormatter& value)
		{
			return value.Write(out);
		}

		void CoclassFormatter::WriteAsBase(std::ostream& out) const
		{
			for (auto& iface : value.Interfaces)
				if (iface.IsConflicting)
					out << Format(iface, InterfaceFormat::AsResolveNameConflict, value.Name + "_");
			out << "	template <typename Type>" << std::endl
				<< "	class " << value.Name << "Coclass : public Com::Object<Type, &CLSID_" << value.Name;
			for (auto& iface : value.Interfaces)
			{
				out << ", ";
				if (iface.IsConflicting)
					out << value.Name << "_";
				out << iface.Name;
			}
			out << ">" << std::endl
				<< "	{" << std::endl
				<< "	public:" << std::endl;
			for (auto& iface : value.Interfaces)
				out << Format(iface, InterfaceFormat::AsCoclassAbstractFunctions);
			for (auto& iface : value.Interfaces)
				out << Format(iface, InterfaceFormat::AsRawFunctions);
			out << "	};" << std::endl;
		}

		void CoclassFormatter::WriteAsObjectHeader(std::ostream& out) const
		{
			out << "#pragma once" << std::endl
				<< "#include \"" << outputName << ".h\"" << std::endl
				<< std::endl
				<< "namespace " << libraryName << std::endl
				<< "{" << std::endl
				<< "	class " << value.Name << " : public " << value.Name << "Coclass<" << value.Name << ">" << std::endl
				<< "	{" << std::endl
				<< "	public:" << std::endl;
			for (auto& iface : value.Interfaces)
				out << Format(iface, InterfaceFormat::AsCoclassFunctionPrototypes);
			out << "	};" << std::endl
				<< "}" << std::endl;
		}

		void CoclassFormatter::WriteAsObjectSource(std::ostream& out) const
		{
			out << "#include \"" << value.Name << ".h\"" << std::endl
				<< std::endl
				<< "namespace " << libraryName << std::endl
				<< "{" << std::endl;
			for (auto& iface : value.Interfaces)
				out << Format(iface, InterfaceFormat::AsCoclassFunctionImplementations, "", value.Name);
			out << "}" << std::endl;
		}

		CoclassFormatter Format(
			const Coclass& value,
			CoclassFormat format,
			const std::string& libraryName,
			const std::string& outputName)
		{
			return{ value, format, libraryName, outputName };
		}
	}
}
