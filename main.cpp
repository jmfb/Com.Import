#include "LibraryLoader.h"
#include "CodeGenerator.h"
#include <iostream>

void DisplayHelp()
{
	std::cout << "C++ COM Import Tool" << std::endl
		<< "Example usages:" << std::endl
		<< "    Com.Import.exe example.tlb" << std::endl
		<< "    - This will generate import headers for this library and all cross-referenced libraries." << std::endl
		<< std::endl
		<< "    Com.Import.exe example.tlb /implement" << std::endl
		<< "    - This will generate the dll files for implementing this library and the import headers" << std::endl
		<< "      for all cross-referenced libraries." << std::endl
		<< std::endl;
}

void LoadTypeLibrary(const std::string& fileName, bool implement = false)
{
	Com::Import::LibraryLoader loader;
	auto result = loader.Load(fileName);
	Com::Import::CodeGenerator::Generate(result, implement);
}

int main(int argc, char** argv)
{
	try
	{
		switch (argc)
		{
		case 2:
			LoadTypeLibrary(argv[1]);
			break;
		case 3:
			LoadTypeLibrary(argv[1], std::strcmp(argv[2], "/implement") == 0);
			break;
		default:
			DisplayHelp();
			break;
		}
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << std::endl;
		return -1;
	}
	return 0;
}
