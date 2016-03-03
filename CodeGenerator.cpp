#include "CodeGenerator.h"
#include <iomanip>
#include <type_traits>
#include <locale>
#include <codecvt>
#include <fstream>
#include <algorithm>

namespace Com
{
	namespace Import
	{
		CodeGenerator::CodeGenerator(std::ostream& out, bool implement)
			: out(out), implement(implement)
		{
		}

		void CodeGenerator::Generate(const LoadLibraryResult& result, bool implement)
		{
			Generate(result.PrimaryLibrary, implement);
			for (auto& reference : result.ReferencedLibraries)
				Generate(reference, false);
		}

		void CodeGenerator::Generate(const Library& library, bool implement)
		{
			std::cout << "Generating header: " << library.HeaderFileName << std::endl;
			CodeGenerator{ std::ofstream{ library.HeaderFileName.c_str() }, implement }.Write(library);
			if (!implement)
				return;
			GenerateMain(library);
			GenerateCoclasses(library);
			//TODO: generate rc, def, manifest
		}

		void CodeGenerator::GenerateMain(const Library& library)
		{
			std::cout << "Generating source: main.cpp" << std::endl;
			std::ofstream out{ "main.cpp" };
			out << "#include \"Com/Com.h\"" << std::endl;
			for (auto& coclass : library.Coclasses)
				out << "#include \"" << coclass.Name << ".h\"" << std::endl;
			out << std::endl
				<< "extern \"C\" BOOL __stdcall DllMain(HINSTANCE instance, DWORD reason, void* reserved)" << std::endl
				<< "{" << std::endl
				<< "\tif (reason == DLL_PROCESS_ATTACH)" << std::endl
				<< "\t\tCom::Module::GetInstance().Initialize(instance);" << std::endl
				<< "\treturn TRUE;" << std::endl
				<< "}" << std::endl
				<< std::endl
				<< "HRESULT __stdcall DllCanUnloadNow()" << std::endl
				<< "{" << std::endl
				<< "\treturn Com::Module::GetInstance().CanUnload() ? S_OK : S_FALSE;" << std::endl
				<< "}" << std::endl
				<< std::endl
				<< "HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppvObject)" << std::endl
				<< "{" << std::endl
				<< "\treturn Com::ObjectList<" << std::endl;
			auto first = true;
			for (auto& coclass : library.Coclasses)
			{
				if (!first)
					out << "," << std::endl;
				first = false;
				out << "\t\t" << library.Name << "::" << coclass.Name;
			}
			out << std::endl
				<< "\t>::Create(rclsid, riid, ppvObject);" << std::endl
				<< "}" << std::endl;
		}

		void CodeGenerator::GenerateCoclasses(const Library& library)
		{
			for (auto& coclass : library.Coclasses)
			{
				GenerateCoclassHeader(library, coclass);
				GenerateCoclassSource(library, coclass);
			}
		}

		void CodeGenerator::GenerateCoclassHeader(const Library& library, const Coclass& coclass)
		{
			auto fileName = coclass.Name + ".h";
			std::cout << "Generating header: " << fileName << std::endl;
			std::ofstream out{ fileName.c_str() };
			out << "#pragma once" << std::endl
				<< "#include \"" << library.HeaderFileName << "\"" << std::endl
				<< std::endl
				<< "namespace " << library.Name << std::endl
				<< "{" << std::endl
				<< "\tclass " << coclass.Name << " : public " << coclass.Name << "Coclass<" << coclass.Name << ">" << std::endl
				<< "\t{" << std::endl
				<< "\tpublic:" << std::endl
				<< "\t};" << std::endl
				<< "}" << std::endl;
		}

		void CodeGenerator::GenerateCoclassSource(const Library& library, const Coclass& coclass)
		{
			auto fileName = coclass.Name + ".cpp";
			std::cout << "Generating source: " << fileName << std::endl;
			std::ofstream out{ fileName.c_str() };
			out << "#include \"" << coclass.Name << ".h\"" << std::endl
				<< std::endl
				<< "namespace " << library.Name << std::endl
				<< "{" << std::endl
				<< "}" << std::endl;
		}

		void CodeGenerator::Write(const Library& library)
		{
			out << "#pragma once" << std::endl
				<< "#include \"Com/Com.h\"" << std::endl;
			for (auto& reference : library.References)
				out << "#include \"" << reference << "\"" << std::endl;
			out << "#pragma pack(push, 8)" << std::endl
				<< "namespace " << library.Name << std::endl
				<< "{" << std::endl;
			Write(library.Enums);
			ForwardDeclare(library.Interfaces);
			Write(library.Aliases);
			Write(library.Records);
			Write(library.Interfaces);
			Write(library.Identifiers);
			WriteWrappers(library.Interfaces);
			if (implement)
				Write(library.Coclasses);
			out << "}" << std::endl;
			WriteComTypeInfo(library.Name, library.Interfaces);
			out << "#pragma pack(pop)" << std::endl;
		}

		void CodeGenerator::Write(const std::vector<Enum>& enums)
		{
			for (auto& enumeration : enums)
				Write(enumeration);
		}

		void CodeGenerator::Write(const Enum& enumeration)
		{
			out << "\tenum class " << enumeration.Name;
			if (std::any_of(enumeration.Values.begin(), enumeration.Values.end(), &ShouldDisplayAsHex))
				out << " : unsigned";
			out << std::endl
				<< "\t{" << std::endl;
			auto first = true;
			for (auto& value : enumeration.Values)
			{
				if (!first)
					out << "," << std::endl;
				first = false;
				out << "\t\t" << value.Name << " = ";
				if (ShouldDisplayAsHex(value))
					WriteHex(value.Value);
				else
					out << value.Value;
			}
			out << std::endl
				<< "\t};" << std::endl;
		}

		bool CodeGenerator::ShouldDisplayAsHex(const EnumValue& value)
		{
			return (value.Value & 0xf0000000) == 0x80000000;
		}

		void CodeGenerator::ForwardDeclare(const std::vector<Interface>& interfaces)
		{
			for (auto& iface : interfaces)
				ForwardDeclare(iface);
		}

		void CodeGenerator::ForwardDeclare(const Interface& iface)
		{
			out << "\tclass __declspec(uuid(\"" << Format(iface.Iid) << "\")) " << iface.Name << ";" << std::endl
				<< "\ttemplate <typename Interface> class " << iface.Name << "PtrT;" << std::endl
				<< "\tusing " << iface.Name << "Ptr = " << iface.Name << "PtrT<" << iface.Name << ">;" << std::endl;
		}

		void CodeGenerator::Write(const std::vector<Alias>& aliases)
		{
			for (auto& alias : aliases)
				Write(alias);
		}

		void CodeGenerator::Write(const Alias& alias)
		{
			out << "\tusing " << alias.NewName << " = " << alias.OldName << ";" << std::endl;
		}

		void CodeGenerator::Write(const std::vector<Record>& records)
		{
			for (auto& record : records)
				Write(record);
		}

		void CodeGenerator::Write(const Record& record)
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

		void CodeGenerator::Write(const std::vector<Interface>& interfaces)
		{
			for (auto& iface : interfaces)
				Write(iface);
		}

		void CodeGenerator::Write(const Interface& iface)
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

		void CodeGenerator::Write(const Function& function)
		{
			out << "\t\t";
			if (function.IsDispatchOnly)
				out << "//";
			out << "virtual ";
			WriteType(function.Retval);
			out << " __stdcall ";
			if (implement)
				out << "raw_";
			out << function.Name << "(";
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

		void CodeGenerator::Write(const std::vector<Identifier>& identifiers)
		{
			for (auto& identifier : identifiers)
				Write(identifier);
		}

		void CodeGenerator::Write(const Identifier& identifier)
		{
			out << "\textern const ::GUID __declspec(selectany) " << identifier.Name << " = {";
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

		void CodeGenerator::Write(const Parameter& parameter)
		{
			switch (parameter.Type.TypeEnum)
			{
			case TypeEnum::Enum:
			case TypeEnum::Record: out << parameter.Type.CustomName; break;
			case TypeEnum::Interface: out << GetSmartPointer(parameter.Type); break;
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
			if (parameter.Out)
				out << "&";
			out << " " << parameter.Name;
		}

		void CodeGenerator::WriteType(const Type& type)
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

		void CodeGenerator::WriteTypeAsRetval(const Type& type)
		{
			switch (type.TypeEnum)
			{
			case TypeEnum::Enum:
			case TypeEnum::Record: out << type.CustomName; break;
			case TypeEnum::Interface: out << GetSmartPointer(type); break;
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

		void CodeGenerator::WriteTypeSuffix(const Type& type)
		{
			if (type.IsArray)
				out << "[" << type.ArraySize << "]";
		}

		void CodeGenerator::Write(const std::vector<Coclass>& coclasses)
		{
			for (auto& coclass : coclasses)
				Write(coclass);
		}

		void CodeGenerator::Write(const Coclass& coclass)
		{
			out << "\ttemplate <typename Type>" << std::endl
				<< "\tclass " << coclass.Name << "Coclass : public Com::Object<Type, &CLSID_" << coclass.Name;
			for (auto& iface : coclass.Interfaces)
				out << ", " << iface.Name;
			out << ">" << std::endl
				<< "\t{" << std::endl
				<< "\tpublic:" << std::endl;
			for (auto& iface : coclass.Interfaces)
				WriteNativeFunctions(iface);
			for (auto& iface : coclass.Interfaces)
				WriteRawFunctions(iface);
			out << "\t};" << std::endl;
		}

		void CodeGenerator::WriteNativeFunctions(const Interface& iface)
		{
			for (auto& function : iface.Functions)
			{
				if (function.VtblOffset >= iface.VtblOffset && function.Retval.TypeEnum == TypeEnum::Hresult)
				{
					out << "\t\tvirtual ";
					if (!function.ArgList.empty() && function.ArgList.back().Retval)
						WriteTypeAsRetval(function.ArgList.back().Type);
					else
						out << "void";
					out << " " << function.Name << "(";
					auto first = true;
					for (auto& argument : function.ArgList)
					{
						if (argument.Retval)
							break;
						if (!first)
							out << ", ";
						first = false;
						Write(argument);
					}
					out << ")" << std::endl
						<< "\t\t{" << std::endl
						<< "\t\t\tthrow Com::NotImplemented(__FUNCTION__);" << std::endl
						<< "\t\t}" << std::endl;
				}
			}
		}

		void CodeGenerator::WriteRawFunctions(const Interface& iface)
		{
			for (auto& function : iface.Functions)
			{
				if (function.VtblOffset >= iface.VtblOffset && function.Retval.TypeEnum == TypeEnum::Hresult)
				{
					out << "\t\tHRESULT __stdcall raw_" << function.Name << "(";
					auto first = true;
					for (auto& argument : function.ArgList)
					{
						if (!first)
							out << ", ";
						first = false;
						WriteType(argument.Type);
						out << " " << argument.Name;
					}
					out << ") final" << std::endl
						<< "\t\t{" << std::endl
						<< "\t\t\ttry" << std::endl
						<< "\t\t\t{" << std::endl
						<< "\t\t\t\t";
					if (!function.ArgList.empty() && function.ArgList.back().Retval)
					{
						auto& retval = function.ArgList.back();
						if (retval.Type.TypeEnum == TypeEnum::Int16)
							out << "Com::CheckPointer(" << retval.Name << ") = ";
						else
							out << "Com::Retval(" << retval.Name << ") = ";
					}
					out << function.Name << "(";
					first = true;
					for (auto& argument : function.ArgList)
					{
						if (argument.Retval)
							break;
						if (!first)
							out << ", ";
						first = false;
						if (argument.Out)
						{
							if (argument.Type.TypeEnum == TypeEnum::Int16)
								out << "Com::CheckPointer(" << argument.Name << ")";
							else
								out << "Com::InOut(" << argument.Name << ")";
						}
						else
						{
							if (argument.Type.TypeEnum == TypeEnum::Int16)
								out << argument.Name;
							else
								out << "Com::In(" << argument.Name << ")";
						}
					}
					out << ");" << std::endl
						<< "\t\t\t}" << std::endl
						<< "\t\t\tcatch (...)" << std::endl
						<< "\t\t\t{" << std::endl
						<< "\t\t\t\treturn Com::HandleException();" << std::endl
						<< "\t\t\t}" << std::endl
						<< "\t\t\treturn S_OK;" << std::endl
						<< "\t\t}" << std::endl;
				}
			}
		}

		void CodeGenerator::WriteWrappers(const std::vector<Interface>& interfaces)
		{
			for (auto& iface : interfaces)
				WriteWrapper(iface);
			for (auto& iface : interfaces)
				WriteWrapperFunctions(iface);
		}

		void CodeGenerator::WriteWrapper(const Interface& iface)
		{
			out << "\ttemplate <typename Interface>" << std::endl
				<< "\tclass " << iface.Name << "PtrT : public " << GetWrapperBase(iface) << std::endl
				<< "\t{" << std::endl
				<< "\tpublic:" << std::endl
				<< "\t\t" << iface.Name << "PtrT(Interface* value = nullptr);" << std::endl
				<< "\t\t" << iface.Name << "PtrT<Interface>& operator=(Interface* value);" << std::endl
				<< "\t\toperator " << iface.Name << "*() const;" << std::endl;
			for (auto& function : iface.Functions)
			{
				if ((function.VtblOffset == 0 && function.IsDispatchOnly) ||
					(function.VtblOffset >= iface.VtblOffset))
				{
					out << "\t\t";
					if (!function.ArgList.empty() && function.ArgList.back().Retval)
						WriteTypeAsRetval(function.ArgList.back().Type);
					else
						out << "void";
					out << " " << function.Name << "(";
					auto first = true;
					for (auto& argument : function.ArgList)
					{
						if (argument.Retval)
							break;
						if (!first)
							out << ", ";
						first = false;
						Write(argument);
					}
					out << ");" << std::endl;
				}
			}
			out << "\t};" << std::endl;
		}

		void CodeGenerator::WriteComTypeInfo(const std::string& libraryName, const std::vector<Interface>& interfaces)
		{
			out << "namespace Com" << std::endl
				<< "{" << std::endl;
			for (auto& iface : interfaces)
				WriteComTypeInfo(libraryName, iface);
			out << "}" << std::endl;
		}

		void CodeGenerator::WriteComTypeInfo(const std::string& libraryName, const Interface& iface)
		{
			auto interfaceName = libraryName + "::" + iface.Name;
			out << "\ttemplate <>" << std::endl
				<< "\tclass TypeInfo<" << interfaceName << "*>" << std::endl
				<< "\t{" << std::endl
				<< "\tpublic:" << std::endl
				<< "\t\tusing In = InValue<" << interfaceName << "*, " << interfaceName << "Ptr>;" << std::endl
				<< "\t\tusing InOut = InOutValue<" << interfaceName << "*, " << interfaceName << "Ptr>;" << std::endl
				<< "\t\tusing Retval = RetvalValue<" << interfaceName << "Ptr, " << interfaceName << "*>;" << std::endl
				<< "\t};" << std::endl;
		}

		void CodeGenerator::WriteWrapperFunctions(const Interface& iface)
		{
			out << "\ttemplate <typename Interface>" << std::endl
				<< "\tinline " << iface.Name << "PtrT<Interface>::" << iface.Name << "PtrT(Interface* value) : " << GetWrapperBase(iface) << "(value)" << std::endl
				<< "\t{" << std::endl
				<< "\t}" << std::endl
				<< "\ttemplate <typename Interface>" << std::endl
				<< "\tinline " << iface.Name << "PtrT<Interface>& " << iface.Name << "PtrT<Interface>::operator=(Interface* value)" << std::endl
				<< "\t{" << std::endl
				<< "\t\tusing Base = " << GetWrapperBase(iface) << ";" << std::endl
				<< "\t\tBase::operator=(value);" << std::endl
				<< "\t\treturn *this;" << std::endl
				<< "\t}" << std::endl
				<< "\ttemplate <typename Interface>" << std::endl
				<< "\tinline " << iface.Name << "PtrT<Interface>::operator " << iface.Name << "*() const" << std::endl
				<< "\t{" << std::endl
				<< "\t\treturn p;" << std::endl
				<< "\t}" << std::endl;
			for (auto& function : iface.Functions)
			{
				if (function.VtblOffset == 0 && function.IsDispatchOnly)
					WriteWrapperDispatch(iface.Name, function);
				else if (function.VtblOffset >= iface.VtblOffset)
					WriteWrapperFunction(iface.Name, function);
			}
		}

		void CodeGenerator::WriteWrapperDispatch(const std::string& interfaceName, const Function& function)
		{
			//TODO: dispatch only implementation
		}

		void CodeGenerator::WriteWrapperFunction(const std::string& interfaceName, const Function& function)
		{
			out << "\ttemplate <typename Interface>" << std::endl
				<< "\tinline ";
			auto hasRetval = !function.ArgList.empty() && function.ArgList.back().Retval;
			if (hasRetval)
				WriteTypeAsRetval(function.ArgList.back().Type);
			else
				out << "void";
			out << " " << interfaceName << "PtrT<Interface>::" << function.Name << "(";
			auto first = true;
			for (auto& argument : function.ArgList)
			{
				if (argument.Retval)
					break;
				if (!first)
					out << ", ";
				first = false;
				Write(argument);
			}
			out << ")" << std::endl
				<< "\t{" << std::endl;
			if (hasRetval)
			{
				out << "\t\t";
				WriteTypeAsRetval(function.ArgList.back().Type);
				out << " retval";
				WriteDefault(function.ArgList.back().Type);
				out << ";" << std::endl;
			}
			out << "\t\t";
			if (function.Retval.TypeEnum == TypeEnum::Hresult)
				out << "auto hr = ";
			out << "p->";
			if (implement)
				out << "raw_";
			out << function.Name << "(";
			first = true;
			for (auto& argument : function.ArgList)
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
			if (function.Retval.TypeEnum == TypeEnum::Hresult)
				out << "\t\tCom::CheckError(hr, __FUNCTION__, \"\");" << std::endl;
			if (hasRetval)
				out << "\t\treturn retval;" << std::endl;
			out << "\t}" << std::endl;
		}

		void CodeGenerator::WriteDefault(const Type& type)
		{
			switch (type.TypeEnum)
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

		std::string CodeGenerator::Format(const GUID& guid)
		{
			const auto bufferSize = 39;
			wchar_t buffer[bufferSize];
			auto result = ::StringFromGUID2(guid, buffer, bufferSize);
			if (result != bufferSize)
				throw std::runtime_error("Error formatting GUID");
			return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(buffer).substr(1, 36);
		}

		std::string CodeGenerator::GetWrapperBase(const Interface& iface)
		{
			if (iface.Base == "IUnknown" || iface.Base == "IDispatch")
				return "Com::Pointer<Interface>";
			return iface.Base + "PtrT<Interface>";
		}

		std::string CodeGenerator::GetSmartPointer(const Type& type)
		{
			return IsStandardOle(type) ?
				"Com::Pointer<" + type.CustomName + ">" :
				type.CustomName + "Ptr";
		}

		bool CodeGenerator::IsStandardOle(const Type& type)
		{
			return type.CustomName == "IPicture" ||
				type.CustomName == "IPictureDisp" ||
				type.CustomName == "IFont" ||
				type.CustomName == "IEnumVARIANT" ||
				type.CustomName == "GUID";
		}
	}
};
