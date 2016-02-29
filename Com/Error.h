#pragma once
#include <objbase.h>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include "Convert.h"

namespace Com
{
	class Error : public std::runtime_error
	{
	private:
		HRESULT hr = S_OK;
		std::string function;
		std::string location;
		std::string description;

	public:
		Error(HRESULT hr, const std::string& function, const std::string& location, const std::string& description)
			: hr(hr),
			function(function),
			location(location),
			description(description),
			std::runtime_error(Format(hr, function, location, description))
		{
		}
		Error(const Error& rhs) = default;
		~Error() = default;

		Error& operator=(const Error& error) = default;

		HRESULT GetHR() const
		{
			return hr;
		}
		const std::string& GetFunction() const
		{
			return function;
		}
		const std::string& GetLocation() const
		{
			return location;
		}
		const std::string& GetDescription() const
		{
			return description;
		}

		HRESULT SetErrorInfo() const
		{
			return SetErrorInfo(hr, what());
		}

		static HRESULT SetErrorInfo(HRESULT hr, const std::string& description)
		{
			Pointer<ICreateErrorInfo> createErrorInfo;
			auto result = ::CreateErrorInfo(&createErrorInfo);
			if (FAILED(result))
				return hr;
			result = createErrorInfo->SetDescription(Com::Put(description));
			if (FAILED(result))
				return hr;
			QueryPointer<IErrorInfo> errorInfo(createErrorInfo);
			if (errorInfo)
				::SetErrorInfo(0, errorInfo);
			return hr;
		}

		static std::string Translate(HRESULT hr)
		{
			Pointer<IErrorInfo> errorInfo;
			auto errorInfoResult = ::GetErrorInfo(0, &errorInfo);
			if (errorInfoResult != S_OK)
				return GetWindowsErrorDescription(hr);
			std::string description;
			auto descriptionResult = errorInfo->GetDescription(Get(description));
			return FAILED(descriptionResult) ?
				"IErrorInfo::GetDescription failed." :
				description;
		}

		static std::string GetWindowsErrorDescription(HRESULT hr)
		{
			char* errorMessage = nullptr;
			auto formatResult = ::FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				nullptr,
				hr,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				reinterpret_cast<LPSTR>(&errorMessage),
				0,
				nullptr);
			std::string description = formatResult != 0 ?
				errorMessage :
				"Windows error description was not found.";
			if (errorMessage)
				::LocalFree(errorMessage);
			return description;
		}

	private:
		static std::string Format(HRESULT hr, const std::string& function, const std::string& location, const std::string& description)
		{
			std::ostringstream out;
			out << "HRESULT:     " << hr << " (0x" << std::setw(8) << std::setfill('0') << std::hex << hr << ")" << std::endl
				<< "Function:    " << function << std::endl
				<< "Location:    " << location << std::endl
				<< "Description: " << description << std::endl;
			return out.str();
		}
	};

	inline void CheckError(HRESULT hr, const char* function, const char* location)
	{
		if (FAILED(hr))
			throw Error(hr, function, location, Error::Translate(hr));
	}
}
