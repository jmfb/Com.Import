#pragma once
#include <iostream>
#include <type_traits>
#include <iomanip>

namespace Com
{
	namespace Import
	{
		template <typename Integer>
		class HexFormatter
		{
		private:
			Integer value;

		public:
			HexFormatter(Integer value)
				: value(value)
			{
			}

			std::ostream& Write(std::ostream& out) const
			{
				static_assert(sizeof(value) <= sizeof(int), "Integer type is too large.");
				using UnsignedInteger = std::make_unsigned<Integer>::type;
				return out << "0x"
					<< std::setw(sizeof(value) * 2)
					<< std::setfill('0')
					<< std::right
					<< std::hex
					<< static_cast<unsigned int>(static_cast<UnsignedInteger>(value))
					<< std::dec;
			}

			friend std::ostream& operator<<(std::ostream& out, const HexFormatter& value)
			{
				return value.Write(out);
			}
		};

		template <typename Integer>
		inline HexFormatter<Integer> Hex(Integer value)
		{
			return{ value };
		}
	}
}
