@echo off

SET CompilerFlags=/nologo /Z7 /Od /Oi /fp:fast /FC -WX -W4 -D_CRT_SECURE_NO_WARNINGS -wd4100 -wd4505 -wd4456
SET LinkerFlags=/incremental:no /opt:ref

IF NOT EXIST build mkdir build

pushd build

cl.exe %CompilerFlags% ..\calc\code\calc.cpp /link %LinkerFlags%

popd
