#include "ParameterFormatter.h"
#include "TypeFormatter.h"

namespace Com
{
	namespace Import
	{
		ParameterFormatter::ParameterFormatter(const Parameter& value, ParameterFormat format)
			: value(value), format(format)
		{
		}

		std::ostream& ParameterFormatter::Write(std::ostream& out) const
		{
			switch (format)
			{
			case ParameterFormat::AsName:
				WriteAsName(out);
				break;
			case ParameterFormat::AsNative:
				WriteAsNative(out);
				break;
			case ParameterFormat::AsWrapper:
				WriteAsWrapper(out);
				break;
			case ParameterFormat::AsWrapperArgument:
				WriteAsWrapperArgument(out);
				break;
			case ParameterFormat::AsCoclassReturnValue:
				WriteAsCoclassReturnValue(out);
				break;
			case ParameterFormat::AsCoclassArgument:
				WriteAsCoclassArgument(out);
				break;
			}
			return out;
		}

		std::ostream& operator<<(std::ostream& out, const ParameterFormatter& value)
		{
			return value.Write(out);
		}

		void ParameterFormatter::WriteAsName(std::ostream& out) const
		{
			out << value.Name;
		}

		void ParameterFormatter::WriteAsNative(std::ostream& out) const
		{
			out << Format(value.Type, TypeFormat::AsNative) << " " << value.Name;
		}

		void ParameterFormatter::WriteAsWrapper(std::ostream& out) const
		{
			out << Format(value.Type, TypeFormat::AsWrapper);
			if (value.Out)
				out << "&";
			out << " " << value.Name;
		}

		void ParameterFormatter::WriteAsWrapperArgument(std::ostream& out) const
		{
			if (value.In && value.Out)
				out << "Com::PutRef";
			else if (value.In)
				out << "Com::Put";
			else
				out << "Com::Get";
			out << "(" << value.Name << ")";
		}

		void ParameterFormatter::WriteAsCoclassReturnValue(std::ostream& out) const
		{
			if (value.Type.TypeEnum == TypeEnum::Int16)
				out << "Com::CheckPointer";
			else
				out << "Com::Retval";
			out << "(" << value.Name << ")";
		}

		void ParameterFormatter::WriteAsCoclassArgument(std::ostream& out) const
		{
			if (value.Out)
			{
				if (value.Type.TypeEnum == TypeEnum::Int16)
					out << "Com::CheckPointer(" << value.Name << ")";
				else
					out << "Com::InOut(" << value.Name << ")";
			}
			else
			{
				if (value.Type.TypeEnum == TypeEnum::Int16)
					out << value.Name;
				else
					out << "Com::In(" << value.Name << ")";
			}
		}

		ParameterFormatter Format(const Parameter& value, ParameterFormat format)
		{
			return{ value, format };
		}
	}
}
