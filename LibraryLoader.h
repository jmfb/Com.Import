#pragma once
#include "DataTypes.h"
#include "Loader.h"
#include "TypeLibrary.h"
#include "TypeInfo.h"
#include "RecordSorter.h"
#include <set>
#include <string>
#include <exception>
#include <stdexcept>
#include <iostream>

namespace Com
{
	namespace Import
	{
		class LibraryLoader : public Loader
		{
		private:
			std::set<std::string> loadedLibraries;
			std::set<std::string> pendingLibraries;
			std::set<std::string> currentReferences;

		public:
			LoadLibraryResult Load(const std::string& typeLibraryFileName)
			{
				LoadLibraryResult result;
				result.PrimaryLibrary = ImportTypeLibrary(typeLibraryFileName);
				while (!pendingLibraries.empty())
				{
					std::string fileName = *pendingLibraries.begin();
					pendingLibraries.erase(pendingLibraries.begin());
					result.ReferencedLibraries.push_back(ImportTypeLibrary(fileName));
				}
				return result;
			}

			void Reference(Pointer<ITypeLib> reference) final
			{
				TypeLibrary typeLibrary{ reference };
				try
				{
					auto path = typeLibrary.QueryPath();
					currentReferences.insert(path);
					if (loadedLibraries.find(path) != loadedLibraries.end())
						return;
					pendingLibraries.insert(path);
				}
				catch (const std::exception& exception)
				{
					std::cout << "Unable to reference type library: " << typeLibrary.GetName() << std::endl;
					std::cerr << exception.what() << std::endl;
				}
			}

		private:
			static std::string GetTitle(const std::string& fileName)
			{
				auto lastSlash = fileName.rfind('\\');
				auto lastDot = fileName.rfind('.');
				if (lastSlash == std::string::npos && lastDot == std::string::npos)
					return fileName;
				if (lastDot == std::string::npos)
					return fileName.substr(lastSlash + 1);
				return fileName.substr(lastSlash + 1, lastDot - lastSlash - 1);
			}

			Library ImportTypeLibrary(const std::string& typeLibraryFileName)
			{
				std::cout << "Importing: " << typeLibraryFileName << std::endl;
				loadedLibraries.insert(typeLibraryFileName);
				currentReferences.clear();

				TypeLibrary typeLibrary(typeLibraryFileName);
				Library library;
				library.Name = typeLibrary.GetName();
				library.HeaderFileName = GetTitle(typeLibraryFileName) + ".h";
				library.Identifiers.push_back({ "LIBID_" + typeLibrary.GetName(), typeLibrary.GetId() });

				auto count = typeLibrary.GetTypeInfoCount();
				for (auto index = 0u; index < count; ++index)
					LoadType(typeLibrary, index, library);

				RecordSorter::SortRecords(library.Records);

				for (auto& reference : currentReferences)
					library.References.push_back(GetTitle(reference) + ".h");

				return library;
			}

			static void LoadType(TypeLibrary& typeLibrary, UINT index, Library& library)
			{
				TypeInfo typeInfo{ typeLibrary.GetTypeInfo(index) };
				switch (typeInfo.GetTypeKind())
				{
				case TKIND_INTERFACE:
				case TKIND_DISPATCH:
				{
					auto value = typeInfo.ToInterface();
					library.Interfaces.push_back(value);
					library.Identifiers.push_back({ value.Prefix + value.Name, value.Iid });
					break;
				}
				case TKIND_COCLASS:
				{
					auto value = typeInfo.ToCoclass();
					library.Coclasses.push_back(value);
					library.Identifiers.push_back({ "CLSID_" + value.Name, value.Clsid });
					break;
				}
				case TKIND_ALIAS:
					library.Aliases.push_back(typeInfo.ToAlias());
					break;

				case TKIND_ENUM:
					library.Enums.push_back(typeInfo.ToEnum());
					break;

				case TKIND_RECORD:
					library.Records.push_back(typeInfo.ToRecord());
					break;
				}
			}
		};
	}
}
