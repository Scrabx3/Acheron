#pragma once

namespace Acheron::Interface
{
	template <typename T>
	class FlashLogger : public RE::GFxLog
	{
	public:
		void LogMessageVarg(LogMessageType, const char* str, std::va_list a_argList) override
		{
			std::string msg(str ? str : "");
			while (!msg.empty() && msg.back() == '\n')
				msg.pop_back();

			auto length = std::vsnprintf(0, 0, msg.c_str(), a_argList) + 1;
			char* buffer = (char*)malloc(sizeof(*buffer) * length);
			if (!buffer)
				return;
			std::vsnprintf(buffer, length, msg.c_str(), a_argList);

			logger::info("{} -> {}", T::NAME, buffer);
			free(buffer);
		}
	};

	class InvalidFile : public std::exception
	{
		virtual const char* what() const throw()
		{
			return "Unable to open .swf, bad or missing file";
		}
	};

}
