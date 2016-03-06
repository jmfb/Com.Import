#pragma once
#include "DataTypes.h"
#include <iostream>

namespace Com
{
	namespace Import
	{
		enum class LibraryFormat
		{
			AsImport
		};

		class LibraryFormatter
		{
		private:
			const Library& value;
			LibraryFormat format;
			bool implement;

		public:
			LibraryFormatter(const Library& value, LibraryFormat format, bool implement);

			std::ostream& Write(std::ostream& out) const;
			friend std::ostream& operator<<(std::ostream& out, const LibraryFormatter& value);

		private:
			void WriteAsImport(std::ostream& out) const;
		};

		LibraryFormatter Format(const Library& library, LibraryFormat format, bool implement = false);
	}
}
