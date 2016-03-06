#include "FunctionFormatter.h"
#include "TypeFormatter.h"
#include "ParameterFormatter.h"

namespace Com
{
	namespace Import
	{
		FunctionFormatter::FunctionFormatter(
			const Function& value,
			FunctionFormat format,
			const std::string& prefix,
			const std::string& scope)
			: value(value), format(format), prefix(prefix), scope(scope)
		{
		}

		std::ostream& FunctionFormatter::Write(std::ostream& out) const
		{
			switch (format)
			{
			case FunctionFormat::AsAbstract:
				WriteAsAbstract(out);
				break;
			case FunctionFormat::AsResolveNameConflict:
				WriteAsResolveNameConflict(out);
				break;
			case FunctionFormat::AsWrapper:
				WriteAsWrapper(out);
				break;
			case FunctionFormat::AsWrapperImplementation:
				WriteAsWrapperImplementation(out);
				break;
			case FunctionFormat::AsWrapperDispatch:
				WriteAsWrapperDispatch(out);
				break;
			case FunctionFormat::AsRawImplementation:
				WriteAsRawImplementation(out);
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

		void FunctionFormatter::WriteAsResolveNameConflict(std::ostream& out) const
		{
			out << "		" << Format(value.Retval, TypeFormat::AsNative) << " __stdcall raw_" << value.Name << "(";
			auto first = true;
			for (auto& argument : value.ArgList)
			{
				if (!first)
					out << ", ";
				first = false;
				out << Format(argument, ParameterFormat::AsNative);
			}
			out << ") final" << std::endl
				<< "		{" << std::endl
				<< "			return " << prefix << "raw_" << value.Name << "(";
			first = true;
			for (auto& argument : value.ArgList)
			{
				if (!first)
					out << ", ";
				first = false;
				out << argument.Name;
			}
			out << ");" << std::endl
				<< "		}" << std::endl;
		}

		void FunctionFormatter::WriteAsWrapper(std::ostream& out) const
		{
			out << "		";
			if (!value.ArgList.empty() && value.ArgList.back().Retval)
				out << Format(value.ArgList.back().Type, TypeFormat::AsWrapper);
			else
				out << "void";
			out << " " << value.Name << "(";
			auto first = true;
			for (auto& argument : value.ArgList)
			{
				if (argument.Retval)
					break;
				if (!first)
					out << ", ";
				first = false;
				out << Format(argument, ParameterFormat::AsWrapper);
			}
			out << ");" << std::endl;
		}

		void FunctionFormatter::WriteAsWrapperImplementation(std::ostream& out) const
		{
			out << "	template <typename Interface>" << std::endl
				<< "	inline ";
			auto hasRetval = !value.ArgList.empty() && value.ArgList.back().Retval;
			if (hasRetval)
				out << Format(value.ArgList.back().Type, TypeFormat::AsWrapper);
			else
				out << "void";
			out << " " << scope << "PtrT<Interface>::" << value.Name << "(";
			auto first = true;
			for (auto& argument : value.ArgList)
			{
				if (argument.Retval)
					break;
				if (!first)
					out << ", ";
				first = false;
				out << Format(argument, ParameterFormat::AsWrapper);
			}
			out << ")" << std::endl
				<< "	{" << std::endl;
			if (hasRetval)
			{
				out << "		";
				out << Format(value.ArgList.back().Type, TypeFormat::AsWrapper);
				out << " retval";
				out << Format(value.ArgList.back().Type, TypeFormat::AsInitializer);
				out << ";" << std::endl;
			}
			out << "		";
			if (value.Retval.TypeEnum == TypeEnum::Hresult)
				out << "auto hr = ";
			out << "p->";
			out << prefix;
			out << value.Name << "(";
			first = true;
			for (auto& argument : value.ArgList)
			{
				if (!first)
					out << ", ";
				first = false;
				if (argument.In && argument.Out)
					out << "Com::PutRef(";
				else if (argument.In)
					out << "Com::Put(";
				else
					out << "Com::Get(";
				out << argument.Name << ")";
			}
			out << ");" << std::endl;
			if (value.Retval.TypeEnum == TypeEnum::Hresult)
				out << "		Com::CheckError(hr, __FUNCTION__, \"\");" << std::endl;
			if (hasRetval)
				out << "		return retval;" << std::endl;
			out << "	}" << std::endl;
		}

		void FunctionFormatter::WriteAsWrapperDispatch(std::ostream& out) const
		{
			//TODO: dispatch-only wrapper implementation
		}

		void FunctionFormatter::WriteAsRawImplementation(std::ostream& out) const
		{
			out << "		HRESULT __stdcall " << prefix << "raw_" << value.Name << "(";
			auto first = true;
			for (auto& argument : value.ArgList)
			{
				if (!first)
					out << ", ";
				first = false;
				out << Format(argument, ParameterFormat::AsNative);
			}
			out << ") final" << std::endl
				<< "		{" << std::endl
				<< "			try" << std::endl
				<< "			{" << std::endl
				<< "				";
			if (!value.ArgList.empty() && value.ArgList.back().Retval)
				out << Format(value.ArgList.back(), ParameterFormat::AsWrapperReturnValue) << " = ";
			out << prefix << value.Name << "(";
			first = true;
			for (auto& argument : value.ArgList)
			{
				if (argument.Retval)
					break;
				if (!first)
					out << ", ";
				first = false;
				out << Format(argument, ParameterFormat::AsWrapperArgument);
			}
			out << ");" << std::endl
				<< "			}" << std::endl
				<< "			catch (...)" << std::endl
				<< "			{" << std::endl
				<< "				return Com::HandleException();" << std::endl
				<< "			}" << std::endl
				<< "			return S_OK;" << std::endl
				<< "		}" << std::endl;
		}

		FunctionFormatter Format(
			const Function& value,
			FunctionFormat format,
			const std::string& prefix,
			const std::string& scope)
		{
			return{ value, format, prefix, scope };
		}
	}
}
