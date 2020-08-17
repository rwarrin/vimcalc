@echo off

SET CompilerFlags=-DNO_MALLOC=1 /nologo /Z7 /Od /Oi /fp:fast /FC -WX -W4 -D_CRT_SECURE_NO_WARNINGS -wd4100 -wd4505 -wd4456
SET LinkerFlags=/incremental:no /opt:ref

IF NOT EXIST build mkdir build

pushd build

cl.exe %CompilerFlags% /LD ..\calc\code\calc.cpp /link %LinkerFlags% /EXPORT:Calc /EXPORT:CalcReset /EXPORT:ToBinary /EXPORT:ToHex /EXPORT:FromHex

cl.exe %CompilerFlags% ..\calc\code\driver.cpp /link %LinkerFlags%

popd
