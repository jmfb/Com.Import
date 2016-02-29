#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <vector>
#include <iostream>
#include <exception>
#include <stdexcept>
#include "Com/Com.h"

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
using namespace Com::Import;

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

	std::cout << "#pragma once" << std::endl;
	//TODO: cross referenced libraries

	std::cout << "#pragma pack(push, 8)" << std::endl
		<< std::endl
		<< "namespace " << library.Name << std::endl
		<< "{" << std::endl;

	std::cout << "//Enumerations" << std::endl;
	for (auto value : library.Enums)
	{
		std::cout << "enum " << value.Name << std::endl
			<< "{" << std::endl;
		for (auto v : value.Values)
		{
			std::cout << "    " << v.Name << " = " << v.Value << "," << std::endl;
		}
		std::cout << "};" << std::endl;
	}

	std::cout << "//Forward declarations" << std::endl;
	for (auto value : library.Interfaces)
	{
		//
	}

	std::cout << "//Aliases" << std::endl;
	for (auto value : library.Aliases)
	{
		std::cout << "using " << value.NewName << " = " << value.OldName << ";" << std::endl;
	}

	//TODO: Sort records
	std::cout << "//Records" << std::endl;
	for (auto value : library.Records)
	{
		std::cout << "#pragma pack(push, " << value.Alignment << ")" << std::endl
			<< "struct " << value.Name << std::endl
			<< "{" << std::endl;
		for (auto m : value.Members)
		{
			std::cout << "    " << m.Name << ";" << std::endl;
		}
		std::cout << "};" << std::endl;
	}

	std::cout << "//Interfaces" << std::endl;
	for (auto value : library.Interfaces)
	{
		std::cout << "class __declspec(uuid(...)) " << value.Name << " : public " << value.Base << std::endl
			<< "{" << std::endl
			<< "public:" << std::endl;
		for (auto f : value.Functions)
		{
			std::cout << "    " << f.Name << "();" << std::endl;
		}
		std::cout << "};" << std::endl;
	}

	std::cout << "//Identifiers" << std::endl;
	for (auto value : library.Identifiers)
	{
		std::cout << "extern const ::GUID " << value.Name << " = {...};" << std::endl;
	}

	std::cout << "//Coclasses" << std::endl;
	for (auto value : library.Coclasses)
	{
		std::cout << "// coclass " << value.Name << std::endl;
		for (auto i : value.Interfaces)
			std::cout << "//   " << i << std::endl;
	}

	std::cout << "}" << std::endl;
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
