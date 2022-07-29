
#include <iostream>
#include <sstream>


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


bool DoTheImportThing( const std::string& pFile) {
  // Create an instance of the Importer class
  Assimp::Importer importer;

  // And have it read the given file with some example postprocessing
  // Usually - if speed is not the most important aspect for you - you'll
  // probably to request more postprocessing than we do in this example.
  const aiScene* scene = importer.ReadFile( pFile,
    aiProcess_CalcTangentSpace       |
    aiProcess_Triangulate            |
    aiProcess_JoinIdenticalVertices  |
    aiProcess_SortByPType);

  // If the import failed, report it
  if (nullptr != scene) {
    std::cerr << importer.GetErrorString() << std::endl;
    return false;
  }

  // Now we can access the file's contents.
  std::cout << "Sponza loaded successfully" << std::endl;

  // We're done. Everything will be cleaned up by the importer destructor
  return true;
}

int main()
{
    std::ostringstream out;
    out << ASSETS_ROOT << "/sponza.obj";
    DoTheImportThing(out.str());
    std::cout << "OK" << std::endl;
}