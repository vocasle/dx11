#include "DynamicConstBuffer.h"
#include "NE_Math.h"

FILE *hLog = nullptr;

struct Material {
	Vec4D Ambient;
};

int main()
{
	fopen_s(&hLog, "log.txt", "w");

	DynamicConstBufferDesc desc;

	Node f("float", NodeType::Float);
	Node f2("float2", NodeType::Float2);
	Node f3("float3", NodeType::Float3);
	desc.AddNode(f);
	desc.AddNode(f2);
	desc.AddNode(f3);

	Node s("material", NodeType::Struct);
	s.AddChild("ambient", NodeType::Float4);
	desc.AddNode(s);
	desc.Print();

	DynamicConstBuffer buffer(desc);
	*buffer.GetValue<float>("float") = 3.14f;
	*buffer.GetValue<Vec2D>("float2") = { 10.0f, 20.0f };
	*buffer.GetValue<Vec4D>("material.ambient") = { 0.5f, 0.5f, 0.5f,
							1.0f };

	Material *mat = buffer.GetValue<Material>("material");
	UtilsDebugPrint("{ ambient: %s }\n", mat->Ambient.ToString().c_str());

	fclose(hLog);
}