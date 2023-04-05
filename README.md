# Rendering techniques with DirectX 11

This repo contains various rendering techniques such as:

- Phong shading
- Shadow mapping
- Dynamic reflections via cubemap
- Fog post process effect
- Bloom post process effect
- Particle system

### Branches

- `master` branch contains a simple Phong shading renderer written in C language. <br />
It contains small math library, Wavefront *.obj parser, supports user input processing and 
camera movement.

	To compile the sample you need to open `dx11.sln` and build `dx11` project.


- `sponza-nofog-nobloom` is the most up to date branch. It uses CMake.

	To compile you need to open top-level `CMakeLists.txt` in Visual Studio.
	To do so you need to `File -> Open -> CMake...` and select `CMakeLists.txt` in the root
	of git repo. After that select a project you want to compile.

Other branches are **obsolete** and I cannot guaranty that they will compile.