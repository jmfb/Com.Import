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
			case ParameterFormat::AsNative:
				WriteAsNative(out);
				break;
			case ParameterFormat::AsWrapper:
				WriteAsWrapper(out);
				break;
			case ParameterFormat::AsWrapperReturnValue:
				WriteAsWrapperReturnValue(out);
				break;
			case ParameterFormat::AsWrapperArgument:
				WriteAsWrapperArgument(out);
				break;
			}
			return out;
		}

		std::ostream& operator<<(std::ostream& out, const ParameterFormatter& value)
		{
			return value.Write(out);
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

		void ParameterFormatter::WriteAsWrapperReturnValue(std::ostream& out) const
		{
			if (value.Type.TypeEnum == TypeEnum::Int16)
				out << "Com::CheckPointer";
			else
				out << "Com::Retval";
			out << "(" << value.Name << ")";
		}

		void ParameterFormatter::WriteAsWrapperArgument(std::ostream& out) const
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
