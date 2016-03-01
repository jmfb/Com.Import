#pragma once
#include "DataTypes.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <type_traits>
#include <locale>
#include <codecvt>

namespace Com
{
	namespace Import
	{
		class CodeGenerator
		{
		private:
			std::ostream& out;

		public:
			CodeGenerator(std::ostream& out)
				: out(out)
			{
			}
			CodeGenerator(const CodeGenerator& rhs) = delete;
			~CodeGenerator() = default;

			CodeGenerator& operator=(const CodeGenerator& rhs) = delete;

			void Write(const Library& library)
			{
				out << "#pragma once" << std::endl
					<< "#include <objbase.h>" << std::endl
					//TODO: library cross references
					<< "#pragma pack(push, 8)" << std::endl
					<< "namespace " << library.Name << std::endl
					<< "{" << std::endl;
				Write(library.Enums);
				ForwardDeclare(library.Interfaces);
				Write(library.Aliases);
				Write(library.Records);
				Write(library.Interfaces);
				Write(library.Identifiers);
				out << "}" << std::endl
					<< "#pragma pack(pop)" << std::endl;
			}

		private:
			void Write(const std::vector<Enum>& enums)
			{
				for (auto& enumeration : enums)
					Write(enumeration);
			}
			void Write(const Enum& enumeration)
			{
				out << "\tenum class " << enumeration.Name << std::endl
					<< "\t{" << std::endl;
				auto first = true;
				for (auto& value : enumeration.Values)
				{
					if (!first)
						out << "," << std::endl;
					first = false;
					out << "\t\t" << value.Name << " = ";
					if (value.Value & 0xffff0000)
						WriteHex(value.Value);
					else
						out << value.Value;
				}
				out << std::endl
					<< "\t};" << std::endl;
			}

			void ForwardDeclare(const std::vector<Interface>& interfaces)
			{
				for (auto& iface : interfaces)
					ForwardDeclare(iface);
			}
			void ForwardDeclare(const Interface& iface)
			{
				out << "\tclass __declspec(uuid(\"" << Format(iface.Iid) << "\")) " << iface.Name << ";" << std::endl;
			}

			void Write(const std::vector<Alias>& aliases)
			{
				for (auto& alias : aliases)
					Write(alias);
			}
			void Write(const Alias& alias)
			{
				out << "\tusing " << alias.NewName << " = " << alias.OldName << ";" << std::endl;
			}

			void Write(const std::vector<Record>& records)
			{
				for (auto& record : records)
					Write(record);
			}
			void Write(const Record& record)
			{
				out << "\t#pragma pack(push, " << record.Alignment << ")" << std::endl
					<< "\tstruct __declspec(uuid(\"" << Format(record.Guid) << "\")) " << record.Name << std::endl
					<< "\t{" << std::endl;
				for (auto& member : record.Members)
				{
					out << "\t\t";
					WriteType(member.Type);
					out << " " << member.Name;
					WriteTypeSuffix(member.Type);
					out << ";" << std::endl;
				}
				out << "\t};" << std::endl
					<< "\t#pragma pack(pop)" << std::endl;
			}

			void Write(const std::vector<Interface>& interfaces)
			{
				for (auto& iface : interfaces)
					Write(iface);
			}
			void Write(const Interface& iface)
			{
				out << "\tclass __declspec(uuid(\"" << Format(iface.Iid) << "\")) " << iface.Name << " : public " << iface.Base << std::endl
					<< "\t{" << std::endl
					<< "\tpublic:" << std::endl;
				auto nextPlaceholderId = 1;
				auto nextValidOffset = iface.VtblOffset;
				for (auto& function : iface.Functions)
				{
					if (function.VtblOffset == 0 && function.IsDispatchOnly)
					{
						Write(function);
						continue;
					}

					if (function.VtblOffset < nextValidOffset)
						continue;

					while (nextValidOffset < function.VtblOffset)
					{
						out << "\t\tvirtual HRESULT __stdcall _VtblGapPlaceholder" << nextPlaceholderId << "() { return E_NOTIMPL; }" << std::endl;
						++nextPlaceholderId;
						nextValidOffset += 4;
					}

					Write(function);
					nextValidOffset += 4;
				}
				out << "\t};" << std::endl;
			}
			void Write(const Function& function)
			{
				out << "\t\t";
				if (function.IsDispatchOnly)
					out << "//";
				out << "virtual ";
				WriteType(function.Retval);
				out << " __stdcall " << function.Name << "(";
				auto first = true;
				for (auto& argument : function.ArgList)
				{
					if (!first)
						out << ", ";
					first = false;
					WriteType(argument.Type);
					out << " " << argument.Name;
				}
				out << ") = 0;" << std::endl;
			}

			void Write(const std::vector<Identifier>& identifiers)
			{
				for (auto& identifier : identifiers)
					Write(identifier);
			}
			void Write(const Identifier& identifier)
			{
				out << "\textern \"C\" const ::GUID __declspec(selectany) " << identifier.Name << " = {";
				WriteHex(identifier.Guid.Data1);
				out << ",";
				WriteHex(identifier.Guid.Data2);
				out << ",";
				WriteHex(identifier.Guid.Data3);
				out << ",{";
				auto first = true;
				for (auto part : identifier.Guid.Data4)
				{
					if (!first)
						out << ",";
					first = false;
					WriteHex(part);
				}
				out << "}};" << std::endl;
			}

			void WriteType(const Type& type)
			{
				switch (type.TypeEnum)
				{
				case TypeEnum::Enum:
				case TypeEnum::Record:
				case TypeEnum::Interface: out << type.CustomName; break;
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
				if (type.Indirection > 0)
					out << std::string(type.Indirection, '*');

			}
			void WriteTypeSuffix(const Type& type)
			{
				if (type.IsArray)
					out << "[" << type.ArraySize << "]";
			}

			static std::string Format(const GUID& guid)
			{
				const auto bufferSize = 39;
				wchar_t buffer[bufferSize];
				auto result = ::StringFromGUID2(guid, buffer, bufferSize);
				if (result != bufferSize)
					throw std::runtime_error("Error formatting GUID");
				return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(buffer).substr(1, 36);
			}

			template <typename Integer>
			void WriteHex(Integer value)
			{
				static_assert(sizeof(value) <= sizeof(int), "Integer type is too large.");
				using UnsignedInteger = std::make_unsigned<Integer>::type;
				out << "0x"
					<< std::setw(sizeof(value) * 2)
					<< std::setfill('0')
					<< std::right
					<< std::hex
					<< static_cast<unsigned int>(static_cast<UnsignedInteger>(value))
					<< std::dec;
			}
		};
	}
};
