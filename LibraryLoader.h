#pragma once
#include "DataTypes.h"
#include "Loader.h"
#include <set>
#include <string>

namespace Com
{
	namespace Import
	{
		class TypeLibrary;

		class LibraryLoader : public Loader
		{
		private:
			std::set<std::string> loadedLibraries;
			std::set<std::string> pendingLibraries;
			std::set<std::string> currentReferences;

		public:
			LoadLibraryResult Load(const std::string& typeLibraryFileName);
			void Reference(Pointer<ITypeLib> reference) final;

		private:
			static std::string GetTitle(const std::string& fileName);
			Library ImportTypeLibrary(const std::string& typeLibraryFileName);
			static void LoadType(TypeLibrary& typeLibrary, UINT index, Library& library);
		};
	}
}
