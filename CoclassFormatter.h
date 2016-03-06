#pragma once
#include "DataTypes.h"
#include <iostream>

namespace Com
{
	namespace Import
	{
		enum class CoclassFormat
		{
			AsBase,
			AsObjectHeader,
			AsObjectSource
		};

		class CoclassFormatter
		{
		private:
			const Coclass& value;
			CoclassFormat format;
			std::string libraryName;
			std::string outputName;

		public:
			CoclassFormatter(
				const Coclass& value,
				CoclassFormat format,
				const std::string& libraryName,
				const std::string& outputName);

			std::ostream& Write(std::ostream& out) const;
			friend std::ostream& operator<<(std::ostream& out, const CoclassFormatter& value);

		private:
			void WriteAsBase(std::ostream& out) const;
			void WriteAsObjectHeader(std::ostream& out) const;
			void WriteAsObjectSource(std::ostream& out) const;
		};

		CoclassFormatter Format(
			const Coclass& value,
			CoclassFormat format,
			const std::string& libraryName = "",
			const std::string& outputName = "");
	}
}
