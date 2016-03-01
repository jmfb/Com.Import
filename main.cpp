#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <vector>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <fstream>
#include "Com/Com.h"

//#import "C:\Program Files (x86)\Common Files\System\ado\msado21.tlb" raw_interfaces_only named_guids
//#import "C:\Windows\SysWOW64\msvbvm60.dll" raw_interfaces_only named_guids
//#import "C:\Temp\DSCommon.tlb" raw_interfaces_only named_guids

#include "DataTypes.h"

void DisplayHelp()
{
	std::cout << "C++ COM Import Tool" << std::endl
		<< "Example usage:" << std::endl
		<< "    Com.Import.exe example.tlb" << std::endl
		<< std::endl;
}

#include "TypeLibrary.h"
#include "TypeInfo.h"
#include "CodeGenerator.h"
using namespace Com::Import;

std::string GetTitle(const std::string& fileName)
{
	auto lastSlash = fileName.rfind('\\');
	auto lastDot = fileName.rfind('.');
	if (lastSlash == std::string::npos && lastDot == std::string::npos)
		return fileName;
	if (lastDot == std::string::npos)
		return fileName.substr(lastSlash + 1);
	return fileName.substr(lastSlash + 1, lastDot - lastSlash - 1);
}

void ImportTypeLibrary(const std::string& typeLibraryFileName)
{
	Library library;

	TypeLibrary typeLibrary(typeLibraryFileName);
	library.Name = typeLibrary.GetName();
	library.Identifiers.push_back({ "LIBID_" + typeLibrary.GetName(), typeLibrary.GetId() });

	auto count = typeLibrary.GetTypeInfoCount();
	for (auto index = 0u; index < count; ++index)
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

		case TKIND_UNION:
			//Do not import unions
			break;

		case TKIND_MODULE:
			//Do not import modules
			break;

		default:
			throw std::runtime_error("Unhandled TYPEKIND: " + std::to_string(typeInfo.GetTypeKind()));
		}
	}

	//TODO: sort records by usage of each other

	auto headerFileName = GetTitle(typeLibraryFileName) + ".h";
	std::cout << "Generating header file: " << headerFileName << std::endl;
	std::ofstream out(headerFileName.c_str());
	CodeGenerator generator(out);
	generator.Write(library);
}

int main(int argc, char** argv)
{
	try
	{
		if (argc != 2)
			DisplayHelp();
		else
			ImportTypeLibrary(argv[1]);
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << std::endl;
		return -1;
	}
	return 0;
}
