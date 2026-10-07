#ifndef PARSE_TEMP_INT_H
#define PARSE_TEMP_INT_H

#include <charconv>
#include <cstdint>
#include <string>

namespace OHOS {
namespace PowerMgr {
inline bool ParseTempInt32(const char *text, int32_t &out)
{
    if (text == nullptr || text[0] == '\0') {
        return false;
    }
    const char *last = text + std::char_traits<char>::length(text);
    auto [ptr, ec] = std::from_chars(text, last, out);
    return ec == std::errc{} && ptr == last;
}
} // namespace PowerMgr
} // namespace OHOS
#endif
