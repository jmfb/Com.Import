#pragma once
#include "DataTypes.h"
#include "Com/Com.h"

namespace Com
{
	namespace Import
	{
		class TypeDescription
		{
		private:
			std::string libraryName;
			Pointer<ITypeInfo2> typeInfo;
			const TYPEDESC& description;

		public:
			TypeDescription(const std::string& libraryName, Pointer<ITypeInfo2> typeInfo, const TYPEDESC& description)
				: libraryName(libraryName), typeInfo(typeInfo), description(description)
			{
			}
			TypeDescription(const TypeDescription& rhs) = delete;
			~TypeDescription() = default;

			TypeDescription& operator=(const TypeDescription& rhs) = delete;

			Type ToType() const
			{
				return DetermineType(description, 0);
			}

		private:
			Type DetermineType(const TYPEDESC& typeDescription, int indirection) const
			{
				switch (typeDescription.vt)
				{
				case VT_PTR:
					return DetermineType(*typeDescription.lptdesc, indirection + 1);
				case VT_CARRAY:
					return ToArrayType(*typeDescription.lpadesc, indirection);
				case VT_USERDEFINED:
					return ToUserDefinedType(typeDescription.hreftype, indirection);
				}
				return{ indirection, ToBasicTypeEnum(typeDescription.vt), "", false, 0 };
			}

			Type ToArrayType(const ARRAYDESC& arrayDescription, int indirection) const
			{
				if (indirection != 0)
					throw std::runtime_error("VT_CARRAY does not support indirection.");
				if (arrayDescription.cDims != 1)
					throw std::runtime_error("VT_CARRAY does not support more than 1 dimension.");
				auto& bounds = arrayDescription.rgbounds[0];
				if (bounds.lLbound != 0)
					throw std::runtime_error("VT_CARRAY does not support lower bounds other than 0.");
				if (bounds.cElements == 0)
					throw std::runtime_error("VT_CARRAY does not support empty arrays.");
				auto typeEnum = ToArrayTypeEnum(arrayDescription.tdescElem.vt);
				return{ 0, typeEnum, "", true, bounds.cElements };
			}

			static TypeEnum ToArrayTypeEnum(VARTYPE vt)
			{
				switch (vt)
				{
				case VT_INT: return TypeEnum::Int;
				case VT_I1: return TypeEnum::Int8;
				case VT_I2: return TypeEnum::Int16;
				case VT_I4: return TypeEnum::Int32;
				case VT_I8: return TypeEnum::Int64;
				case VT_UINT: return TypeEnum::UInt;
				case VT_UI1: return TypeEnum::UInt8;
				case VT_UI2: return TypeEnum::UInt16;
				case VT_UI4: return TypeEnum::UInt32;
				case VT_UI8: return TypeEnum::UInt64;
				}
				throw std::runtime_error("VT_CARRAY unsupported type.");
			}

			static TypeEnum ToBasicTypeEnum(VARTYPE vt)
			{
				if ((vt & VT_BYREF) == VT_BYREF)
					throw std::runtime_error("VT_BYREF is unsupported.");
				if ((vt & VT_ARRAY) == VT_ARRAY)
					throw std::runtime_error("T_ARRAY is unsupported.");
				switch (vt)
				{
				case VT_VOID: return TypeEnum::Void;
				case VT_EMPTY: return TypeEnum::Empty;
				case VT_NULL: return TypeEnum::Null;
				case VT_INT: return TypeEnum::Int;
				case VT_I1: return TypeEnum::Int8;
				case VT_I2: return TypeEnum::Int16;
				case VT_I4: return TypeEnum::Int32;
				case VT_I8: return TypeEnum::Int64;
				case VT_UINT: return TypeEnum::UInt;
				case VT_UI1: return TypeEnum::UInt8;
				case VT_UI2: return TypeEnum::UInt16;
				case VT_UI4: return TypeEnum::UInt32;
				case VT_UI8: return TypeEnum::UInt64;
				case VT_R4: return TypeEnum::Float;
				case VT_R8: return TypeEnum::Double;
				case VT_CY: return TypeEnum::Currency;
				case VT_DATE: return TypeEnum::Date;
				case VT_BSTR: return TypeEnum::String;
				case VT_DISPATCH: return TypeEnum::Dispatch;
				case VT_ERROR: return TypeEnum::Error;
				case VT_BOOL: return TypeEnum::Bool;
				case VT_VARIANT: return TypeEnum::Variant;
				case VT_DECIMAL: return TypeEnum::Decimal;
				case VT_RECORD: return TypeEnum::Record;
				case VT_UNKNOWN: return TypeEnum::Unknown;
				case VT_HRESULT: return TypeEnum::Hresult;
				case VT_SAFEARRAY: return TypeEnum::SafeArray;
				case VT_CLSID: return TypeEnum::Guid;
				case VT_LPSTR: return TypeEnum::StringPtrA;
				case VT_LPWSTR: return TypeEnum::StringPtrW;
				}
				throw std::runtime_error("Unsupported VARTYPE: " + std::to_string(vt));
			}

			Type ToUserDefinedType(HREFTYPE handle, int indirection) const
			{
				Pointer<ITypeInfo> customType;
				auto hr = typeInfo->GetRefTypeInfo(handle, &customType);
				CheckError(hr, __FUNCTION__, "GetRefTypeInfo");

				auto customName = GetUserDefinedTypeName(customType);
				switch (GetTypeKind(customType))
				{
				case TKIND_ENUM:
					return{ indirection, TypeEnum::Enum, customName, false, 0 };
				case TKIND_ALIAS:
					if (customName == "vsIndentStyle" ||
						customName == "OLE_COLOR" ||
						customName == "MsoRGBType")
						return{ indirection, TypeEnum::Enum, customName, false, 0 };
					return{ indirection, TypeEnum::Interface, customName, false, 0 };
				case TKIND_INTERFACE:
				case TKIND_DISPATCH:
					return{ indirection, TypeEnum::Interface, customName, false, 0 };
				case TKIND_RECORD:
					return{ indirection, TypeEnum::Record, customName, false, 0 };
				case TKIND_COCLASS:
					return{ indirection, TypeEnum::Interface, GetDefaultInterfaceName(customType), false, 0 };
				}
				throw std::runtime_error("Unsupported user defined type TYPEKIND.");
			}

			std::string GetUserDefinedTypeName(Pointer<ITypeInfo> customType) const
			{
				std::string customName;
				auto hr = customType->GetDocumentation(-1, Get(customName), nullptr, nullptr, nullptr);
				CheckError(hr, __FUNCTION__, "GetDocumentation");

				Pointer<ITypeLib> customLibrary;
				UINT customIndex = 0;
				hr = customType->GetContainingTypeLib(&customLibrary, &customIndex);
				CheckError(hr, __FUNCTION__, "GetContainingTypeLib");

				std::string customLibraryName;
				hr = customLibrary->GetDocumentation(-1, Get(customLibraryName), nullptr, nullptr, nullptr);
				CheckError(hr, __FUNCTION__, "customLibrary->GetDocumentation");

				if (customLibraryName == libraryName)
					return customName;

				if (customLibraryName == "stdole")
				{
					if (customName == "IUnknown" ||
						customName == "IDispatch" ||
						customName == "IEnumVARIANT" ||
						customName == "IPictureDisp" ||
						customName == "IFont" ||
						customName == "OLE_COLOR" ||
						customName == "OLE_HANDLE" ||
						customName == "OLE_XPOS_CONTAINER" ||
						customName == "OLE_YPOS_CONTAINER" ||
						customName == "GUID")
						return customName;
					return "I" + customName;
				}

				//TODO: Add reference to library
				return customLibraryName + "::" + customName;
			}

			static TYPEKIND GetTypeKind(Pointer<ITypeInfo> customType)
			{
				TYPEATTR* attributes = nullptr;
				auto hr = customType->GetTypeAttr(&attributes);
				CheckError(hr, __FUNCTION__, "GetTypeAttr");
				auto typeKind = attributes->typekind;
				customType->ReleaseTypeAttr(attributes);
				return typeKind;
			}

			//TODO: This function is identical to TypeInfo::GetInterfaceName(0)
			std::string GetDefaultInterfaceName(Pointer<ITypeInfo> customType) const
			{
				HREFTYPE referenceHandle = 0;
				auto hr = customType->GetRefTypeOfImplType(0, &referenceHandle);
				CheckError(hr, __FUNCTION__, "GetRefTypeOfImplType");

				Pointer<ITypeInfo> referenceType;
				hr = customType->GetRefTypeInfo(referenceHandle, &referenceType);
				CheckError(hr, __FUNCTION__, "GetRefTypeInfo");

				std::string name;
				hr = referenceType->GetDocumentation(-1, Get(name), nullptr, nullptr, nullptr);
				CheckError(hr, __FUNCTION__, "GetDocumentation");

				Pointer<ITypeLib> referenceLibrary;
				UINT referenceIndex = 0;
				hr = referenceType->GetContainingTypeLib(&referenceLibrary, &referenceIndex);
				CheckError(hr, __FUNCTION__, "GetContainingTypeLib");

				std::string referenceLibraryName;
				hr = referenceLibrary->GetDocumentation(-1, Get(referenceLibraryName), nullptr, nullptr, nullptr);
				CheckError(hr, __FUNCTION__, "referenceTypeLibrary->GetDocumentation");
				if (libraryName == referenceLibraryName)
					return name;

				if (libraryName == "stdole")
				{
					if (name == "IUnknown" || name == "IDispatch")
						return name;
					return "I" + name;
				}

				//TODO: Add reference to library
				return referenceLibraryName + "::" + name;
			}
		};
	}
}
