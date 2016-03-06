#pragma once
#include "DataTypes.h"
#include <iostream>

namespace Com
{
	namespace Import
	{
		enum class FunctionFormat
		{
			AsAbstract,
			AsResolveNameConflict,
			AsWrapper,
			AsWrapperImplementation,
			AsWrapperDispatch,
			AsRawImplementation
		};

		class FunctionFormatter
		{
		private:
			const Function& value;
			FunctionFormat format;
			std::string prefix;
			std::string scope;

		public:
			FunctionFormatter(
				const Function& value,
				FunctionFormat format,
				const std::string& prefix,
				const std::string& scope);

			std::ostream& Write(std::ostream& out) const;
			friend std::ostream& operator<<(std::ostream& out, const FunctionFormatter& value);

		private:
			void WriteAsAbstract(std::ostream& out) const;
			void WriteAsResolveNameConflict(std::ostream& out) const;
			void WriteAsWrapper(std::ostream& out) const;
			void WriteAsWrapperImplementation(std::ostream& out) const;
			void WriteAsWrapperDispatch(std::ostream& out) const;
			void WriteAsRawImplementation(std::ostream& out) const;
		};

		FunctionFormatter Format(
			const Function& value,
			FunctionFormat format,
			const std::string& prefix = "",
			const std::string& scope = "");
	}
}
