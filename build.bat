set PLATFORM=""
set WORKINGDIR=%cd%

:while
if not "%1"=="" (
	if "%1" == "-h" (
		set HELP=1
	) else if "%1" == "-win" (
		set PLATFORM="win32"
	) else if "%1" == "-web" (
		set PLATFORM="web"
	) else (
		echo Unknown command
		goto :exit
	)

	shift
	goto :while
)


if %PLATFORM% == "win32" (
    if not exist build mkdir build
    cd build
    if not exist win mkdir win
    cd win
    cmake ../../ -G "Visual Studio 17 2022"
    cd %WORKINGDIR%
) else if %PLATFORM% == "web" (
    if not exist ..\..\build mkdir ..\..\build
    cd build
    if not exist web mkdir web
    cd web
    call ..\..\emsdk\emsdk_env.bat activate
    emcmake cmake ../../ -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS="-s USE_GLFW=3 -s ALLOW_MEMORY_GROWTH=1" -DCMAKE_EXECUTABLE_SUFFIX=".html"
    cmake --build .
    cd %WORKINGDIR%
)