#pragma once
#include "DataTypes.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <type_traits>
#include <set>

namespace Com
{
	namespace Import
	{
		enum class FunctionDefinition
		{
			Abstract,
			Prototype,
			Definition
		};

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
			static void GenerateSolution(const LoadLibraryResult& result);
			static void GenerateProject(const LoadLibraryResult& result);
			static void GenerateProjectFilters(const LoadLibraryResult& result);
			void Write(const Library& library);
			void Write(const std::vector<Enum>& enums);
			void ForwardDeclare(const std::vector<Interface>& interfaces);
			void Write(const std::vector<Alias>& aliases);
			void Write(const std::vector<Record>& records);
			void Write(const std::vector<Interface>& interfaces);
			void Write(const std::vector<Identifier>& identifiers);
			void Write(const std::vector<Coclass>& coclasses);
			void Write(const Coclass& coclass);
			void WriteWrappers(const std::vector<Interface>& interfaces);
			void WriteComTypeInfo(const std::string& libraryName, const std::vector<Interface>& interfaces);
		};
	}
};
