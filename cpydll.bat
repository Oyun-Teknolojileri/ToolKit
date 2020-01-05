xcopy /d /y "%~dp0\Bin\ToolKit.*" "%~dp0\Editor\bin\Release_Editor\netcoreapp3.0"
xcopy /d /y "%~dp0\Bin\ToolKit_d.*" "%~dp0\Editor\bin\Debug_Editor\netcoreapp3.0"
cd "%~dp0\Editor\bin\Debug_Editor\netcoreapp3.0"
del ToolKit.*
ren ToolKit_d.* ToolKit.*

