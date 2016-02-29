#pragma once
#include "DataTypes.h"
#include "TypeDescription.h"
#include "Com/Com.h"

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
			ElementDescription(const std::string& libraryName, Pointer<ITypeInfo2> typeInfo, const std::string& name, const ELEMDESC& description)
				: libraryName(libraryName), typeInfo(typeInfo), name(name), description(description)
			{
			}
			ElementDescription(const ElementDescription& rhs) = delete;
			~ElementDescription() = default;

			ElementDescription& operator=(const ElementDescription& rhs) = delete;

			bool HasFlag(USHORT flag) const
			{
				return (description.paramdesc.wParamFlags & flag) == flag;
			}

			Type GetType() const
			{
				return TypeDescription{ libraryName, typeInfo, description.tdesc }.ToType();
			}

			Parameter ToParameter() const
			{
				return
				{
					name,
					GetType(),
					HasFlag(PARAMFLAG_FIN),
					HasFlag(PARAMFLAG_FOUT),
					HasFlag(PARAMFLAG_FRETVAL)
				};
			}
		};
	}
}
