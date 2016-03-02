#define _CRT_SECURE_NO_WARNINGS
#include "LibraryLoader.h"
#include "CodeGenerator.h"
#include <iostream>

void DisplayHelp()
{
	std::cout << "C++ COM Import Tool" << std::endl
		<< "Example usage:" << std::endl
		<< "    Com.Import.exe example.tlb" << std::endl
		<< std::endl;
}

void LoadTypeLibrary(const std::string& fileName)
{
	Com::Import::LibraryLoader loader;
	auto result = loader.Load(fileName);
	Com::Import::CodeGenerator::Generate(result);
}

int main(int argc, char** argv)
{
	try
	{
		if (argc != 2)
			DisplayHelp();
		else
			LoadTypeLibrary(argv[1]);
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << std::endl;
		return -1;
	}
	return 0;
}
