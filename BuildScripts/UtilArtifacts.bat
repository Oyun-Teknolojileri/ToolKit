@echo off
setlocal enabledelayedexpansion

set "source=.\..\Bin"
set "import_dest=.\..\Utils\Import"
set "packer_dest=.\..\Utils\Packer"

if /i "%1"=="debug" (
    set "postfix=_d"
    if not exist "%import_dest%" mkdir "%import_dest%"
    if not exist "%packer_dest%" mkdir "%packer_dest%"
) else (
    set "postfix="
)

copy "%source%\ToolKit%postfix%.lib" "%packer_dest%\ToolKit%postfix%.lib"
copy "%source%\ToolKit%postfix%.dll" "%packer_dest%\ToolKit%postfix%.dll"
copy "%source%\ToolKit%postfix%.pdb" "%packer_dest%\ToolKit%postfix%.pdb"

copy "%source%\ToolKit%postfix%.lib" "%import_dest%\ToolKit%postfix%.lib"
copy "%source%\ToolKit%postfix%.dll" "%import_dest%\ToolKit%postfix%.dll"
copy "%source%\ToolKit%postfix%.pdb" "%import_dest%\ToolKit%postfix%.pdb"

echo Files renamed and copied successfully.
