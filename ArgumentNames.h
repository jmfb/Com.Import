#pragma once
#include <Com/Com.h>
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
			ArgumentNames(Pointer<ITypeInfo2> typeInfo, MEMBERID member, UINT argumentCount);
			ArgumentNames(const ArgumentNames& rhs) = delete;
			~ArgumentNames();

			ArgumentNames& operator=(const ArgumentNames& rhs) = delete;

			std::string operator[](UINT index) const;
		};
	}
}
