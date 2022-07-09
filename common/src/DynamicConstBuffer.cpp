#include "DynamicConstBuffer.h"

std::string NodeTypeToString(NodeType inType)
{
    switch (inType)
    {
    case NodeType::Struct:
        return "Struct";
    case NodeType::Array:
        return "Array";
    case NodeType::Float:
        return "Float";
    case NodeType::Float2:
        return "Float2";
    case NodeType::Float3:
        return "Float3";
    case NodeType::Float4:
        return "Float4";
    case NodeType::Float3X3:
        return "Float3X3";
    case NodeType::Float4X4:
        return "Float4X4";
    default:
        return "None";
    }
}