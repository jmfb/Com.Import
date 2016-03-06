#include "InterfaceFormatter.h"
#include "GuidFormatter.h"
#include "FunctionFormatter.h"

namespace Com
{
	namespace Import
	{
		InterfaceFormatter::InterfaceFormatter(
			const Interface& value,
			InterfaceFormat format,
			const std::string& prefix,
			const std::string& scope)
			: value(value), format(format), prefix(prefix), scope(scope)
		{
		}

		std::ostream& InterfaceFormatter::Write(std::ostream& out) const
		{
			switch (format)
			{
			case InterfaceFormat::AsForwardDeclaration:
				WriteAsForwardDeclaration(out);
				break;
			case InterfaceFormat::AsWrapperForwardDeclaration:
				WriteAsWrapperForwardDeclaration(out);
				break;
			case InterfaceFormat::AsNative:
				WriteAsNative(out);
				break;
			case InterfaceFormat::AsResolveNameConflict:
				WriteAsResolveNameConflict(out);
				break;
			case InterfaceFormat::AsWrapper:
				WriteAsWrapper(out);
				break;
			case InterfaceFormat::AsWrapperFunctions:
				WriteAsWrapperFunctions(out);
				break;
			case InterfaceFormat::AsRawFunctions:
				WriteAsRawFunctions(out);
				break;
			case InterfaceFormat::AsTypeInfoSpecialization:
				WriteAsTypeInfoSpecialization(out);
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

		void InterfaceFormatter::WriteAsWrapperForwardDeclaration(std::ostream& out) const
		{
			out << "	template <typename Interface> class " << value.Name << "PtrT;" << std::endl
				<< "	using " << value.Name << "Ptr = " << value.Name << "PtrT<" << value.Name << ">;" << std::endl;
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

		void InterfaceFormatter::WriteAsResolveNameConflict(std::ostream& out) const
		{
			out << "	class " << Format(value.Iid, GuidFormat::AsAttribute) << " " << prefix << value.Name << " : public " << value.Name << std::endl
				<< "	{" << std::endl
				<< "	public:" << std::endl;
			for (auto& function : value.Functions)
				if ((function.VtblOffset == 0 && function.IsDispatchOnly) || (function.VtblOffset >= value.VtblOffset))
					out << Format(function, FunctionFormat::AsAbstract, value.Name + "_raw_");
			for (auto& function : value.Functions)
				if ((function.VtblOffset == 0 && function.IsDispatchOnly) || (function.VtblOffset >= value.VtblOffset))
					out << Format(function, FunctionFormat::AsResolveNameConflict, value.Name + "_");
			out << "	};" << std::endl;
		}

		void InterfaceFormatter::WriteAsWrapper(std::ostream& out) const
		{
			out << "	template <typename Interface>" << std::endl
				<< "	class " << value.Name << "PtrT : public " << GetWrapperBase() << std::endl
				<< "	{" << std::endl
				<< "	public:" << std::endl
				<< "		" << value.Name << "PtrT(Interface* value = nullptr);" << std::endl
				<< "		" << value.Name << "PtrT<Interface>& operator=(Interface* value);" << std::endl
				<< "		operator " << value.Name << "*() const;" << std::endl;
			for (auto& function : value.Functions)
				if ((function.VtblOffset == 0 && function.IsDispatchOnly) || (function.VtblOffset >= value.VtblOffset))
					out << Format(function, FunctionFormat::AsWrapper);
			out << "	};" << std::endl;
		}

		void InterfaceFormatter::WriteAsWrapperFunctions(std::ostream& out) const
		{
			out << "	template <typename Interface>" << std::endl
				<< "	inline " << value.Name << "PtrT<Interface>::" << value.Name << "PtrT(Interface* value) : " << GetWrapperBase() << "(value)" << std::endl
				<< "	{" << std::endl
				<< "	}" << std::endl
				<< "	template <typename Interface>" << std::endl
				<< "	inline " << value.Name << "PtrT<Interface>& " << value.Name << "PtrT<Interface>::operator=(Interface* value)" << std::endl
				<< "	{" << std::endl
				<< "		using Base = " << GetWrapperBase() << ";" << std::endl
				<< "		Base::operator=(value);" << std::endl
				<< "		return *this;" << std::endl
				<< "	}" << std::endl
				<< "	template <typename Interface>" << std::endl
				<< "	inline " << value.Name << "PtrT<Interface>::operator " << value.Name << "*() const" << std::endl
				<< "	{" << std::endl
				<< "		return p;" << std::endl
				<< "	}" << std::endl;
			for (auto& function : value.Functions)
			{
				if (function.VtblOffset == 0 && function.IsDispatchOnly)
					out << Format(function, FunctionFormat::AsWrapperDispatch, "", value.Name);
				else if (function.VtblOffset >= value.VtblOffset)
					out << Format(function, FunctionFormat::AsWrapperImplementation, prefix, value.Name);
			}
		}

		void InterfaceFormatter::WriteAsRawFunctions(std::ostream& out) const
		{
			for (auto& function : value.Functions)
				if (function.VtblOffset >= value.VtblOffset && function.Retval.TypeEnum == TypeEnum::Hresult)
					out << Format(function, FunctionFormat::AsRawImplementation, value.IsConflicting ? value.Name + "_" : "");
		}

		void InterfaceFormatter::WriteAsTypeInfoSpecialization(std::ostream& out) const
		{
			auto interfaceName = scope + "::" + value.Name;
			out << "	template <>" << std::endl
				<< "	class TypeInfo<" << interfaceName << "*>" << std::endl
				<< "	{" << std::endl
				<< "	public:" << std::endl
				<< "		using In = InValue<" << interfaceName << "*, " << interfaceName << "Ptr>;" << std::endl
				<< "		using InOut = InOutValue<" << interfaceName << "*, " << interfaceName << "Ptr>;" << std::endl
				<< "		using Retval = RetvalValue<" << interfaceName << "Ptr, " << interfaceName << "*>;" << std::endl
				<< "	};" << std::endl;
		}

		std::string InterfaceFormatter::GetWrapperBase() const
		{
			if (value.Base == "IUnknown" || value.Base == "IDispatch")
				return "Com::Pointer<Interface>";
			return value.Base + "PtrT<Interface>";
		}

		InterfaceFormatter Format(
			const Interface& value,
			InterfaceFormat format,
			const std::string& prefix,
			const std::string& scope)
		{
			return{ value, format, prefix, scope };
		}
	}
}
