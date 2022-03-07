@echo off
tcc main.c -I"E:\__AUX__\sdk\include" ^
-L"E:\__AUX__\sdk\lib" ^
-o _aux.exe -lopengl32 -lglu32 -lglut32 ^
-v -bench -funsigned-char -fms-extensions ^
-fdollars-in-identifiers -mno-sse ^
-m32 -lmsvcrt -lkernel32 -luser32 -lgdi32 && timeout /T 0 /NOBREAK >nul
:#pelook -o -s _aux.exe
:#del __aux__.exe
:#upx -9 --ultra-brute _aux.exe -o__aux__.exe
if ERRORLEVEL 1 pause