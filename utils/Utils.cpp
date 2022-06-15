#include "Utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <debugapi.h>
#include <io.h>
#include <sys/stat.h>
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

std::vector<uint8_t> UtilsReadData(const char* filepath)
{
    FILE* f;
    fopen_s(&f, filepath, "rb");
    if (!f)
    {
        UtilsDebugPrint("ERROR: Failed to read data from %s", filepath);
        return {};
    }

    struct stat sb = {};
    if (stat(filepath, &sb) == -1)
    {
        UtilsDebugPrint("ERROR: Failed to get file stats from %s", filepath);
        return {};
    }
    std::vector<uint8_t> bytes(sb.st_size);
    fread(&bytes[0], sb.st_size, 1, f);
    fclose(f);
    return bytes;
}

void UtilsCreateVertexBuffer(ID3D11Device* device, const void* data, size_t num, size_t structSize, ID3D11Buffer** ppBuffer)
{
    D3D11_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pSysMem = data;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = structSize * num;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.StructureByteStride = structSize;
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

    HR(device->CreateBuffer(&bufferDesc, &subresourceData, ppBuffer))
}

void UtilsCreateIndexBuffer(ID3D11Device* device, const void* data, size_t num, ID3D11Buffer** ppBuffer)
{
    D3D11_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pSysMem = data;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(uint32_t) * num;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.StructureByteStride = 0;
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

    HR(device->CreateBuffer(&bufferDesc, &subresourceData, ppBuffer));
}

void UtilsWriteData(const char* filepath, const char* bytes, const size_t sz, const bool isBinary)
{
    FILE* out = nullptr;
    fopen_s(&out, filepath, isBinary ? "wb" : "w");

    if (out)
    {
        for (size_t i = 0; i < sz; ++i)
        {
            fputc(bytes[i], out);
        }

        fclose(out);
    }
}

void UtilsUpdateConstantBuffer(ID3D11DeviceContext* context,
    size_t bufferSize,
    void* data,
    ID3D11Buffer* dest)
{
    D3D11_MAPPED_SUBRESOURCE mapped = {};

    if (FAILED(context->Map((ID3D11Resource*)dest,
        0,
        D3D11_MAP_WRITE_DISCARD,
        0,
        &mapped)))
    {
        UtilsFatalError("ERROR: Failed to map constant buffer\n");
    }
    memcpy(mapped.pData, data, bufferSize);
    context->Unmap((ID3D11Resource*)dest, 0);
}

void UtilsCreateConstantBuffer(ID3D11Device* device,
    size_t byteWidth,
    ID3D11Buffer** pDest)
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.ByteWidth = byteWidth;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

    if (FAILED(device->CreateBuffer(&bufferDesc, NULL, pDest)))
    {
        UtilsFatalError("ERROR: Failed to create per frame constants cbuffer\n");
    }
}