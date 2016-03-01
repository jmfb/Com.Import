#pragma once
#include "DataTypes.h"
#include "VariableDescription.h"
#include "FunctionDescription.h"
#include "Com/Com.h"
#include <algorithm>

namespace Com
{
	namespace Import
	{
		class TypeInfo
		{
		private:
			std::string libraryName;
			Pointer<ITypeInfo2> typeInfo;
			TYPEATTR* attributes = nullptr;

		public:
			TypeInfo(Pointer<ITypeInfo> value)
			{
				auto hr = value->QueryInterface(&typeInfo);
				CheckError(hr, __FUNCTION__, "QueryInterface");
				hr = typeInfo->GetTypeAttr(&attributes);
				CheckError(hr, __FUNCTION__, "GetTypeAttr");
				Pointer<ITypeLib> typeLibrary;
				UINT index = 0;
				hr = typeInfo->GetContainingTypeLib(&typeLibrary, &index);
				CheckError(hr, __FUNCTION__, "GetContainingTypeLib");
				hr = typeLibrary->GetDocumentation(-1, Get(libraryName), nullptr, nullptr, nullptr);
				CheckError(hr, __FUNCTION__, "GetDocumentation");
			}
			TypeInfo(const TypeInfo& rhs) = delete;
			~TypeInfo()
			{
				if (attributes != nullptr)
					typeInfo->ReleaseTypeAttr(attributes);
			}

			TypeInfo& operator=(const TypeInfo& rhs) = delete;

			const GUID& GetId() const
			{
				return attributes->guid;
			}

			std::string GetName() const
			{
				std::string name;
				auto hr = typeInfo->GetDocumentation(-1, Get(name), nullptr, nullptr, nullptr);
				CheckError(hr, __FUNCTION__, "GetDocumentation");
				return name;
			}

			TYPEKIND GetTypeKind() const
			{
				TYPEKIND typeKind;
				auto hr = typeInfo->GetTypeKind(&typeKind);
				CheckError(hr, __FUNCTION__, "GeTypeKind");
				return typeKind;
			}

			Enum ToEnum() const
			{
				Enum result{ GetName(), {} };
				for (auto index = 0u; index < attributes->cVars; ++index)
					result.Values.push_back(VariableDescription{ libraryName, typeInfo, index }.ToEnumValue());
				return result;
			}

			Alias ToAlias() const
			{
				switch (attributes->tdescAlias.vt)
				{
				case VT_I4:
					return{ "long", GetName() };
				case VT_UI4:
					return{ "unsigned long", GetName() };
				case VT_USERDEFINED:
					break;
				default:
					throw std::runtime_error("Unsupported source type for alias.");
				}

				Pointer<ITypeInfo> originalType;
				auto hr = typeInfo->GetRefTypeInfo(attributes->tdescAlias.hreftype, &originalType);
				CheckError(hr, __FUNCTION__, "typeInfo->GetRefTypeInfo");

				std::string originalName;
				hr = originalType->GetDocumentation(-1, Get(originalName), nullptr, nullptr, nullptr);
				CheckError(hr, __FUNCTION__, "originalType->GetDocumentation");

				Pointer<ITypeLib> originalLibrary;
				UINT index = 0;
				hr = originalType->GetContainingTypeLib(&originalLibrary, &index);
				CheckError(hr, __FUNCTION__, "originalType->GetContainingTypeLib");

				std::string originalLibraryName;
				hr = originalLibrary->GetDocumentation(-1, Get(originalLibraryName), nullptr, nullptr, nullptr);
				CheckError(hr, __FUNCTION__, "originalLibrary->GetDocumentation");

				//TODO: Add reference to other library
				if (libraryName != originalLibraryName)
					originalName = originalLibraryName + "::" + originalName;
				return{ originalName, GetName() };
			}

			Coclass ToCoclass() const
			{
				Coclass result{ GetName(), GetId(), {} };
				for (auto index = 0u; index < attributes->cImplTypes; ++index)
					result.Interfaces.push_back(GetInterfaceName(index));
				return result;
			}

			std::string GetInterfaceName(UINT index) const
			{
				HREFTYPE referenceHandle = 0;
				auto hr = typeInfo->GetRefTypeOfImplType(index, &referenceHandle);
				CheckError(hr, __FUNCTION__, "GetRefTypeOfImplType");

				Pointer<ITypeInfo> referenceType;
				hr = typeInfo->GetRefTypeInfo(referenceHandle, &referenceType);
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
				if (referenceLibraryName == libraryName)
					return name;

				if (referenceLibraryName == "stdole")
				{
					if (name == "IUnknown" || name == "IDispatch")
						return name;
					return "I" + name;
				}

				//TODO: Add reference to library
				return referenceLibraryName + "::" + name;
			}

			Record ToRecord() const
			{
				Record result{ GetName(), GetId(), attributes->cbAlignment, {} };
				for (auto index = 0u; index < attributes->cVars; ++index)
					result.Members.push_back(VariableDescription{ libraryName, typeInfo, index }.ToParameter());
				return result;
			}

			Interface ToInterface() const
			{
				Interface result{ GetId(), "IID_", GetName(), "IUnknown", false, 12, {} };
				if (GetTypeKind() == TKIND_DISPATCH)
				{
					result.Base = "IDispatch";
					result.SupportsDispatch = true;
					result.VtblOffset += 16;
					ULONG flags = 0;
					auto hr = typeInfo->GetTypeFlags(&flags);
					CheckError(hr, __FUNCTION__, "GetTypeFlags");
					if ((flags & TYPEFLAG_FDUAL) != TYPEFLAG_FDUAL)
						result.Prefix = "DIID_";
				}
				if (attributes->cImplTypes == 1)
					TryUpdateBaseInterface(result);
				for (auto index = 0u; index < attributes->cFuncs; ++index)
					result.Functions.push_back(FunctionDescription{ libraryName, typeInfo, index }.ToFunction(result.SupportsDispatch));
				std::sort(result.Functions.begin(), result.Functions.end(), &FunctionIsLessThan);
				return result;
			}

		private:
			void TryUpdateBaseInterface(Interface& value) const
			{
				HREFTYPE baseHandle = 0;
				auto hr = typeInfo->GetRefTypeOfImplType(0, &baseHandle);
				CheckError(hr, __FUNCTION__, "GetRefTypeOfImplType");

				Pointer<ITypeInfo> baseType;
				hr = typeInfo->GetRefTypeInfo(baseHandle, &baseType);
				CheckError(hr, __FUNCTION__, "GetRefTyepInfo");

				TypeInfo baseTypeInfo{ baseType };
				value.Base = baseTypeInfo.GetName();
				value.VtblOffset = baseTypeInfo.attributes->cbSizeVft;
				if (baseTypeInfo.libraryName != libraryName && baseTypeInfo.libraryName != "stdole")
					value.Name = baseTypeInfo.libraryName + "::" + value.Name;
			}

			static bool FunctionIsLessThan(const Function& lhs, const Function& rhs)
			{
				if (!lhs.IsDispatchOnly)
					return rhs.IsDispatchOnly ?
						true :
						lhs.VtblOffset < rhs.VtblOffset;
				if (rhs.IsDispatchOnly)
					return false;
				if (lhs.MemberId != rhs.MemberId)
					return lhs.MemberId < rhs.MemberId;
				if (lhs.IsPropGet != rhs.IsPropGet)
					return lhs.IsPropGet;
				if (lhs.IsPropPut != rhs.IsPropPut)
					return lhs.IsPropPut;
				if (lhs.IsPropPutRef != rhs.IsPropPutRef)
					return lhs.IsPropPutRef;
				return false;
			}
		};
	}
}
