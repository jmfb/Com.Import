#include "TypeFormatter.h"
#include <stdexcept>
#include <set>
#include <string>

namespace Com
{
	namespace Import
	{
		TypeFormatter::TypeFormatter(const Type& value, TypeFormat format)
			: value(value), format(format)
		{
		}

		std::ostream& TypeFormatter::Write(std::ostream& out) const
		{
			switch (format)
			{
			case TypeFormat::AsNative:
				WriteAsNative(out);
				break;
			case TypeFormat::AsWrapper:
				WriteAsWrapper(out);
				break;
			case TypeFormat::AsSuffix:
				WriteAsSuffix(out);
				break;
			case TypeFormat::AsInitializer:
				WriteAsInitializer(out);
				break;
			}
			return out;
		}

		std::ostream& operator<<(std::ostream& out, const TypeFormatter& value)
		{
			return value.Write(out);
		}

		void TypeFormatter::WriteAsNative(std::ostream& out) const
		{
			switch (value.TypeEnum)
			{
			case TypeEnum::Enum: out << value.CustomName; break;
			case TypeEnum::Record: out << value.CustomName; break;
			case TypeEnum::Interface: out << value.CustomName; break;
			case TypeEnum::Void: out << "void"; break;
			case TypeEnum::Int: out << "int"; break;
			case TypeEnum::Int8: out << "char"; break;
			case TypeEnum::Int16: out << "short"; break;
			case TypeEnum::Int32: out << "long"; break;
			case TypeEnum::Int64: out << "long long"; break;
			case TypeEnum::UInt: out << "unsigned int"; break;
			case TypeEnum::UInt8: out << "unsigned char"; break;
			case TypeEnum::UInt16: out << "unsigned short"; break;
			case TypeEnum::UInt32: out << "unsigned long"; break;
			case TypeEnum::UInt64: out << "unsigned long long"; break;
			case TypeEnum::Float: out << "float"; break;
			case TypeEnum::Double: out << "double"; break;
			case TypeEnum::Currency: out << "CURRENCY"; break;
			case TypeEnum::Date: out << "DATE"; break;
			case TypeEnum::String: out << "BSTR"; break;
			case TypeEnum::Dispatch: out << "IDispatch*"; break;
			case TypeEnum::Error: out << "SCODE"; break;
			case TypeEnum::Bool: out << "VARIANT_BOOL"; break;
			case TypeEnum::Variant: out << "VARIANT"; break;
			case TypeEnum::Decimal: out << "DECIMAL"; break;
			case TypeEnum::Unknown: out << "IUnknown*"; break;
			case TypeEnum::Hresult: out << "HRESULT"; break;
			case TypeEnum::SafeArray: out << "SAFEARRAY*"; break;
			case TypeEnum::Guid: out << "::GUID"; break;
			case TypeEnum::StringPtrA: out << "char*"; break;
			case TypeEnum::StringPtrW: out << "wchar_t*"; break;
			default:
				throw std::runtime_error("Invalid type.");
			}
			if (value.Indirection > 0)
				out << std::string(value.Indirection, '*');
		}

		void TypeFormatter::WriteAsWrapper(std::ostream& out) const
		{
			switch (value.TypeEnum)
			{
			case TypeEnum::Enum: out << value.CustomName; break;
			case TypeEnum::Record: out << value.CustomName; break;
			case TypeEnum::Interface: out << GetSmartPointer(); break;
			case TypeEnum::Int: out << "int"; break;
			case TypeEnum::Int8: out << "char"; break;
			case TypeEnum::Int16: out << "short"; break;
			case TypeEnum::Int32: out << "long"; break;
			case TypeEnum::Int64: out << "long long"; break;
			case TypeEnum::UInt: out << "unsigned int"; break;
			case TypeEnum::UInt8: out << "unsigned char"; break;
			case TypeEnum::UInt16: out << "unsigned short"; break;
			case TypeEnum::UInt32: out << "unsigned long"; break;
			case TypeEnum::UInt64: out << "unsigned long long"; break;
			case TypeEnum::Float: out << "float"; break;
			case TypeEnum::Double: out << "double"; break;
			case TypeEnum::Currency: out << "CURRENCY"; break;
			case TypeEnum::Date: out << "std::chrono::system_clock::time_point"; break;
			case TypeEnum::String: out << "std::string"; break;
			case TypeEnum::Dispatch: out << "Com::Pointer<IDispatch>"; break;
			case TypeEnum::Error: out << "SCODE"; break;
			case TypeEnum::Bool: out << "bool"; break;
			case TypeEnum::Variant: out << "Com::Variant"; break;
			case TypeEnum::Decimal: out << "DECIMAL"; break;
			case TypeEnum::Unknown: out << "Com::Pointer<IUnknown>"; break;
			case TypeEnum::Hresult: out << "HRESULT"; break;
			case TypeEnum::SafeArray: out << "SAFEARRAY*"; break;
			case TypeEnum::Guid: out << "::GUID"; break;
			default:
				throw std::runtime_error("Invalid type as return value.");
			}
		}

		void TypeFormatter::WriteAsSuffix(std::ostream& out) const
		{
			if (value.IsArray)
				out << "[" << value.ArraySize << "]";
		}

		void TypeFormatter::WriteAsInitializer(std::ostream& out) const
		{
			switch (value.TypeEnum)
			{
			case TypeEnum::Int:
			case TypeEnum::Int8:
			case TypeEnum::Int16:
			case TypeEnum::Int32:
			case TypeEnum::Int64:
			case TypeEnum::UInt:
			case TypeEnum::UInt8:
			case TypeEnum::UInt16:
			case TypeEnum::UInt32:
			case TypeEnum::UInt64:
			case TypeEnum::Float:
			case TypeEnum::Double:
			case TypeEnum::Error:
				out << " = 0";
				break;

			case TypeEnum::Bool:
				out << " = false";
				break;

			case TypeEnum::Hresult:
				out << " = S_OK";
				break;
			}
		}

		std::string TypeFormatter::GetSmartPointer() const
		{
			std::set<std::string> standardInterfaces
			{
				"IUnknown",
				"IDispatch",
				"IPicture",
				"IPictureDisp",
				"IFont",
				"IEnumVARIANT"
			};
			auto isStandardInterface = standardInterfaces.find(value.CustomName) != standardInterfaces.end();
			return isStandardInterface ?
				"Com::Pointer<" + value.CustomName + ">" :
				value.CustomName + "Ptr";
		}

		TypeFormatter Format(const Type& type, TypeFormat format)
		{
			return{ type, format };
		}
	}
}
