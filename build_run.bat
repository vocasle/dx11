@echo off
echo Compiling %TARGET% executable
CALL cmake --build build --target %TARGET%
IF %ERRORLEVEL% NEQ 0 (
	echo Failed to compile %TARGET%
	EXIT 1
)
echo Copying compile_commands.json
CALL copy build\compile_commands.json .
echo Invoking %TARGET%.exe
CALL build\%TARGET%\%TARGET%.exe
