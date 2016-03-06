#pragma once
#include "DataTypes.h"
#include <Com/Com.h>

namespace Com
{
	namespace Import
	{
		class ElementDescription
		{
		private:
			std::string libraryName;
			Pointer<ITypeInfo2> typeInfo;
			std::string name;
			const ELEMDESC& description;

		public:
			ElementDescription(
				const std::string& libraryName,
				Pointer<ITypeInfo2> typeInfo,
				const std::string& name,
				const ELEMDESC& description);
			ElementDescription(const ElementDescription& rhs) = delete;
			~ElementDescription() = default;

			ElementDescription& operator=(const ElementDescription& rhs) = delete;

			bool HasFlag(USHORT flag) const;
			Type GetType() const;
			Parameter ToParameter() const;
		};
	}
}
