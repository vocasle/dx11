#pragma once

#include <string>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <functional>

#include <wrl/client.h>
#include <d3d11.h>


#include "Utils.h"

enum class NodeType
{
    Struct,
    Array,
    Float = sizeof(float),
    Float2 = sizeof(float) * 2,
    Float3 = sizeof(float) * 3,
    Float4 = sizeof(float) * 4,
    Float3X3 = sizeof(float) * 3 * 3,
    Float4X4 = sizeof(float) * 4 * 4,
    None,
};

std::string NodeTypeToString(NodeType inType);

struct Node
{
    std::string Name;
    NodeType Type;
    std::vector<Node> Children;

    Node(): Node("", NodeType::None) {}
    Node(const std::string& inName, NodeType inType): Name(inName), Type(inType) {}

    void AddChild(const std::string& inName, NodeType inType)
    {
        assert(Type == NodeType::Array || Type == NodeType::Struct 
            && "Only composite types allowed to have children");
        Node n;
        n.Name = Type == NodeType::Struct
            ? UtilsFormatStr("%s.%s", Name.c_str(), inName.c_str())
            : UtilsFormatStr("s[%lu]", Name.c_str(), Children.size());
        n.Type = inType;
        Children.push_back(n);
    }

    void Print() const
    {
        UtilsDebugPrint("{ Name: %s, Type: %s }\n", Name.c_str(), NodeTypeToString(Type).c_str());
        for (const auto& child : Children)
        {
            child.Print();
        }
    }

    void Visit(std::function<void(const Node& node)> inVisitor) const
    {
        inVisitor(*this);
        for (auto& child : Children)
        {
            child.Visit(inVisitor);
        }
    }
};


class DynamicConstBufferDesc
{
public:
    void AddNode(const Node& inNode)
    {
        mNodes.push_back(inNode);
    }

    void Print() const
    {
        for (const auto& node : mNodes)
        {
            node.Print();
        }
    }

    const std::vector<Node>& GetNodes() const { return mNodes; }

private:
    std::vector<Node> mNodes;
};

class DynamicConstBuffer
{
public:
    DynamicConstBuffer(const DynamicConstBufferDesc& inDesc)
    {
        size_t sz = 0;
        for (auto& node : inDesc.GetNodes())
        {
            auto visitor = [&sz](const Node& node) -> void
            {
                if (node.Type != NodeType::Array && node.Type != NodeType::Struct)
                {
                    sz += static_cast<size_t>(node.Type);
                }
            };
            node.Visit(visitor);
        }
        mBytes.resize(sz);

        size_t offset = 0;
        for (auto& node : inDesc.GetNodes())
        {
            auto visitor = [this, &offset](const Node& node) -> void
            {
                if (node.Type != NodeType::Struct && node.Type != NodeType::Array)
                {
                    mValues[node.Name] = { node.Type, &mBytes[offset] };
                    offset += static_cast<size_t>(node.Type);
                }
                else
                {
                    mValues[node.Name] = { node.Type, &mBytes[offset] };
                }
            };
            node.Visit(visitor);
        }
    }

    ID3D11Buffer* Get() const
    {
        return mBuffer.Get();
    }

    template <typename T, typename S>
    T* operator[](const S& inName)
    {
        for (const auto& [key, value] : mValues)
        {
            if (key == inName)
            {
                return static_cast<T*>(value.Ptr);
            }
        }
        return nullptr;
    }

    template <typename T, typename S>
    T* GetValue(const S& inName) const
    {
        for (const auto& [key, value] : mValues)
        {
            if (key == inName)
            {
                return static_cast<T*>(value.Ptr);
            }
        }
        return nullptr;
    }

    template <typename T, typename S>
    void SetValue(const S& inName, const T& inValue) const
    {
        for (const auto& [key, value] : mValues)
        {
            if (key == inName)
            {
                *static_cast<T*>(value.Ptr) = inValue;
            }
        }
    }

    void UpdateConstantBuffer(ID3D11DeviceContext* context)
    {
	    D3D11_MAPPED_SUBRESOURCE mapped = {};

	    if (FAILED(context->Map(mBuffer.Get(),
		    0,
		    D3D11_MAP_WRITE_DISCARD,
		    0,
		    &mapped)))
	    {
		    UtilsFatalError("ERROR: Failed to map constant buffer\n");
	    }
	    memcpy(mapped.pData, mBytes.data(), mBytes.size());
	    context->Unmap(mBuffer.Get(), 0);
    }

    void CreateConstantBuffer(ID3D11Device* device)
    {
	    D3D11_BUFFER_DESC bufferDesc = {};
	    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	    bufferDesc.ByteWidth = mBytes.size();
	    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

	    if (FAILED(device->CreateBuffer(&bufferDesc, NULL, mBuffer.ReleaseAndGetAddressOf())))
	    {
		    UtilsFatalError("ERROR: Failed to create per frame constants cbuffer\n");
	    }
    }

private:

    struct Value
    {
        NodeType Type;
        void* Ptr;
    };

    std::vector<uint8_t> mBytes;
    std::unordered_map<std::string, Value> mValues;
    Microsoft::WRL::ComPtr<ID3D11Buffer> mBuffer;
};