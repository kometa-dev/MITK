#include "StringUtilities.h"

#include <dcmtk/dcmdata/dcvrpn.h>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace
{
#ifdef _WIN32
  std::wstring strToWstr(const std::string& str, UINT codePage)
  {
    std::wstring wstr;
    if (auto n = MultiByteToWideChar(codePage, 0, str.c_str(), str.size() + 1, /*dst*/NULL, 0)) {
      wstr.resize(n - 1);
      if (!MultiByteToWideChar(codePage, 0, str.c_str(), str.size() + 1, /*dst*/&wstr[0], n)) {
        wstr.clear();
      }
    }
    return wstr;
  }
  std::string wstrToStr(const std::wstring& wstr, UINT codePage)
  {
    std::string str;
    if (auto n = WideCharToMultiByte(codePage, 0, wstr.c_str(), wstr.size() + 1, /*dst*/NULL, 0, /*defchr*/0, NULL)) {
      str.resize(n - 1);
      if (!WideCharToMultiByte(codePage, 0, wstr.c_str(), wstr.size() + 1, /*dst*/&str[0], n, /*defchr*/0, NULL)) {
        str.clear();
      }
    }
    return str;
  }
  std::string strToStr(const std::string &str, UINT codePageSrc, UINT codePageDst)
  {
    return wstrToStr(strToWstr(str, codePageSrc), codePageDst);
  }
#endif
}

namespace Utilities
{
  std::string convertLocalToUTF8(const std::string& str)
  {
#ifdef _WIN32
    return strToStr(str, CP_ACP, CP_UTF8);
#else
    return str;
#endif
  }

  std::string convertLocalToOEM(const std::string& str)
  {
#ifdef _WIN32
    return strToStr(str, CP_ACP, CP_OEMCP);
#else
    return str;
#endif
  }

  std::string convertUTF8ToLocal(const std::string& str)
  {
#ifdef _WIN32
    return strToStr(str, CP_UTF8, CP_ACP);
#else
    return str;
#endif
  }

#ifdef _WIN32
  std::string convertToUtf8(const std::wstring& wstr)
  {
    return wstrToStr(wstr, CP_UTF8);
  }
#endif

  bool isValidUtf8(const char* str)
  {
    if (!str) {
      return true;
    }

    for (auto bytes = reinterpret_cast<const unsigned char *>(str); *bytes != 0x00;) {
      unsigned int cp = 0;
      int num = 0;
      if ((*bytes & 0x80) == 0x00) {
        // U+0000 to U+007F 
        cp = (*bytes & 0x7F);
        num = 1;
      } else if ((*bytes & 0xE0) == 0xC0) {
        // U+0080 to U+07FF 
        cp = (*bytes & 0x1F);
        num = 2;
      } else if ((*bytes & 0xF0) == 0xE0) {
        // U+0800 to U+FFFF 
        cp = (*bytes & 0x0F);
        num = 3;
      } else if ((*bytes & 0xF8) == 0xF0) {
        // U+10000 to U+10FFFF 
        cp = (*bytes & 0x07);
        num = 4;
      } else {
        return false;
      }

      ++bytes;
      for (int i = 1; i < num; ++i) {
        if ((*bytes & 0xC0) != 0x80) {
          return false;
        }
        cp = (cp << 6) | (*bytes & 0x3F);
        ++bytes;
      }

      if ((cp > 0x10FFFF) ||
        ((cp >= 0xD800) && (cp <= 0xDFFF)) ||
        ((cp <= 0x007F) && (num != 1)) ||
        ((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
        ((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
        ((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4))) {
        return false;
      }
    }

    return true;
  }

  std::string formatPersonName(const std::string& pn, int group)
  {
    std::string r;
    for (int i = 0; i < 3; i++) {
      if (DcmPersonName::getFormattedNameFromString(pn, r, group).good() && !r.empty()) {
        return r;
      }
      // search for any informative group
      if (--group < 0) {
        group = 2;
      }
    }
    return pn;
  }

}
