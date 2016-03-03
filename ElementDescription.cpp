#include "ElementDescription.h"
#include "TypeDescription.h"

namespace Com
{
	namespace Import
	{
		ElementDescription::ElementDescription(
			const std::string& libraryName,
			Pointer<ITypeInfo2> typeInfo,
			const std::string& name,
			const ELEMDESC& description)
			: libraryName(libraryName),
			typeInfo(typeInfo),
			name(name),
			description(description)
		{
		}

		bool ElementDescription::HasFlag(USHORT flag) const
		{
			return (description.paramdesc.wParamFlags & flag) == flag;
		}

		Type ElementDescription::GetType() const
		{
			return TypeDescription{ libraryName, typeInfo, description.tdesc }.ToType();
		}

		Parameter ElementDescription::ToParameter() const
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
	}
}
