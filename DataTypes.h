#pragma once
#include <objbase.h>
#include <string>
#include <vector>

namespace Com
{
	namespace Import
	{
		struct Identifier
		{
			std::string Name;
			GUID Guid;
		};

		struct EnumValue
		{
			std::string Name;
			long Value;
		};

		struct Enum
		{
			std::string Name;
			std::vector<EnumValue> Values;
		};

		struct Alias
		{
			std::string OldName;
			std::string NewName;
		};

		enum class TypeEnum
		{
			Enum,
			Interface,
			Void,
			Empty,
			Null,
			Int,
			Int8,
			Int16,
			Int32,
			Int64,
			UInt,
			UInt8,
			UInt16,
			UInt32,
			UInt64,
			Float,
			Double,
			Currency,
			Date,
			String,
			Dispatch,
			Error,
			Bool,
			Variant,
			Decimal,
			Record,
			Unknown,
			Hresult,
			SafeArray,
			Guid,
			StringPtrA,
			StringPtrW,
		};

		struct Type
		{
			int Indirection;
			TypeEnum TypeEnum;
			std::string CustomName;
			bool IsArray;
			unsigned long ArraySize;
		};

		struct Parameter
		{
			std::string Name;
			Type Type;
			bool In;
			bool Out;
			bool Retval;
		};

		struct Function
		{
			unsigned long VtblOffset;
			bool IsDispatchOnly;
			MEMBERID MemberId;
			std::string Name;
			Type Retval;
			std::vector<Parameter> ArgList;
			std::string RootName;
			bool IsProperty;
			bool IsPropGet;
			bool IsPropPut;
			bool IsPropPutRef;
		};

		struct Interface
		{
			GUID Iid;
			std::string Prefix;
			std::string Name;
			std::string Base;
			GUID BaseIid;
			bool SupportsDispatch;
			unsigned int VtblOffset;
			std::vector<Function> Functions;
		};

		struct Coclass
		{
			std::string Name;
			GUID Clsid;
			std::vector<Interface> Interfaces;
		};

		struct Record
		{
			std::string Name;
			GUID Guid;
			unsigned long Alignment;
			std::vector<Parameter> Members;
		};

		struct Library
		{
			std::string Name;
			std::string HeaderFileName;
			GUID Libid;
			WORD MajorVersion;
			WORD MinorVersion;
			std::vector<std::string> References;
			std::vector<Identifier> Identifiers;
			std::vector<Enum> Enums;
			std::vector<Alias> Aliases;
			std::vector<Coclass> Coclasses;
			std::vector<Interface> Interfaces;
			std::vector<Record> Records;
		};

		struct LoadLibraryResult
		{
			Library PrimaryLibrary;
			std::vector<Library> ReferencedLibraries;
		};
	}
}
