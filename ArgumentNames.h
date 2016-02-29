#pragma once
#include "Com/Com.h"
#include <locale>
#include <codecvt>
#include <vector>
#include <string>

namespace Com
{
	namespace Import
	{
		class ArgumentNames
		{
		private:
			std::vector<BSTR> names;

		public:
			ArgumentNames(Pointer<ITypeInfo2> typeInfo, MEMBERID member, UINT argumentCount)
			{
				names.resize(argumentCount + 1, nullptr);
				UINT count = 0;
				auto hr = typeInfo->GetNames(member, names.data(), names.size(), &count);
				CheckError(hr, __FUNCTION__, "GetNames");
				names.resize(count);
				if (names.empty())
					return;
				::SysFreeString(names.front());
				names.erase(names.begin());
			}
			ArgumentNames(const ArgumentNames& rhs) = delete;
			~ArgumentNames()
			{
				for (auto name : names)
					::SysFreeString(name);
			}

			ArgumentNames& operator=(const ArgumentNames& rhs) = delete;

			std::string operator[](UINT index) const
			{
				if (index >= names.size())
					return "value";
				return names[index] == nullptr ?
					"" :
					std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(names[index]);
			}
		};
	}
}
