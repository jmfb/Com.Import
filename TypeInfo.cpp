#include "TypeInfo.h"
#include "VariableDescription.h"
#include "FunctionDescription.h"
#include "Loader.h"
#include <algorithm>
#include <map>

namespace Com
{
	namespace Import
	{
		TypeInfo::TypeInfo(Pointer<ITypeInfo> value)
		{
			auto hr = value->QueryInterface(&typeInfo);
			CheckError(hr, __FUNCTION__, "QueryInterface");
			hr = typeInfo->GetTypeAttr(&attributes);
			CheckError(hr, __FUNCTION__, "GetTypeAttr");
			hr = GetLibrary()->GetDocumentation(-1, Get(libraryName), nullptr, nullptr, nullptr);
			CheckError(hr, __FUNCTION__, "GetDocumentation");
		}

		TypeInfo::~TypeInfo()
		{
			if (attributes != nullptr)
				typeInfo->ReleaseTypeAttr(attributes);
		}

		const GUID& TypeInfo::GetId() const
		{
			return attributes->guid;
		}

		std::string TypeInfo::GetName() const
		{
			std::string name;
			auto hr = typeInfo->GetDocumentation(-1, Get(name), nullptr, nullptr, nullptr);
			CheckError(hr, __FUNCTION__, "GetDocumentation");
			return name;
		}

		const std::string& TypeInfo::GetLibraryName() const
		{
			return libraryName;
		}

		Pointer<ITypeLib> TypeInfo::GetLibrary() const
		{
			Pointer<ITypeLib> library;
			UINT index = 0;
			auto hr = typeInfo->GetContainingTypeLib(&library, &index);
			CheckError(hr, __FUNCTION__, "GetContainingTypeLib");
			return library;
		}

		TYPEKIND TypeInfo::GetTypeKind() const
		{
			TYPEKIND typeKind;
			auto hr = typeInfo->GetTypeKind(&typeKind);
			CheckError(hr, __FUNCTION__, "GeTypeKind");
			return typeKind;
		}

		Enum TypeInfo::ToEnum() const
		{
			Enum result{ GetName(),{} };
			for (auto index = 0u; index < attributes->cVars; ++index)
				result.Values.push_back(VariableDescription{ libraryName, typeInfo, index }.ToEnumValue());
			return result;
		}

		Alias TypeInfo::ToAlias() const
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

			if (libraryName != originalLibraryName)
			{
				Loader::AddReference(originalLibrary);
				originalName = originalLibraryName + "::" + originalName;
			}
			return{ originalName, GetName() };
		}

		Coclass TypeInfo::ToCoclass() const
		{
			std::map<std::string, int> countByFunction;
			Coclass result{ GetName(), GetId(),{} };
			for (auto index = 0u; index < attributes->cImplTypes; ++index)
			{
				auto iface = GetInterface(index);
				result.Interfaces.push_back(iface);
				for (auto& function : iface.Functions)
					if (function.VtblOffset >= iface.VtblOffset && function.Retval.TypeEnum == TypeEnum::Hresult)
						++countByFunction[function.Name];
			}
			for (auto& iface : result.Interfaces)
				if (std::any_of(iface.Functions.begin(), iface.Functions.end(), [&](auto f){ return countByFunction[f.Name] > 1; }))
					iface.IsConflicting = true;
			return result;
		}

		Interface TypeInfo::GetInterface(UINT index) const
		{
			HREFTYPE referenceHandle = 0;
			auto hr = typeInfo->GetRefTypeOfImplType(index, &referenceHandle);
			CheckError(hr, __FUNCTION__, "GetRefTypeOfImplType");

			Pointer<ITypeInfo> referenceType;
			hr = typeInfo->GetRefTypeInfo(referenceHandle, &referenceType);
			CheckError(hr, __FUNCTION__, "GetRefTypeInfo");

			TypeInfo referenceTypeInfo{ referenceType };
			auto referenceLibraryName = referenceTypeInfo.GetLibraryName();
			if (referenceLibraryName != libraryName && referenceLibraryName != "stdole")
				Loader::AddReference(referenceTypeInfo.GetLibrary());
			return referenceTypeInfo.ToInterface();
		}

		std::string TypeInfo::GetInterfaceName(UINT index) const
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

			Loader::AddReference(referenceLibrary);
			return referenceLibraryName + "::" + name;
		}

		Record TypeInfo::ToRecord() const
		{
			Record result{ GetName(), GetId(), attributes->cbAlignment,{} };
			for (auto index = 0u; index < attributes->cVars; ++index)
				result.Members.push_back(VariableDescription{ libraryName, typeInfo, index }.ToParameter());
			return result;
		}

		Interface TypeInfo::ToInterface() const
		{
			Interface result{ GetId(), "IID_", GetName(), "IUnknown", IID_IUnknown, false, 12, {}, false };
			if (GetTypeKind() == TKIND_DISPATCH)
			{
				result.Base = "IDispatch";
				result.BaseIid = IID_IDispatch;
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

		void TypeInfo::TryUpdateBaseInterface(Interface& value) const
		{
			HREFTYPE baseHandle = 0;
			auto hr = typeInfo->GetRefTypeOfImplType(0, &baseHandle);
			CheckError(hr, __FUNCTION__, "GetRefTypeOfImplType");

			Pointer<ITypeInfo> baseType;
			hr = typeInfo->GetRefTypeInfo(baseHandle, &baseType);
			CheckError(hr, __FUNCTION__, "GetRefTyepInfo");

			TypeInfo baseTypeInfo{ baseType };
			value.Base = baseTypeInfo.GetName();
			value.BaseIid = baseTypeInfo.GetId();
			value.VtblOffset = baseTypeInfo.attributes->cbSizeVft;
			if (baseTypeInfo.libraryName != libraryName && baseTypeInfo.libraryName != "stdole")
				value.Name = baseTypeInfo.libraryName + "::" + value.Name;
		}

		bool TypeInfo::FunctionIsLessThan(const Function& lhs, const Function& rhs)
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
	}
}
