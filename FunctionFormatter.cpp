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
			case FunctionFormat::AsCoclassAbstract:
				WriteAsCoclassAbstract(out);
				break;
			case FunctionFormat::AsCoclassPrototype:
				WriteAsCoclassPrototype(out);
				break;
			case FunctionFormat::AsCoclassImplementation:
				WriteAsCoclassImplementation(out);
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
			WriteArguments(out, ParameterFormat::AsNative, false);
			out << ") = 0;" << std::endl;
		}

		void FunctionFormatter::WriteAsResolveNameConflict(std::ostream& out) const
		{
			out << "		" << Format(value.Retval, TypeFormat::AsNative) << " __stdcall raw_" << value.Name << "(";
			WriteArguments(out, ParameterFormat::AsNative, false);
			out << ") final" << std::endl
				<< "		{" << std::endl
				<< "			return " << prefix << "raw_" << value.Name << "(";
			WriteArguments(out, ParameterFormat::AsName, false);
			out << ");" << std::endl
				<< "		}" << std::endl;
		}

		void FunctionFormatter::WriteAsWrapper(std::ostream& out) const
		{
			out << "		";
			if (HasRetval())
				out << Format(GetRetval().Type, TypeFormat::AsWrapper);
			else
				out << "void";
			out << " " << value.Name << "(";
			WriteArguments(out, ParameterFormat::AsWrapper, true);
			out << ");" << std::endl;
		}

		void FunctionFormatter::WriteAsWrapperImplementation(std::ostream& out) const
		{
			out << "	template <typename Interface>" << std::endl
				<< "	inline ";
			if (HasRetval())
				out << Format(GetRetval().Type, TypeFormat::AsWrapper);
			else
				out << "void";
			out << " " << scope << "PtrT<Interface>::" << value.Name << "(";
			WriteArguments(out, ParameterFormat::AsWrapper, true);
			out << ")" << std::endl
				<< "	{" << std::endl;
			if (HasRetval())
			{
				out << "		";
				out << Format(GetRetval().Type, TypeFormat::AsWrapper);
				out << " retval";
				out << Format(GetRetval().Type, TypeFormat::AsInitializer);
				out << ";" << std::endl;
			}
			out << "		";
			if (value.Retval.TypeEnum == TypeEnum::Hresult)
				out << "auto hr = ";
			out << "p->";
			out << prefix;
			out << value.Name << "(";
			WriteArguments(out, ParameterFormat::AsWrapperArgument, false);
			out << ");" << std::endl;
			if (value.Retval.TypeEnum == TypeEnum::Hresult)
				out << "		Com::CheckError(hr, __FUNCTION__, \"\");" << std::endl;
			if (HasRetval())
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
			WriteArguments(out, ParameterFormat::AsNative, false);
			out << ") final" << std::endl
				<< "		{" << std::endl
				<< "			return Com::RunAction([&](){ ";
			if (HasRetval())
				out << Format(GetRetval(), ParameterFormat::AsCoclassReturnValue) << " = ";
			out << prefix << value.Name << "(";
			WriteArguments(out, ParameterFormat::AsCoclassArgument, true);
			out << "); });" << std::endl
				<< "		}" << std::endl;
		}

		void FunctionFormatter::WriteAsCoclassAbstract(std::ostream& out) const
		{
			out << "		virtual ";
			if (HasRetval())
				out << Format(GetRetval().Type, TypeFormat::AsWrapper);
			else
				out << "void";
			out << " " << prefix << value.Name << "(";
			WriteArguments(out, ParameterFormat::AsWrapper, true);
			out << ") = 0;" << std::endl;
		}

		void FunctionFormatter::WriteAsCoclassPrototype(std::ostream& out) const
		{
			out << "		";
			if (HasRetval())
				out << Format(GetRetval().Type, TypeFormat::AsWrapper);
			else
				out << "void";
			out << " " << prefix << value.Name << "(";
			WriteArguments(out, ParameterFormat::AsWrapper, true);
			out << ") final;" << std::endl;
		}

		void FunctionFormatter::WriteAsCoclassImplementation(std::ostream& out) const
		{
			out << "	";
			if (HasRetval())
				out << Format(GetRetval().Type, TypeFormat::AsWrapper);
			else
				out << "void";
			out << " " << scope << "::" << prefix << value.Name << "(";
			WriteArguments(out, ParameterFormat::AsWrapper, true);
			out << ")" << std::endl
				<< "	{" << std::endl
				<< "		throw Com::NotImplemented(__FUNCTION__);" << std::endl
				<< "	}" << std::endl
				<< std::endl;
		}

		bool FunctionFormatter::HasRetval() const
		{
			return !value.ArgList.empty() && value.ArgList.back().Retval;
		}

		const Parameter& FunctionFormatter::GetRetval() const
		{
			return value.ArgList.back();
		}

		void FunctionFormatter::WriteArguments(std::ostream& out, ParameterFormat format, bool skipReturnValue) const
		{
			auto first = true;
			for (auto& argument : value.ArgList)
			{
				if (skipReturnValue && argument.Retval)
					break;
				if (!first)
					out << ", ";
				first = false;
				out << Format(argument, format);
			}
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
