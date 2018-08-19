
#include <cstdarg>
#include <cstdio>
#include <string>

#include "Common/Common.h"
#include "Common/CommonTypes.h"
#include "Common/Logging/Log.h"
#include "Common/MsgHandler.h"
#include "Common/StringUtil.h"

#ifdef _WIN32
#include <windows.h>
#endif

bool DefaultMsgHandler(const char* caption, const char* text, bool yes_no, MsgType style);
static MsgAlertHandler msg_handler = DefaultMsgHandler;
static bool AlertEnabled = true;

std::string DefaultStringTranslator(const char* text);
static StringTranslator str_translator = DefaultStringTranslator;

void RegisterMsgAlertHandler(MsgAlertHandler handler)
{
  msg_handler = handler;
}

void RegisterStringTranslator(StringTranslator translator)
{
  str_translator = translator;
}

void SetEnableAlert(bool enable)
{
  AlertEnabled = enable;
}

std::string GetStringT(const char* string)
{
  return str_translator(string);
}

bool MsgAlert(bool yes_no, MsgType style, const char* format, ...)
{
    std::string caption;
  char buffer[2048];

  static std::string info_caption;
  static std::string warn_caption;
  static std::string ques_caption;
  static std::string crit_caption;

  if (!info_caption.length())
  {
    info_caption = str_translator(_trans("Information"));
    ques_caption = str_translator(_trans("Question"));
    warn_caption = str_translator(_trans("Warning"));
    crit_caption = str_translator(_trans("Critical"));
  }

  switch (style)
  {
  case MsgType::Information:
    caption = info_caption;
    break;
  case MsgType::Question:
    caption = ques_caption;
    break;
  case MsgType::Warning:
    caption = warn_caption;
    break;
  case MsgType::Critical:
    caption = crit_caption;
    break;
  }

  va_list args;
  va_start(args, format);
  CharArrayFromFormatV(buffer, sizeof(buffer) - 1, str_translator(format).c_str(), args);
  va_end(args);

  ERROR_LOG(MASTER_LOG, "%s: %s", caption.c_str(), buffer);

    if (msg_handler && (AlertEnabled || style == MsgType::Question || style == MsgType::Critical))
    return msg_handler(caption.c_str(), buffer, yes_no, style);

  return true;
}

bool DefaultMsgHandler(const char* caption, const char* text, bool yes_no, MsgType style)
{
#ifdef _WIN32
  int window_style = MB_ICONINFORMATION;
  if (style == MsgType::Question)
    window_style = MB_ICONQUESTION;
  if (style == MsgType::Warning)
    window_style = MB_ICONWARNING;

  return IDYES == MessageBox(0, UTF8ToTStr(text).c_str(), UTF8ToTStr(caption).c_str(),
                             window_style | (yes_no ? MB_YESNO : MB_OK));
#else
  fprintf(stderr, "%s\n", text);

    return false;
#endif
}

std::string DefaultStringTranslator(const char* text)
{
  return text;
}
