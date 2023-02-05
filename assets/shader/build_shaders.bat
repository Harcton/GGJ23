
@ECHO OFF
SETLOCAL
CD /D %~dp0

SET BUILD_VERTEX_SHADER=%SPEHSENGINE_ROOT%\scripts\build_vertex_shader.bat
SET BUILD_FRAGMENT_SHADER=%SPEHSENGINE_ROOT%\scripts\build_fragment_shader.bat
SET SHADER_DATA_DIR=%CD%\..\..\bin\data\assets\shader
SET EMBEDDED_PATH=%SPEHSENGINE_ROOT%\SpehsEngine\Graphics\EmbeddedShaders\source


REM Copy shader includes
XCOPY %EMBEDDED_PATH%\*.sh /y
XCOPY %EMBEDDED_PATH%\varying.def.sc /y


REM Clear old shaders
IF EXIST %SHADER_DATA_DIR% RMDIR /S /Q %SHADER_DATA_DIR%
MKDIR %SHADER_DATA_DIR%


CALL %BUILD_VERTEX_SHADER% %CD%     object              %SHADER_DATA_DIR%
CALL %BUILD_VERTEX_SHADER% %CD%     object_skinned      %SHADER_DATA_DIR%

CALL %BUILD_FRAGMENT_SHADER% %CD%   object              %SHADER_DATA_DIR%


PAUSE
EXIT \B 0
