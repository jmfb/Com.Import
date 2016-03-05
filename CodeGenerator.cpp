#include "CodeGenerator.h"
#include <iomanip>
#include <type_traits>
#include <locale>
#include <codecvt>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <map>
#include <set>
#include "GuidFormatter.h"
#include "HexFormatter.h"
#include "TypeFormatter.h"
#include "EnumFormatter.h"
#include "AliasFormatter.h"
#include "InterfaceFormatter.h"
#include "RecordFormatter.h"
#include "IdentifierFormatter.h"
#include "FunctionFormatter.h"
#include "ParameterFormatter.h"

namespace Com
{
	namespace Import
	{
		CodeGenerator::CodeGenerator(std::ostream& out, bool implement)
			: out(out), implement(implement)
		{
		}

		void CodeGenerator::Generate(const LoadLibraryResult& result, bool implement)
		{
			Generate(result.PrimaryLibrary, implement);
			for (auto& reference : result.ReferencedLibraries)
				Generate(reference, false);
			if (!implement)
				return;
			GenerateSolution(result);
			GenerateProject(result);
			GenerateProjectFilters(result);
		}

		void CodeGenerator::Generate(const Library& library, bool implement)
		{
			auto fileName = library.OutputName + ".h";
			std::cout << "Generating header: " << fileName << std::endl;
			CodeGenerator{ std::ofstream{ fileName.c_str() }, implement }.Write(library);
			if (!implement)
				return;
			GenerateMain(library);
			GenerateCoclasses(library);
			GenerateDef(library);
			GenerateResources(library);
			GeneratePackages();
			GenerateManifest(library);
		}

		void CodeGenerator::GenerateMain(const Library& library)
		{
			std::cout << "Generating source: main.cpp" << std::endl;
			std::ofstream out{ "main.cpp" };
			out << "#include \"Com/Com.h\"" << std::endl;
			for (auto& coclass : library.Coclasses)
				out << "#include \"" << coclass.Name << ".h\"" << std::endl;
			out << std::endl
				<< "extern \"C\" BOOL __stdcall DllMain(HINSTANCE instance, DWORD reason, void* reserved)" << std::endl
				<< "{" << std::endl
				<< "	if (reason == DLL_PROCESS_ATTACH)" << std::endl
				<< "		Com::Module::GetInstance().Initialize(instance);" << std::endl
				<< "	return TRUE;" << std::endl
				<< "}" << std::endl
				<< std::endl
				<< "HRESULT __stdcall DllCanUnloadNow()" << std::endl
				<< "{" << std::endl
				<< "	return Com::Module::GetInstance().CanUnload() ? S_OK : S_FALSE;" << std::endl
				<< "}" << std::endl
				<< std::endl
				<< "HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppvObject)" << std::endl
				<< "{" << std::endl
				<< "	return Com::ObjectList<" << std::endl;
			auto first = true;
			for (auto& coclass : library.Coclasses)
			{
				if (!first)
					out << "," << std::endl;
				first = false;
				out << "		" << library.Name << "::" << coclass.Name;
			}
			out << std::endl
				<< "	>::Create(rclsid, riid, ppvObject);" << std::endl
				<< "}" << std::endl;
		}

		void CodeGenerator::GenerateCoclasses(const Library& library)
		{
			for (auto& coclass : library.Coclasses)
			{
				GenerateCoclassHeader(library, coclass);
				GenerateCoclassSource(library, coclass);
			}
		}

		void CodeGenerator::GenerateCoclassHeader(const Library& library, const Coclass& coclass)
		{
			auto fileName = coclass.Name + ".h";
			std::cout << "Generating header: " << fileName << std::endl;
			std::ofstream out{ fileName.c_str() };
			out << "#pragma once" << std::endl
				<< "#include \"" << library.OutputName << ".h\"" << std::endl
				<< std::endl
				<< "namespace " << library.Name << std::endl
				<< "{" << std::endl
				<< "	class " << coclass.Name << " : public " << coclass.Name << "Coclass<" << coclass.Name << ">" << std::endl
				<< "	{" << std::endl
				<< "	public:" << std::endl;
			for (auto& iface : coclass.Interfaces)
				CodeGenerator(out, true).WriteNativeFunctions(
					iface,
					FunctionDefinition::Prototype,
					coclass.Name);
			out << "	};" << std::endl
				<< "}" << std::endl;
		}

		void CodeGenerator::GenerateCoclassSource(const Library& library, const Coclass& coclass)
		{
			auto fileName = coclass.Name + ".cpp";
			std::cout << "Generating source: " << fileName << std::endl;
			std::ofstream out{ fileName.c_str() };
			out << "#include \"" << coclass.Name << ".h\"" << std::endl
				<< std::endl
				<< "namespace " << library.Name << std::endl
				<< "{" << std::endl;
			for (auto& iface : coclass.Interfaces)
				CodeGenerator(out, true).WriteNativeFunctions(
					iface,
					FunctionDefinition::Definition,
					coclass.Name);
			out << "}" << std::endl;
		}

		void CodeGenerator::GenerateDef(const Library& library)
		{
			auto fileName = library.Name + ".def";
			std::cout << "Generate module definition: " << fileName << std::endl;
			std::ofstream out{ fileName.c_str() };
			out << "LIBRARY \"" << library.OutputName << "\"" << std::endl
				<< "EXPORTS" << std::endl
				<< "	DllCanUnloadNow private" << std::endl
				<< "	DllGetClassObject private" << std::endl;
		}

		void CodeGenerator::GenerateResources(const Library& library)
		{
			auto rcFileName = library.Name + ".rc";
			auto headerFileName = "resource.h";
			std::cout << "Generate resources: " << rcFileName << "/" << headerFileName << std::endl;
			std::ofstream out{ rcFileName.c_str() };

			auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			auto year = std::localtime(&time)->tm_year + 1900;

			out << "// Microsoft Visual C++ generated resource script." << std::endl
				<< "//" << std::endl
				<< "#include \"resource.h\"" << std::endl
				<< std::endl
				<< "#define APSTUDIO_READONLY_SYMBOLS" << std::endl
				<< "/////////////////////////////////////////////////////////////////////////////" << std::endl
				<< "//" << std::endl
				<< "// Generated from the TEXTINCLUDE 2 resource." << std::endl
				<< "//" << std::endl
				<< "#include \"winres.h\"" << std::endl
				<< std::endl
				<< "/////////////////////////////////////////////////////////////////////////////" << std::endl
				<< "#undef APSTUDIO_READONLY_SYMBOLS" << std::endl
				<< std::endl
				<< "/////////////////////////////////////////////////////////////////////////////" << std::endl
				<< "// English (United States) resources" << std::endl
				<< std::endl
				<< "#if !defined(AFX_RESOURCE_DLL) || defined(AFX_TARG_ENU)" << std::endl
				<< "LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US" << std::endl
				<< std::endl
				<< "#ifdef APSTUDIO_INVOKED" << std::endl
				<< "/////////////////////////////////////////////////////////////////////////////" << std::endl
				<< "//" << std::endl
				<< "// TEXTINCLUDE" << std::endl
				<< "//" << std::endl
				<< std::endl
				<< "1 TEXTINCLUDE " << std::endl
				<< "BEGIN" << std::endl
				<< "    \"resource.h\\0\"" << std::endl
				<< "END" << std::endl
				<< std::endl
				<< "2 TEXTINCLUDE " << std::endl
				<< "BEGIN" << std::endl
				<< "    \"#include \"\"winres.h\"\"\\r\\n\"" << std::endl
				<< "    \"\\0\"" << std::endl
				<< "END" << std::endl
				<< std::endl
				<< "3 TEXTINCLUDE " << std::endl
				<< "BEGIN" << std::endl
				<< "    \"1 TYPELIB \"\"" << library.OutputName << ".tlb\"\"\\r\\n\"" << std::endl
				<< "    \"\\0\"" << std::endl
				<< "END" << std::endl
				<< std::endl
				<< "#endif    // APSTUDIO_INVOKED" << std::endl
				<< std::endl
				<< std::endl
				<< "/////////////////////////////////////////////////////////////////////////////" << std::endl
				<< "//" << std::endl
				<< "// Version" << std::endl
				<< "//" << std::endl
				<< std::endl
				<< "VS_VERSION_INFO VERSIONINFO" << std::endl
				<< " FILEVERSION 1,0,0,1" << std::endl
				<< " PRODUCTVERSION 1,0,0,1" << std::endl
				<< " FILEFLAGSMASK 0x3fL" << std::endl
				<< "#ifdef _DEBUG" << std::endl
				<< " FILEFLAGS 0x1L" << std::endl
				<< "#else" << std::endl
				<< " FILEFLAGS 0x0L" << std::endl
				<< "#endif" << std::endl
				<< " FILEOS 0x40004L" << std::endl
				<< " FILETYPE 0x2L" << std::endl
				<< " FILESUBTYPE 0x0L" << std::endl
				<< "BEGIN" << std::endl
				<< "    BLOCK \"StringFileInfo\"" << std::endl
				<< "    BEGIN" << std::endl
				<< "        BLOCK \"040904b0\"" << std::endl
				<< "        BEGIN" << std::endl
				<< "            VALUE \"CompanyName\", \"TODO\"" << std::endl
				<< "            VALUE \"FileDescription\", \"" << library.Name << "\"" << std::endl
				<< "            VALUE \"FileVersion\", \"1.0.0.0\"" << std::endl
				<< "            VALUE \"InternalName\", \"" << library.OutputName << ".dll\"" << std::endl
				<< "            VALUE \"LegalCopyright\", \"Copyright(C) " << year << "\"" << std::endl
				<< "            VALUE \"OriginalFilename\", \"" << library.OutputName << ".dll\"" << std::endl
				<< "            VALUE \"ProductName\", \"" << library.Name << "\"" << std::endl
				<< "            VALUE \"ProductVersion\", \"1.0.0.0\"" << std::endl
				<< "        END" << std::endl
				<< "    END" << std::endl
				<< "    BLOCK \"VarFileInfo\"" << std::endl
				<< "    BEGIN" << std::endl
				<< "        VALUE \"Translation\", 0x409, 1200" << std::endl
				<< "    END" << std::endl
				<< "END" << std::endl
				<< std::endl
				<< "#endif    // English (United States) resources" << std::endl
				<< "/////////////////////////////////////////////////////////////////////////////" << std::endl
				<< std::endl
				<< std::endl
				<< std::endl
				<< "#ifndef APSTUDIO_INVOKED" << std::endl
				<< "/////////////////////////////////////////////////////////////////////////////" << std::endl
				<< "//" << std::endl
				<< "// Generated from the TEXTINCLUDE 3 resource." << std::endl
				<< "//" << std::endl
				<< "1 TYPELIB \"" << library.OutputName << ".tlb\"" << std::endl
				<< std::endl
				<< "/////////////////////////////////////////////////////////////////////////////" << std::endl
				<< "#endif    // not APSTUDIO_INVOKED" << std::endl
				<< std::endl
				<< std::endl;

			std::ofstream{ headerFileName }
				<< "//{{NO_DEPENDENCIES}}" << std::endl
				<< "// Microsoft Visual C++ generated include file." << std::endl
				<< "// Used by " << library.Name << ".rc" << std::endl
				<< std::endl
				<< "// Next default values for new objects" << std::endl
				<< "// " << std::endl
				<< "#ifdef APSTUDIO_INVOKED" << std::endl
				<< "#ifndef APSTUDIO_READONLY_SYMBOLS" << std::endl
				<< "#define _APS_NEXT_RESOURCE_VALUE        101" << std::endl
				<< "#define _APS_NEXT_COMMAND_VALUE         40001" << std::endl
				<< "#define _APS_NEXT_CONTROL_VALUE         1001" << std::endl
				<< "#define _APS_NEXT_SYMED_VALUE           101" << std::endl
				<< "#endif" << std::endl
				<< "#endif" << std::endl;
		}

		void CodeGenerator::GeneratePackages()
		{
			auto fileName = "packages.config";
			std::cout << "Generating packages: " << fileName << std::endl;
			std::ofstream{ fileName }
				<< "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl
				<< "<packages>" << std::endl
				<< "	<package id=\"Jmfb.Com\" version=\"1.0.5\" targetFramework=\"native\" />" << std::endl
				<< "</packages>" << std::endl;
		}

		void CodeGenerator::GenerateManifest(const Library& library)
		{
			auto fileName = library.OutputName + ".manifest";
			std::cout << "Generating manifest: " << fileName << std::endl;
			std::ofstream out{ fileName.c_str() };
			out << "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>" << std::endl
				<< "<assembly" << std::endl
				<< "	xmlns=\"urn:schemas-microsoft-com:asm.v1\"" << std::endl
				<< "	manifestVersion=\"1.0\">" << std::endl
				<< "	<assemblyIdentity" << std::endl
				<< "		type=\"win32\"" << std::endl
				<< "		name=\"" << library.OutputName << "\"" << std::endl
				<< "		version=\"1.0.0.0\" />" << std::endl
				<< "	<file name=\"" << library.OutputName << ".dll\">" << std::endl;
			for (auto& coclass : library.Coclasses)
			{
				out << "		<comClass" << std::endl
					<< "			clsid=\"{" << Format(coclass.Clsid, GuidFormat::AsString) << "}\"" << std::endl
					<< "			threadingModel=\"Free\" />" << std::endl;
			}
			out << "		<typelib" << std::endl
				<< "			tlbid=\"{" << Format(library.Libid, GuidFormat::AsString) << "}\"" << std::endl
				<< "			version=\"" << library.MajorVersion << "." << library.MinorVersion << "\"" << std::endl
				<< "			helpdir=\"\" />" << std::endl
				<< "	</file>" << std::endl;
			for (auto& iface : library.Interfaces)
			{
				out << "	<comInterfaceExternalProxyStub" << std::endl
					<< "		name=\"" << iface.Name << "\"" << std::endl
					<< "		iid=\"{" << Format(iface.Iid, GuidFormat::AsString) << "}\"" << std::endl
					<< "		proxyStubClsid32=\"{00020424-0000-0000-C000-000000000046}\"" << std::endl
					<< "		baseInterface=\"{" << Format(iface.BaseIid, GuidFormat::AsString) << "}\"" << std::endl
					<< "		tlbid=\"{" << Format(library.Libid, GuidFormat::AsString) << "}\" />" << std::endl;
			}
			out << "</assembly>" << std::endl;
		}

		void CodeGenerator::GenerateSolution(const LoadLibraryResult& result)
		{
			auto fileName = result.PrimaryLibrary.Name + ".sln";
			std::ofstream{ fileName.c_str() }
				<< "Microsoft Visual Studio Solution File, Format Version 12.00" << std::endl
				<< "# Visual Studio 14" << std::endl
				<< "VisualStudioVersion = 14.0.24720.0" << std::endl
				<< "MinimumVisualStudioVersion = 10.0.40219.1" << std::endl
				<< "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" << result.PrimaryLibrary.Name
					<< "\", \"" << result.PrimaryLibrary.Name << ".vcxproj\", \"{" << Format(result.PrimaryLibrary.Libid, GuidFormat::AsString) << "}\"" << std::endl
				<< "EndProject" << std::endl
				<< "Global" << std::endl
				<< "	GlobalSection(SolutionConfigurationPlatforms) = preSolution" << std::endl
				<< "		Debug|x86 = Debug|x86" << std::endl
				<< "		Release|x86 = Release|x86" << std::endl
				<< "	EndGlobalSection" << std::endl
				<< "	GlobalSection(ProjectConfigurationPlatforms) = postSolution" << std::endl
				<< "		{" << Format(result.PrimaryLibrary.Libid, GuidFormat::AsString) << "}.Debug|x86.ActiveCfg = Debug|Win32" << std::endl
				<< "		{" << Format(result.PrimaryLibrary.Libid, GuidFormat::AsString) << "}.Debug|x86.Build.0 = Debug|Win32" << std::endl
				<< "		{" << Format(result.PrimaryLibrary.Libid, GuidFormat::AsString) << "}.Release|x86.ActiveCfg = Release|Win32" << std::endl
				<< "		{" << Format(result.PrimaryLibrary.Libid, GuidFormat::AsString) << "}.Release|x86.Build.0 = Release|Win32" << std::endl
				<< "	EndGlobalSection" << std::endl
				<< "	GlobalSection(SolutionProperties) = preSolution" << std::endl
				<< "		HideSolutionNode = FALSE" << std::endl
				<< "	EndGlobalSection" << std::endl
				<< "EndGlobal" << std::endl;
		}

		void CodeGenerator::GenerateProject(const LoadLibraryResult& result)
		{
			auto fileName = result.PrimaryLibrary.Name + ".vcxproj";
			std::cout << "Generating project file: " << fileName << std::endl;
			std::ofstream out{ fileName.c_str() };
			out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl
				<< "<Project DefaultTargets=\"Build\" ToolsVersion=\"14.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << std::endl
				<< "  <ItemGroup Label=\"ProjectConfigurations\">" << std::endl
				<< "    <ProjectConfiguration Include=\"Debug|Win32\">" << std::endl
				<< "      <Configuration>Debug</Configuration>" << std::endl
				<< "      <Platform>Win32</Platform>" << std::endl
				<< "    </ProjectConfiguration>" << std::endl
				<< "    <ProjectConfiguration Include=\"Release|Win32\">" << std::endl
				<< "      <Configuration>Release</Configuration>" << std::endl
				<< "      <Platform>Win32</Platform>" << std::endl
				<< "    </ProjectConfiguration>" << std::endl
				<< "  </ItemGroup>" << std::endl
				<< "  <PropertyGroup Label=\"Globals\">" << std::endl
				<< "    <ProjectGuid>{" << Format(result.PrimaryLibrary.Libid, GuidFormat::AsString) << "}</ProjectGuid>" << std::endl
				<< "    <Keyword>Win32Proj</Keyword>" << std::endl
				<< "    <RootNamespace>" << result.PrimaryLibrary.Name << "</RootNamespace>" << std::endl
				<< "    <WindowsTargetPlatformVersion>8.1</WindowsTargetPlatformVersion>" << std::endl
				<< "  </PropertyGroup>" << std::endl
				<< "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />" << std::endl
				<< "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\" Label=\"Configuration\">" << std::endl
				<< "    <ConfigurationType>DynamicLibrary</ConfigurationType>" << std::endl
				<< "    <UseDebugLibraries>true</UseDebugLibraries>" << std::endl
				<< "    <PlatformToolset>v140</PlatformToolset>" << std::endl
				<< "    <CharacterSet>MultiByte</CharacterSet>" << std::endl
				<< "  </PropertyGroup>" << std::endl
				<< "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|Win32'\" Label=\"Configuration\">" << std::endl
				<< "    <ConfigurationType>DynamicLibrary</ConfigurationType>" << std::endl
				<< "    <UseDebugLibraries>false</UseDebugLibraries>" << std::endl
				<< "    <PlatformToolset>v140</PlatformToolset>" << std::endl
				<< "    <WholeProgramOptimization>true</WholeProgramOptimization>" << std::endl
				<< "    <CharacterSet>MultiByte</CharacterSet>" << std::endl
				<< "  </PropertyGroup>" << std::endl
				<< "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />" << std::endl
				<< "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">" << std::endl
				<< "    <LinkIncremental>true</LinkIncremental>" << std::endl
				<< "    <TargetName>" << result.PrimaryLibrary.OutputName << "</TargetName>" << std::endl
				<< "    <GenerateManifest>true</GenerateManifest>" << std::endl
				<< "  </PropertyGroup>" << std::endl
				<< "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|Win32'\">" << std::endl
				<< "    <LinkIncremental>false</LinkIncremental>" << std::endl
				<< "    <TargetName>" << result.PrimaryLibrary.OutputName << "</TargetName>" << std::endl
				<< "    <GenerateManifest>false</GenerateManifest>" << std::endl
				<< "  </PropertyGroup>" << std::endl
				<< "  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">" << std::endl
				<< "    <ClCompile>" << std::endl
				<< "      <WarningLevel>Level3</WarningLevel>" << std::endl
				<< "      <Optimization>Disabled</Optimization>" << std::endl
				<< "      <PreprocessorDefinitions>WIN32;_DEBUG;_WINDOWS;_USRDLL;_CRT_SECURE_NO_WARNINGS;%(PreprocessorDefinitions)</PreprocessorDefinitions>" << std::endl
				<< "      <RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>" << std::endl
				<< "    </ClCompile>" << std::endl
				<< "    <Link>" << std::endl
				<< "      <SubSystem>Windows</SubSystem>" << std::endl
				<< "      <GenerateDebugInformation>true</GenerateDebugInformation>" << std::endl
				<< "      <ModuleDefinitionFile>" << result.PrimaryLibrary.Name << ".def</ModuleDefinitionFile>" << std::endl
				<< "      <EnableUAC>false</EnableUAC>" << std::endl
				<< "    </Link>" << std::endl
				<< "    <Manifest>" << std::endl
				<< "      <AdditionalManifestFiles>" << result.PrimaryLibrary.OutputName << ".manifest</AdditionalManifestFiles>" << std::endl
				<< "    </Manifest>" << std::endl
				<< "  </ItemDefinitionGroup>" << std::endl
				<< "  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|Win32'\">" << std::endl
				<< "    <ClCompile>" << std::endl
				<< "      <WarningLevel>Level3</WarningLevel>" << std::endl
				<< "      <Optimization>MaxSpeed</Optimization>" << std::endl
				<< "      <FunctionLevelLinking>true</FunctionLevelLinking>" << std::endl
				<< "      <IntrinsicFunctions>true</IntrinsicFunctions>" << std::endl
				<< "      <PreprocessorDefinitions>WIN32;NDEBUG;_WINDOWS;_USRDLL;_CRT_SECURE_NO_WARNINGS;%(PreprocessorDefinitions)</PreprocessorDefinitions>" << std::endl
				<< "      <RuntimeLibrary>MultiThreaded</RuntimeLibrary>" << std::endl
				<< "    </ClCompile>" << std::endl
				<< "    <Link>" << std::endl
				<< "      <SubSystem>Windows</SubSystem>" << std::endl
				<< "      <EnableCOMDATFolding>true</EnableCOMDATFolding>" << std::endl
				<< "      <OptimizeReferences>true</OptimizeReferences>" << std::endl
				<< "      <ModuleDefinitionFile>" << result.PrimaryLibrary.Name << ".def</ModuleDefinitionFile>" << std::endl
				<< "    </Link>" << std::endl
				<< "    <Manifest>" << std::endl
				<< "      <AdditionalManifestFiles>" << result.PrimaryLibrary.OutputName << ".manifest</AdditionalManifestFiles>" << std::endl
				<< "    </Manifest>" << std::endl
				<< "  </ItemDefinitionGroup>" << std::endl
				<< "  <ItemGroup>" << std::endl
				<< "    <ClInclude Include=\"" << result.PrimaryLibrary.OutputName << ".h\" />" << std::endl;
			for (auto& library : result.ReferencedLibraries)
				out << "    <ClInclude Include=\"" << library.OutputName << ".h\" />" << std::endl;
			for (auto& coclass : result.PrimaryLibrary.Coclasses)
				out << "    <ClInclude Include=\"" << coclass.Name << ".h\" />" << std::endl;
			out << "    <ClInclude Include=\"resource.h\" />" << std::endl
				<< "  </ItemGroup>" << std::endl
				<< "  <ItemGroup>" << std::endl
				<< "    <Manifest Include=\"" << result.PrimaryLibrary.OutputName << ".manifest\" />" << std::endl
				<< "  </ItemGroup>" << std::endl
				<< "  <ItemGroup>" << std::endl
				<< "    <None Include=\"" << result.PrimaryLibrary.OutputName << ".tlb\" />" << std::endl
				<< "    <None Include=\"" << result.PrimaryLibrary.Name << ".def\" />" << std::endl
				<< "    <None Include=\"packages.config\" />" << std::endl
				<< "  </ItemGroup>" << std::endl
				<< "  <ItemGroup>" << std::endl;
			for (auto& coclass : result.PrimaryLibrary.Coclasses)
				out << "    <ClCompile Include=\"" << coclass.Name << ".cpp\" />" << std::endl;
			out << "    <ClCompile Include=\"main.cpp\" />" << std::endl
				<< "  </ItemGroup>" << std::endl
				<< "  <ItemGroup>" << std::endl
				<< "    <ResourceCompile Include=\"" << result.PrimaryLibrary.Name << ".rc\" />" << std::endl
				<< "  </ItemGroup>" << std::endl
				<< "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />" << std::endl
				<< "</Project>" << std::endl;
		}

		void CodeGenerator::GenerateProjectFilters(const LoadLibraryResult& result)
		{
			auto fileName = result.PrimaryLibrary.Name + ".vcxproj.filters";
			std::cout << "Generating project filters: " << fileName << std::endl;
			std::ofstream out{ fileName.c_str() };
			out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl
				<< "<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << std::endl
				<< "  <ItemGroup>" << std::endl
				<< "    <ClInclude Include=\"resource.h\">" << std::endl
				<< "      <Filter>Resources</Filter>" << std::endl
				<< "    </ClInclude>" << std::endl
				<< "    <ClInclude Include=\"" << result.PrimaryLibrary.OutputName << ".h\">" << std::endl
				<< "      <Filter>Imports</Filter>" << std::endl
				<< "    </ClInclude>" << std::endl;
			for (auto& library : result.ReferencedLibraries)
				out << "    <ClInclude Include=\"" << library.OutputName << ".h\">" << std::endl
					<< "      <Filter>Imports</Filter>" << std::endl
					<< "    </ClInclude>" << std::endl;
			for (auto& coclass : result.PrimaryLibrary.Coclasses)
				out << "    <ClInclude Include=\"" << coclass.Name << ".h\">" << std::endl
					<< "      <Filter>Classes</Filter>" << std::endl
					<< "    </ClInclude>" << std::endl;
			out << "  </ItemGroup>" << std::endl
				<< "  <ItemGroup>" << std::endl
				<< "    <None Include=\"packages.config\" />" << std::endl
				<< "    <None Include=\"" << result.PrimaryLibrary.OutputName << ".tlb\">" << std::endl
				<< "      <Filter>Resources</Filter>" << std::endl
				<< "    </None>" << std::endl
				<< "    <None Include=\"" << result.PrimaryLibrary.Name << ".def\">" << std::endl
				<< "      <Filter>Resources</Filter>" << std::endl
				<< "    </None>" << std::endl
				<< "  </ItemGroup>" << std::endl
				<< "  <ItemGroup>" << std::endl
				<< "    <ClCompile Include=\"main.cpp\" />" << std::endl;
			for (auto& coclass : result.PrimaryLibrary.Coclasses)
				out << "    <ClCompile Include=\"" << coclass.Name << ".cpp\">" << std::endl
					<< "      <Filter>Classes</Filter>" << std::endl
					<< "    </ClCompile>" << std::endl;
			out << "  </ItemGroup>" << std::endl
				<< "  <ItemGroup>" << std::endl
				<< "    <Filter Include=\"Classes\">" << std::endl
				<< "      <UniqueIdentifier>{1a045a97-b4c4-44aa-8d30-334dddbc898d}</UniqueIdentifier>" << std::endl
				<< "    </Filter>" << std::endl
				<< "    <Filter Include=\"Imports\">" << std::endl
				<< "      <UniqueIdentifier>{2bb3859c-3647-4c7f-a525-60ce09efb0f9}</UniqueIdentifier>" << std::endl
				<< "    </Filter>" << std::endl
				<< "    <Filter Include=\"Resources\">" << std::endl
				<< "      <UniqueIdentifier>{d8988015-03d0-49f0-b3ac-49f515ce5289}</UniqueIdentifier>" << std::endl
				<< "    </Filter>" << std::endl
				<< "  </ItemGroup>" << std::endl
				<< "  <ItemGroup>" << std::endl
				<< "    <Manifest Include=\"" << result.PrimaryLibrary.OutputName << ".manifest\">" << std::endl
				<< "      <Filter>Resources</Filter>" << std::endl
				<< "    </Manifest>" << std::endl
				<< "  </ItemGroup>" << std::endl
				<< "  <ItemGroup>" << std::endl
				<< "    <ResourceCompile Include=\"" << result.PrimaryLibrary.Name << ".rc\">" << std::endl
				<< "      <Filter>Resources</Filter>" << std::endl
				<< "    </ResourceCompile>" << std::endl
				<< "  </ItemGroup>" << std::endl
				<< "</Project>" << std::endl;
		}

		void CodeGenerator::Write(const Library& library)
		{
			out << "#pragma once" << std::endl
				<< "#include \"Com/Com.h\"" << std::endl;
			for (auto& reference : library.References)
				out << "#include \"" << reference << "\"" << std::endl;
			out << "#pragma pack(push, 8)" << std::endl
				<< "namespace " << library.Name << std::endl
				<< "{" << std::endl;
			Write(library.Enums);
			ForwardDeclare(library.Interfaces);
			Write(library.Aliases);
			Write(library.Records);
			Write(library.Interfaces);
			Write(library.Identifiers);
			WriteWrappers(library.Interfaces);
			if (implement)
				Write(library.Coclasses);
			out << "}" << std::endl;
			WriteComTypeInfo(library.Name, library.Interfaces);
			out << "#pragma pack(pop)" << std::endl;
		}

		void CodeGenerator::Write(const std::vector<Enum>& enums)
		{
			for (auto& enumeration : enums)
				out << Format(enumeration);
		}

		void CodeGenerator::ForwardDeclare(const std::vector<Interface>& interfaces)
		{
			for (auto& iface : interfaces)
				ForwardDeclare(iface);
		}

		void CodeGenerator::ForwardDeclare(const Interface& iface)
		{
			out << Format(iface, InterfaceFormat::AsForwardDeclaration)
				<< "	template <typename Interface> class " << iface.Name << "PtrT;" << std::endl
				<< "	using " << iface.Name << "Ptr = " << iface.Name << "PtrT<" << iface.Name << ">;" << std::endl;
		}

		void CodeGenerator::Write(const std::vector<Alias>& aliases)
		{
			for (auto& alias : aliases)
				out << Format(alias);
		}

		void CodeGenerator::Write(const std::vector<Record>& records)
		{
			for (auto& record : records)
				out << Format(record);
		}

		void CodeGenerator::Write(const std::vector<Interface>& interfaces)
		{
			for (auto& iface : interfaces)
				out << Format(iface, InterfaceFormat::AsNative, implement ? "raw_" : "");
		}

		void CodeGenerator::Write(const std::vector<Identifier>& identifiers)
		{
			for (auto& identifier : identifiers)
				out << Format(identifier);
		}

		void CodeGenerator::Write(const std::vector<Coclass>& coclasses)
		{
			for (auto& coclass : coclasses)
				Write(coclass);
		}

		void CodeGenerator::Write(const Coclass& coclass)
		{
			for (auto& iface : coclass.Interfaces)
			{
				if (iface.IsConflicting)
				{
					out << "	class __declspec(uuid(\"" << Format(iface.Iid, GuidFormat::AsString) << "\")) "
						<< iface.Name << "_" << coclass.Name << " : public " << iface.Name << std::endl
						<< "	{" << std::endl
						<< "	public:" << std::endl;
					for (auto& function : iface.Functions)
					{
						if ((function.VtblOffset == 0 && function.IsDispatchOnly) || (function.VtblOffset >= iface.VtblOffset))
						{
							out << "		virtual ";
							out << Format(function.Retval, TypeFormat::AsNative);
							out << " __stdcall " << iface.Name << "_raw_" << function.Name << "(";
							auto first = true;
							for (auto& argument : function.ArgList)
							{
								if (!first)
									out << ", ";
								first = false;
								out << Format(argument.Type, TypeFormat::AsNative);
								out << " " << argument.Name;
							}
							out << ") = 0;" << std::endl;
							out << "		";
							out << Format(function.Retval, TypeFormat::AsNative);
							out << " __stdcall raw_" << function.Name << "(";
							first = true;
							for (auto& argument : function.ArgList)
							{
								if (!first)
									out << ", ";
								first = false;
								out << Format(argument.Type, TypeFormat::AsNative);
								out << " " << argument.Name;
							}
							out << ") final" << std::endl
								<< "		{" << std::endl
								<< "			return " << iface.Name << "_raw_" << function.Name << "(";
							first = true;
							for (auto& argument : function.ArgList)
							{
								if (!first)
									out << ", ";
								first = false;
								out << argument.Name;
							}
							out << ");" << std::endl
								<< "		}" << std::endl;
						}
					}
					out << "	};" << std::endl;
				}
			}
			out << "	template <typename Type>" << std::endl
				<< "	class " << coclass.Name << "Coclass : public Com::Object<Type, &CLSID_" << coclass.Name;
			for (auto& iface : coclass.Interfaces)
			{
				out << ", " << iface.Name;
				if (iface.IsConflicting)
					out << "_" << coclass.Name;
			}
			out << ">" << std::endl
				<< "	{" << std::endl
				<< "	public:" << std::endl;
			for (auto& iface : coclass.Interfaces)
				WriteNativeFunctions(
					iface,
					FunctionDefinition::Abstract,
					coclass.Name);
			for (auto& iface : coclass.Interfaces)
				WriteRawFunctions(iface);
			out << "	};" << std::endl;
		}

		void CodeGenerator::WriteNativeFunctions(
			const Interface& iface,
			FunctionDefinition definition,
			const std::string& className)
		{
			for (auto& function : iface.Functions)
			{
				if (function.VtblOffset >= iface.VtblOffset && function.Retval.TypeEnum == TypeEnum::Hresult)
				{
					out << "	";
					if (definition != FunctionDefinition::Definition)
						out << "	";
					if (definition == FunctionDefinition::Abstract)
						out << "virtual ";
					if (!function.ArgList.empty() && function.ArgList.back().Retval)
						out << Format(function.ArgList.back().Type, TypeFormat::AsWrapper);
					else
						out << "void";
					out << " ";
					if (definition == FunctionDefinition::Definition)
						out << className << "::";
					if (iface.IsConflicting)
						out << iface.Name << "_";
					out << function.Name << "(";
					auto first = true;
					for (auto& argument : function.ArgList)
					{
						if (argument.Retval)
							break;
						if (!first)
							out << ", ";
						first = false;
						out << Format(argument);
					}
					out << ")";
					switch (definition)
					{
					case FunctionDefinition::Abstract:
						out << " = 0;" << std::endl;
						break;
					case FunctionDefinition::Prototype:
						out << " final;" << std::endl;
						break;
					case FunctionDefinition::Definition:
						out << std::endl
							<< "	{" << std::endl
							<< "		throw Com::NotImplemented(__FUNCTION__);" << std::endl
							<< "	}" << std::endl
							<< std::endl;
						break;
					}
				}
			}
		}

		void CodeGenerator::WriteRawFunctions(const Interface& iface)
		{
			for (auto& function : iface.Functions)
			{
				if (function.VtblOffset >= iface.VtblOffset && function.Retval.TypeEnum == TypeEnum::Hresult)
				{
					out << "		HRESULT __stdcall ";
					if (iface.IsConflicting)
						out << iface.Name << "_";
					out << "raw_" << function.Name << "(";
					auto first = true;
					for (auto& argument : function.ArgList)
					{
						if (!first)
							out << ", ";
						first = false;
						out << Format(argument.Type, TypeFormat::AsNative);
						out << " " << argument.Name;
					}
					out << ") final" << std::endl
						<< "		{" << std::endl
						<< "			try" << std::endl
						<< "			{" << std::endl
						<< "				";
					if (!function.ArgList.empty() && function.ArgList.back().Retval)
					{
						auto& retval = function.ArgList.back();
						if (retval.Type.TypeEnum == TypeEnum::Int16)
							out << "Com::CheckPointer(" << retval.Name << ") = ";
						else
							out << "Com::Retval(" << retval.Name << ") = ";
					}
					if (iface.IsConflicting)
						out << iface.Name << "_";
					out << function.Name << "(";
					first = true;
					for (auto& argument : function.ArgList)
					{
						if (argument.Retval)
							break;
						if (!first)
							out << ", ";
						first = false;
						if (argument.Out)
						{
							if (argument.Type.TypeEnum == TypeEnum::Int16)
								out << "Com::CheckPointer(" << argument.Name << ")";
							else
								out << "Com::InOut(" << argument.Name << ")";
						}
						else
						{
							if (argument.Type.TypeEnum == TypeEnum::Int16)
								out << argument.Name;
							else
								out << "Com::In(" << argument.Name << ")";
						}
					}
					out << ");" << std::endl
						<< "			}" << std::endl
						<< "			catch (...)" << std::endl
						<< "			{" << std::endl
						<< "				return Com::HandleException();" << std::endl
						<< "			}" << std::endl
						<< "			return S_OK;" << std::endl
						<< "		}" << std::endl;
				}
			}
		}

		void CodeGenerator::WriteWrappers(const std::vector<Interface>& interfaces)
		{
			for (auto& iface : interfaces)
				WriteWrapper(iface);
			for (auto& iface : interfaces)
				WriteWrapperFunctions(iface);
		}

		void CodeGenerator::WriteWrapper(const Interface& iface)
		{
			out << "	template <typename Interface>" << std::endl
				<< "	class " << iface.Name << "PtrT : public " << GetWrapperBase(iface) << std::endl
				<< "	{" << std::endl
				<< "	public:" << std::endl
				<< "		" << iface.Name << "PtrT(Interface* value = nullptr);" << std::endl
				<< "		" << iface.Name << "PtrT<Interface>& operator=(Interface* value);" << std::endl
				<< "		operator " << iface.Name << "*() const;" << std::endl;
			for (auto& function : iface.Functions)
			{
				if ((function.VtblOffset == 0 && function.IsDispatchOnly) ||
					(function.VtblOffset >= iface.VtblOffset))
				{
					out << "		";
					if (!function.ArgList.empty() && function.ArgList.back().Retval)
						out << Format(function.ArgList.back().Type, TypeFormat::AsWrapper);
					else
						out << "void";
					out << " " << function.Name << "(";
					auto first = true;
					for (auto& argument : function.ArgList)
					{
						if (argument.Retval)
							break;
						if (!first)
							out << ", ";
						first = false;
						out << Format(argument);
					}
					out << ");" << std::endl;
				}
			}
			out << "	};" << std::endl;
		}

		void CodeGenerator::WriteComTypeInfo(const std::string& libraryName, const std::vector<Interface>& interfaces)
		{
			out << "namespace Com" << std::endl
				<< "{" << std::endl;
			for (auto& iface : interfaces)
				out << Format(iface, InterfaceFormat::AsTypeInfoSpecialization, "", libraryName);
			out << "}" << std::endl;
		}

		void CodeGenerator::WriteWrapperFunctions(const Interface& iface)
		{
			out << "	template <typename Interface>" << std::endl
				<< "	inline " << iface.Name << "PtrT<Interface>::" << iface.Name << "PtrT(Interface* value) : " << GetWrapperBase(iface) << "(value)" << std::endl
				<< "	{" << std::endl
				<< "	}" << std::endl
				<< "	template <typename Interface>" << std::endl
				<< "	inline " << iface.Name << "PtrT<Interface>& " << iface.Name << "PtrT<Interface>::operator=(Interface* value)" << std::endl
				<< "	{" << std::endl
				<< "		using Base = " << GetWrapperBase(iface) << ";" << std::endl
				<< "		Base::operator=(value);" << std::endl
				<< "		return *this;" << std::endl
				<< "	}" << std::endl
				<< "	template <typename Interface>" << std::endl
				<< "	inline " << iface.Name << "PtrT<Interface>::operator " << iface.Name << "*() const" << std::endl
				<< "	{" << std::endl
				<< "		return p;" << std::endl
				<< "	}" << std::endl;
			for (auto& function : iface.Functions)
			{
				if (function.VtblOffset == 0 && function.IsDispatchOnly)
					WriteWrapperDispatch(iface.Name, function);
				else if (function.VtblOffset >= iface.VtblOffset)
					out << Format(function, FunctionFormat::AsWrapperImplementation, implement ? "raw_" : "", iface.Name);
			}
		}

		void CodeGenerator::WriteWrapperDispatch(const std::string& interfaceName, const Function& function)
		{
			//TODO: dispatch only implementation
		}

		std::string CodeGenerator::GetWrapperBase(const Interface& iface)
		{
			if (iface.Base == "IUnknown" || iface.Base == "IDispatch")
				return "Com::Pointer<Interface>";
			return iface.Base + "PtrT<Interface>";
		}
	}
};
