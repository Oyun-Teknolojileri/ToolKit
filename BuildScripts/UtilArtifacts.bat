@echo off
setlocal enabledelayedexpansion

set "source=.\..\Bin"
set "import_dest=.\..\Utils\Import"
set "packer_dest=.\..\Utils\Packer"

if /i "%1"=="debug" (
    if not exist "%import_dest%" mkdir "%import_dest%"
    if not exist "%packer_dest%" mkdir "%packer_dest%"
	
    copy "%source%\ToolKit_d.lib" "%packer_dest%\ToolKit_util_d.lib"
    copy "%source%\ToolKit_d.dll" "%packer_dest%\ToolKit_util_d.dll"
    copy "%source%\ToolKit_d.pdb" "%packer_dest%\ToolKit_util_d.pdb"

    copy "%source%\ToolKit_d.lib" "%import_dest%\ToolKit_util_d.lib"
    copy "%source%\ToolKit_d.dll" "%import_dest%\ToolKit_util_d.dll"
    copy "%source%\ToolKit_d.pdb" "%import_dest%\ToolKit_util_d.pdb"

    echo Debug files renamed and copied successfully.
) else (

    copy "%source%\ToolKit.lib" "%packer_dest%\ToolKit_util.lib"
    copy "%source%\ToolKit.dll" "%packer_dest%\ToolKit_util.dll"
    copy "%source%\ToolKit.pdb" "%packer_dest%\ToolKit_util.pdb"

    copy "%source%\ToolKit.lib" "%import_dest%\ToolKit_util.lib"
    copy "%source%\ToolKit.dll" "%import_dest%\ToolKit_util.dll"
    copy "%source%\ToolKit.pdb" "%import_dest%\ToolKit_util.pdb"

    echo Release files renamed and copied successfully.
)