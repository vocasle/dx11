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

std::string UtilsFormatStr(const char* fmt, ...)
{
    std::string out;
    va_list args;
    va_start(args, fmt);
    va_list args_copy;
    va_copy(args_copy, args);
    size_t len = vsnprintf(nullptr, 0, fmt, args_copy);
    out.resize(len);
    vsnprintf(&out[0], len + 1, fmt, args);
    va_end(args_copy);
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

unsigned char* UtilsReadData(const char* filepath, unsigned int* bufferSize)
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
    unsigned char* bytes = new unsigned char[sb.st_size];
    fread(bytes, sb.st_size, 1, f);
    fclose(f);
    *bufferSize = sb.st_size;
    return bytes;
}