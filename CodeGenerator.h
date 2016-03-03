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

		public:
			CodeGenerator(std::ostream& out);
			CodeGenerator(const CodeGenerator& rhs) = delete;
			~CodeGenerator() = default;

			CodeGenerator& operator=(const CodeGenerator& rhs) = delete;

			static void Generate(const LoadLibraryResult& result);
			static void Generate(const Library& library);
			void Write(const Library& library);

		private:
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
			void WriteType(const Type& type);
			void WriteTypeSuffix(const Type& type);
			static std::string Format(const GUID& guid);

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
