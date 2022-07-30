#include "ModelLoader.h"

#include <sstream>

int main()
{
	std::ostringstream out;
	out << SPONZA_ROOT << "/sponza.glb";

	ModelLoader ml;
	ml.Load(out.str());
}