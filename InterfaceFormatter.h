#pragma once
#include "DataTypes.h"
#include <iostream>

namespace Com
{
	namespace Import
	{
		enum class InterfaceFormat
		{
			AsForwardDeclaration,
			AsNative,
			AsTypeInfoSpecialization
		};

		class InterfaceFormatter
		{
		private:
			const Interface& value;
			InterfaceFormat format;
			std::string prefix;
			std::string scope;

		public:
			InterfaceFormatter(
				const Interface& value,
				InterfaceFormat format,
				const std::string& prefix,
				const std::string& scope);

			std::ostream& Write(std::ostream& out) const;
			friend std::ostream& operator<<(std::ostream& out, const InterfaceFormatter& value);

		private:
			void WriteAsForwardDeclaration(std::ostream& out) const;
			void WriteAsNative(std::ostream& out) const;
			void WriteAsTypeInfoSpecialization(std::ostream& out) const;
		};

		InterfaceFormatter Format(
			const Interface& value,
			InterfaceFormat format,
			const std::string& prefix = "",
			const std::string& scope = "");
	}
}
