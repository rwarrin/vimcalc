@echo off

SET CompilerFlags=/nologo /Z7 /Od /Oi /fp:fast /FC /MTd -WX -W4 -D_CRT_SECURE_NO_WARNINGS -wd4100 -wd4505 -wd4456 -wd4611
SET LinkerFlags=/incremental:no /opt:ref

REM SET CompilerFlags=/nologo /O2 /Oi /fp:precise /FC /MT -D_CRT_SECURE_NO_WARNINGS 
REM SET LinkerFlags=/incremental:no /opt:ref

IF NOT EXIST build mkdir build

pushd build

cl.exe %CompilerFlags% /LD ..\calc\code\calc.cpp /link %LinkerFlags% /EXPORT:Calc /EXPORT:CalcReset /EXPORT:ToBinary /EXPORT:ToHex /EXPORT:FromHex /EXPORT:Info
cl.exe %CompilerFlags% /LD ..\calc\code\calc_memory.cpp /link %LinkerFlags% /EXPORT:LoadCalcMemory

cl.exe %CompilerFlags% ..\calc\code\driver.cpp /link %LinkerFlags%

popd
