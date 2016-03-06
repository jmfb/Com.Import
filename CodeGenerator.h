#pragma once
#include "DataTypes.h"

namespace Com
{
	namespace Import
	{
		class CodeGenerator
		{
		public:
			static void Generate(const LoadLibraryResult& result, bool implement);

		private:
			static void GenerateImport(const Library& library, bool implement);
			static void GenerateSolution(const LoadLibraryResult& result);
			static void GenerateProject(const LoadLibraryResult& result);
			static void GenerateProjectFilters(const LoadLibraryResult& result);
			static void GeneratePackages();
			static void GenerateResourceHeader(const Library& library);
			static void GenerateResources(const Library& library);
			static void GenerateDef(const Library& library);
			static void GenerateManifest(const Library& library);
			static void GenerateMain(const Library& library);
			static void GenerateCoclassHeader(const Library& library, const Coclass& coclass);
			static void GenerateCoclassSource(const Library& library, const Coclass& coclass);
		};
	}
}
