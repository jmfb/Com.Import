#pragma once
#include "DataTypes.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <type_traits>

namespace Com
{
	namespace Import
	{
		class CodeGenerator
		{
		private:
			std::ostream& out;
			bool implement;

		public:
			CodeGenerator(std::ostream& out, bool implement);
			CodeGenerator(const CodeGenerator& rhs) = delete;
			~CodeGenerator() = default;

			CodeGenerator& operator=(const CodeGenerator& rhs) = delete;

			static void Generate(const LoadLibraryResult& result, bool implement);

		private:
			static void Generate(const Library& library, bool implement);
			static void GenerateMain(const Library& library);
			static void GenerateCoclasses(const Library& library);
			static void GenerateCoclassHeader(const Library& library, const Coclass& coclass);
			static void GenerateCoclassSource(const Library& library, const Coclass& coclass);
			static void GenerateDef(const Library& library);
			static void GenerateResources(const Library& library);
			static void GeneratePackages();
			static void GenerateManifest(const Library& library);
			static void GenerateProject(const LoadLibraryResult& result);
			static void GenerateProjectFilters(const LoadLibraryResult& result);
			static std::string GetLibraryOutputName(const Library& library);
			void Write(const Library& library);
			void Write(const std::vector<Enum>& enums);
			void Write(const Enum& enumeration);
			static bool ShouldDisplayAsHex(const EnumValue& value);
			void ForwardDeclare(const std::vector<Interface>& interfaces);
			void ForwardDeclare(const Interface& iface);
			void Write(const std::vector<Alias>& aliases);
			void Write(const Alias& alias);
			void Write(const std::vector<Record>& records);
			void Write(const Record& record);
			void Write(const std::vector<Interface>& interfaces);
			void Write(const Interface& iface);
			void Write(const Function& function);
			void Write(const std::vector<Identifier>& identifiers);
			void Write(const Identifier& identifier);
			void Write(const Parameter& parameter);
			void WriteType(const Type& type);
			void WriteTypeAsRetval(const Type& type);
			void WriteTypeSuffix(const Type& type);
			void Write(const std::vector<Coclass>& coclasses);
			void Write(const Coclass& coclass);
			void WriteNativeFunctions(const Interface& iface, bool interfaceSpecificFunctions);
			void WriteRawFunctions(const Interface& iface, bool interfaceSpecificFunctions);
			void WriteWrappers(const std::vector<Interface>& interfaces);
			void WriteWrapper(const Interface& iface);
			void WriteComTypeInfo(const std::string& libraryName, const std::vector<Interface>& interfaces);
			void WriteComTypeInfo(const std::string& libraryName, const Interface& iface);
			void WriteWrapperFunctions(const Interface& iface);
			void WriteWrapperDispatch(const std::string& interfaceName, const Function& function);
			void WriteWrapperFunction(const std::string& interfaceName, const Function& function);
			void WriteDefault(const Type& type);
			static std::string Format(const GUID& guid);
			static std::string GetWrapperBase(const Interface& iface);
			static std::string GetSmartPointer(const Type& type);
			static bool IsStandardOle(const Type& type);

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
