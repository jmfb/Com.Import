#pragma once
#include <objbase.h>
#include <iostream>

namespace Com
{
	namespace Import
	{
		enum class GuidFormat
		{
			AsString,
			AsAttribute,
			AsInitializer
		};

		class GuidFormatter
		{
		private:
			const GUID& value;
			GuidFormat format;

		public:
			GuidFormatter(const GUID& value, GuidFormat format);

			std::ostream& Write(std::ostream& out) const;
			friend std::ostream& operator<<(std::ostream& out, const GuidFormatter& value);

		private:
			void WriteAsString(std::ostream& out) const;
			void WriteAsAttribute(std::ostream& out) const;
			void WriteAsInitializer(std::ostream& out) const;
		};

		GuidFormatter Format(const GUID& value, GuidFormat format);
	}
}
