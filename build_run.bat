cmake --build build --target %TARGET%
copy build\compile_commands.json .
build\%TARGET%\%TARGET%.exe
