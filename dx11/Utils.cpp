#include "Utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <debugapi.h>
#include <io.h>
#include <fcntl.h>

void UtilsDebugPrint(const char* fmt, ...)
{
    char out[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(out, sizeof(out), fmt, args);
    va_end(args);
    OutputDebugStringA(out);
    fprintf(stdout, out);
}

void UtilsFatalError(const char* fmt, ...)
{
    char out[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(out, sizeof(out), fmt, args);
    va_end(args);
    fprintf(stderr, out);
    OutputDebugStringA(out);
    ExitProcess(EXIT_FAILURE);
}

const char* UtilsFormatStr(const char* fmt, ...)
{
    static char out[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(out, sizeof(out), fmt, args);
    va_end(args);
    return out;
}

int UtilStrFindLastChar(const char* str, const char ch)
{
    int pos = -1;
    const char* begin = str;
    while (str && *str)
    {
        if (*str == ch)
            pos = (int)(str - begin);
        ++str;
    }
    return pos;
}

void UtilsStrSub(const char* str, uint32_t start, uint32_t end, char out[], uint32_t maxSize)
{
    const uint32_t len = (uint32_t)strlen(str);
    assert(start < len&& end < len);
    uint32_t max = len < maxSize ? len : maxSize - 1;
    max = max < end ? max : end;

    for (uint32_t i = 0; i < max; ++i)
    {
        out[i] = str[i];
    }
    out[max] = 0;
}

std::vector<uint8_t> UtilsReadData(const char* filepath)
{
    FILE* f;
    fopen_s(&f, filepath, "rb");
    if (!f)
    {
        UTILS_FATAL_ERROR("Failed to read data from %s", filepath);
    }

    struct stat sb;
    if (stat(filepath, &sb) == -1)
    {
        UTILS_FATAL_ERROR("Failed to get file stats from %s", filepath);
    }
    std::vector<uint8_t> bytes(sb.st_size);
    fread(&bytes[0], sb.st_size, 1, f);
    fclose(f);
    return bytes;
}

std::wstring UtilsString2WideString(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}
