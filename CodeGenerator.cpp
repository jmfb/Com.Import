#include "CodeGenerator.h"
#include "CoclassFormatter.h"
#include "LibraryFormatter.h"
#include "GuidFormatter.h"
#include <fstream>
#include <chrono>
#include <ctime>

namespace Com
{
	namespace Import
	{
		void CodeGenerator::Generate(const LoadLibraryResult& result, bool implement)
		{
			GenerateImport(result.PrimaryLibrary, implement);
			for (auto& reference : result.ReferencedLibraries)
				GenerateImport(reference, false);
			if (!implement)
				return;
			GenerateSolution(result);
			GenerateProject(result);
			GenerateProjectFilters(result);
			GeneratePackages();
			GenerateResourceHeader(result.PrimaryLibrary);
			GenerateResources(result.PrimaryLibrary);
			GenerateDef(result.PrimaryLibrary);
			GenerateManifest(result.PrimaryLibrary);
			GenerateMain(result.PrimaryLibrary);
			for (auto& coclass : result.PrimaryLibrary.Coclasses)
			{
				GenerateCoclassHeader(result.PrimaryLibrary, coclass);
				GenerateCoclassSource(result.PrimaryLibrary, coclass);
			}
		}

		void CodeGenerator::GenerateImport(const Library& library, bool implement)
		{
			auto fileName = library.OutputName + ".h";
			std::cout << "Generating import: " << fileName << std::endl;
			std::ofstream{ fileName.c_str() } << Format(library, LibraryFormat::AsImport, implement);
		}

		void CodeGenerator::GenerateSolution(const LoadLibraryResult& result)
		{
			auto fileName = result.PrimaryLibrary.Name + ".sln";
			std::cout << "Generating solution: " << fileName << std::endl;
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
			std::cout << "Generating project: " << fileName << std::endl;
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
				<< "  <Import Project=\"packages\\Jmfb.Com.1.0.6\\build\\native\\Jmfb.Com.targets\" "
					<< "Condition=\"Exists('packages\\Jmfb.Com.1.0.6\\build\\native\\Jmfb.Com.targets')\" />" << std::endl
				<< "  <Target Name=\"EnsureNuGetPackageBuildImports\" BeforeTargets=\"PrepareForBuild\">" << std::endl
				<< "    <PropertyGroup>" << std::endl
				<< "      <ErrorText>This project references NuGet package(s) that are missing on this computer. "
					<< "Use NuGet Package Restore to download them.  For more information, see http://go.microsoft.com/fwlink/?LinkID=322105. "
					<< "The missing file is {0}.</ErrorText>" << std::endl
				<< "    </PropertyGroup>" << std::endl
				<< "    <Error Condition=\"!Exists('packages\\Jmfb.Com.1.0.6\\build\\native\\Jmfb.Com.targets')\" "
					<< "Text=\"$([System.String]::Format('$(ErrorText)', 'packages\\Jmfb.Com.1.0.6\\build\\native\\Jmfb.Com.targets'))\" />" << std::endl
				<< "  </Target>" << std::endl
				<< "</Project>" << std::endl;
		}

		void CodeGenerator::GenerateProjectFilters(const LoadLibraryResult& result)
		{
			auto fileName = result.PrimaryLibrary.Name + ".vcxproj.filters";
			std::cout << "Generating filters: " << fileName << std::endl;
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

		void CodeGenerator::GeneratePackages()
		{
			auto fileName = "packages.config";
			std::cout << "Generating packages: " << fileName << std::endl;
			std::ofstream{ fileName }
				<< "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl
				<< "<packages>" << std::endl
				<< "	<package id=\"Jmfb.Com\" version=\"1.0.6\" targetFramework=\"native\" />" << std::endl
				<< "</packages>" << std::endl;
		}

		void CodeGenerator::GenerateResourceHeader(const Library& library)
		{
			auto fileName = "resource.h";
			std::cout << "Generate resource header: " << fileName << std::endl;
			std::ofstream{ fileName }
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

		void CodeGenerator::GenerateResources(const Library& library)
		{
			auto fileName = library.Name + ".rc";
			std::cout << "Generate resources: " << fileName << std::endl;

			auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			auto year = std::localtime(&time)->tm_year + 1900;

			std::ofstream{ fileName.c_str() }
				<< "// Microsoft Visual C++ generated resource script." << std::endl
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
		}

		void CodeGenerator::GenerateDef(const Library& library)
		{
			auto fileName = library.Name + ".def";
			std::cout << "Generate module definition: " << fileName << std::endl;
			std::ofstream{ fileName.c_str() }
				<< "LIBRARY \"" << library.OutputName << "\"" << std::endl
				<< "EXPORTS" << std::endl
				<< "	DllCanUnloadNow private" << std::endl
				<< "	DllGetClassObject private" << std::endl;
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

		void CodeGenerator::GenerateMain(const Library& library)
		{
			std::cout << "Generating source: main.cpp" << std::endl;
			std::ofstream out{ "main.cpp" };
			out << "#include <Com/Com.h>" << std::endl;
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

		void CodeGenerator::GenerateCoclassHeader(const Library& library, const Coclass& coclass)
		{
			auto fileName = coclass.Name + ".h";
			std::cout << "Generating header: " << fileName << std::endl;
			std::ofstream{ fileName.c_str() } << Format(coclass, CoclassFormat::AsObjectHeader, library.Name, library.OutputName);
		}

		void CodeGenerator::GenerateCoclassSource(const Library& library, const Coclass& coclass)
		{
			auto fileName = coclass.Name + ".cpp";
			std::cout << "Generating source: " << fileName << std::endl;
			std::ofstream{ fileName.c_str() } << Format(coclass, CoclassFormat::AsObjectSource, library.Name);
		}
	}
};
