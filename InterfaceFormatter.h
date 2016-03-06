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
			AsWrapperForwardDeclaration,
			AsNative,
			AsResolveNameConflict,
			AsWrapper,
			AsWrapperFunctions,
			AsRawFunctions,
			AsCoclassAbstractFunctions,
			AsCoclassFunctionPrototypes,
			AsCoclassFunctionImplementations,
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
			void WriteAsWrapperForwardDeclaration(std::ostream& out) const;
			void WriteAsNative(std::ostream& out) const;
			void WriteAsResolveNameConflict(std::ostream& out) const;
			void WriteAsWrapper(std::ostream& out) const;
			void WriteAsWrapperFunctions(std::ostream& out) const;
			void WriteAsRawFunctions(std::ostream& out) const;
			void WriteAsCoclassAbstractFunctions(std::ostream& out) const;
			void WriteAsCoclassFunctionPrototypes(std::ostream& out) const;
			void WriteAsCoclassFunctionImplementations(std::ostream& out) const;
			void WriteAsTypeInfoSpecialization(std::ostream& out) const;

			std::string InterfaceFormatter::GetWrapperBase() const;
		};

		InterfaceFormatter Format(
			const Interface& value,
			InterfaceFormat format,
			const std::string& prefix = "",
			const std::string& scope = "");
	}
}
