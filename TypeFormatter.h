#pragma once
#include "DataTypes.h"
#include <iostream>

namespace Com
{
	namespace Import
	{
		enum class TypeFormat
		{
			AsNative,
			AsWrapper,
			AsSuffix,
			AsInitializer
		};

		class TypeFormatter
		{
		private:
			const Type& value;
			TypeFormat format;

		public:
			TypeFormatter(const Type& value, TypeFormat format);

			std::ostream& Write(std::ostream& out) const;
			friend std::ostream& operator<<(std::ostream& out, const TypeFormatter& value);

		private:
			void WriteAsNative(std::ostream& out) const;
			void WriteAsWrapper(std::ostream& out) const;
			void WriteAsSuffix(std::ostream& out) const;
			void WriteAsInitializer(std::ostream& out) const;

			std::string GetSmartPointer() const;
		};

		TypeFormatter Format(const Type& type, TypeFormat format);
	}
}
