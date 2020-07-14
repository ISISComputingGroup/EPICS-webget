@echo off
setlocal
set "ARCH=%1"
set "TESTPATH=%~dp0webgetApp/src/O.%ARCH%"
if exist "%TESTPATH%\runner.exe" (
    call iocBoot\ioctestwebget\dllPath.bat
    %TESTPATH%\runner.exe --gtest_output=xml:./test-reports/TEST-Webget.xml
) else (
    @echo No tests to run
)
