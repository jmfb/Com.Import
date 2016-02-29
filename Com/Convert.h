#pragma once
#include <Windows.h>
#include <OleAuto.h>
#include <string>
#include <locale>
#include <codecvt>
#include <chrono>
#include <ctime>
#include "Pointer.h"

namespace Com
{
	template <typename Source, typename Target>
	inline void Assign(const Source& source, Target& target)
	{
		target = source;
	}

	template <typename Type, typename Source, typename Target>
	class GetValue
	{
	private:
		Target& target;
		Source source = TypeInfo<Target>::DefaultSource;

	public:
		GetValue(Target& value)
			: target(value)
		{
		}
		GetValue(const GetValue<Type, Source, Target>& rhs) = default;
		~GetValue()
		{
			Assign(source, target);
		}

		GetValue<Type, Source, Target>& operator=(const GetValue<Type, Source, Target>& rhs) = default;

		operator Type*()
		{
			return &source;
		}
	};

	template <typename Type, typename Source, typename Target>
	class PutValue
	{
	private:
		Target target;

	public:
		PutValue(const Source& value)
		{
			Assign(value, target);
		}
		PutValue(const PutValue<Type, Source, Target>& rhs) = default;
		~PutValue() = default;

		PutValue<Type, Source, Target>& operator=(const PutValue<Type, Source, Target>& rhs) = default;

		operator Type()
		{
			return target;
		}
	};

	template <typename Type, typename Source, typename Target>
	class PutRefValue
	{
	private:
		Source& source;
		Target target;

	public:
		PutRefValue(Source& value)
			: source(value)
		{
			Assign(source, target);
		}
		PutRefValue(const PutRefValue<Type, Source, Target>& rhs) = default;
		~PutRefValue()
		{
			Assign(target, source);
		}

		PutRefValue<Type, Source, Target>& operator=(const PutRefValue<Type, Source, Target>& rhs) = default;

		operator Type*()
		{
			return &target;
		}
	};

	template <typename Type>
	class TypeInfo
	{
	public:
		using Get = GetValue<Type, Type, Type>;
		using Put = Type;
		using PutRef = GetValue<Type, Type, Type>;
	};

	template <typename Target>
	inline typename TypeInfo<Target>::Get Get(Target& value)
	{
		return{ value };
	}

	template <typename Source>
	inline typename TypeInfo<Source>::Put Put(const Source& source)
	{
		return{ source };
	}

	template <typename Source>
	inline typename TypeInfo<Source>::PutRef PutRef(Source& source)
	{
		return{ source };
	}

	////////////////////////////////////////////////////////////////////////////////
	// Intrinsic Type Support

	template <typename Target>
	class GetValue<Target, Target, Target>
	{
	private:
		Target& target;

	public:
		GetValue(Target& value)
			: target(value)
		{
		}
		GetValue(const GetValue<Target, Target, Target>& rhs) = default;
		~GetValue() = default;

		GetValue<Target, Target, Target>& operator=(const GetValue<Target, Target, Target>& rhs) = default;

		operator Target*()
		{
			return &target;
		}
	};

	////////////////////////////////////////////////////////////////////////////////
	// std::string

	template <>
	class TypeInfo<std::string>
	{
	public:
		using Get = GetValue<BSTR, String, std::string>;
		using Put = PutValue<BSTR, std::string, String>;
		using PutRef = PutRefValue<BSTR, std::string, String>;
		static constexpr BSTR DefaultSource = nullptr;
	};

	template <>
	inline void Assign<String, std::string>(const String& source, std::string& target)
	{
		if (source)
			target = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(source);
		else
			target.clear();
	}

	template <>
	inline void Assign<std::string, String>(const std::string& source, String& target)
	{
		target = source.c_str();
	}

	////////////////////////////////////////////////////////////////////////////////
	// std::wstring

	template <>
	class TypeInfo<std::wstring>
	{
	public:
		using Get = GetValue<BSTR, String, std::wstring>;
		using Put = PutValue<BSTR, std::wstring, String>;
		using PutRef = PutRefValue<BSTR, std::wstring, String>;
		static constexpr BSTR DefaultSource = nullptr;
	};

	template <>
	inline void Assign<String, std::wstring>(const String& source, std::wstring& target)
	{
		if (source)
			target = source;
		else
			target.clear();
	}

	template <>
	inline void Assign<std::wstring, String>(const std::wstring& source, String& target)
	{
		target = source.c_str();
	}

	////////////////////////////////////////////////////////////////////////////////
	// bool

	template <>
	class TypeInfo<bool>
	{
	public:
		using Get = GetValue<VARIANT_BOOL, VARIANT_BOOL, bool>;
		using Put = PutValue<VARIANT_BOOL, bool, VARIANT_BOOL>;
		using PutRef = PutRefValue<VARIANT_BOOL, bool, VARIANT_BOOL>;
		static constexpr VARIANT_BOOL DefaultSource = VARIANT_FALSE;
	};

	template <>
	inline void Assign<VARIANT_BOOL, bool>(const VARIANT_BOOL& source, bool& target)
	{
		target = source != VARIANT_FALSE;
	}

	template <>
	inline void Assign<bool, VARIANT_BOOL>(const bool& source, VARIANT_BOOL& target)
	{
		target = source ? VARIANT_TRUE : VARIANT_FALSE;
	}

	////////////////////////////////////////////////////////////////////////////////
	// std::chrono::time_point

	template <>
	class TypeInfo<std::chrono::system_clock::time_point>
	{
	public:
		using Get = GetValue<DATE, DATE, std::chrono::system_clock::time_point>;
		using Put = PutValue<DATE, std::chrono::system_clock::time_point, DATE>;
		using PutRef = PutRefValue<DATE, std::chrono::system_clock::time_point, DATE>;
		static constexpr DATE DefaultSource = 0;
	};

	template <>
	inline void Assign<DATE, std::chrono::system_clock::time_point>(const DATE& source, std::chrono::system_clock::time_point& target)
	{
		SYSTEMTIME systemTime;
		::VariantTimeToSystemTime(source, &systemTime);
		std::tm tm;
		tm.tm_sec = systemTime.wSecond;
		tm.tm_min = systemTime.wMinute;
		tm.tm_hour = systemTime.wHour;
		tm.tm_mday = systemTime.wDay;
		tm.tm_mon = systemTime.wMonth - 1;
		tm.tm_year = systemTime.wYear - 1900;
		tm.tm_isdst = -1;
		tm.tm_wday = systemTime.wDayOfWeek;
		tm.tm_yday = -1;
		target = std::chrono::system_clock::from_time_t(std::mktime(&tm));
	}

	template <>
	inline void Assign<std::chrono::system_clock::time_point, DATE>(const std::chrono::system_clock::time_point& source, DATE& target)
	{
		auto time = std::chrono::system_clock::to_time_t(source);
		auto tm = *std::localtime(&time);
		SYSTEMTIME systemTime;
		systemTime.wSecond = tm.tm_sec;
		systemTime.wMinute = tm.tm_min;
		systemTime.wHour = tm.tm_hour;
		systemTime.wDay = tm.tm_mday;
		systemTime.wMonth = tm.tm_mon + 1;
		systemTime.wYear = tm.tm_year + 1900;
		systemTime.wDayOfWeek = tm.tm_wday;
		::SystemTimeToVariantTime(&systemTime, &target);
	}
}
