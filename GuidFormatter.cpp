#include "GuidFormatter.h"
#include <locale>
#include <codecvt>
#include <stdexcept>
#include "HexFormatter.h"

namespace Com
{
	namespace Import
	{
		GuidFormatter::GuidFormatter(const GUID& value, GuidFormat format)
			: value(value), format(format)
		{
		}

		std::ostream& GuidFormatter::Write(std::ostream& out) const
		{
			switch (format)
			{
			case GuidFormat::AsString:
				WriteAsString(out);
				break;
			case GuidFormat::AsAttribute:
				WriteAsAttribute(out);
				break;
			case GuidFormat::AsInitializer:
				WriteAsInitializer(out);
				break;
			}
			return out;
		}

		std::ostream& operator<<(std::ostream& out, const GuidFormatter& value)
		{
			return value.Write(out);
		}

		void GuidFormatter::WriteAsString(std::ostream& out) const
		{
			const auto bufferSize = 39;
			wchar_t buffer[bufferSize];
			auto result = ::StringFromGUID2(value, buffer, bufferSize);
			if (result != bufferSize)
				throw std::runtime_error("Error formatting GUID");
			out << std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(buffer).substr(1, 36);
		}

		void GuidFormatter::WriteAsAttribute(std::ostream& out) const
		{
			out << "__declspec(uuid(\"";
			WriteAsString(out);
			out << "\"))";
		}

		void GuidFormatter::WriteAsInitializer(std::ostream& out) const
		{
			out << "{" << Hex(value.Data1) << "," << Hex(value.Data2) << "," << Hex(value.Data3) << ",{";
			auto first = true;
			for (auto part : value.Data4)
			{
				if (!first)
					out << ",";
				first = false;
				out << Hex(part);
			}
			out << "}}";
		}

		GuidFormatter Format(const GUID& value, GuidFormat format)
		{
			return{ value, format };
		}
	}
}
