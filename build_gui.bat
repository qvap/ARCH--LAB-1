@echo off
echo Сборка GUI приложения...
C:\msys64\msys2_shell.cmd -ucrt64 -defterm -here -no-start -c "g++ -std=c++17 -Wall -Wextra -O2 -IC:/msys64/ucrt64/lib/wx/include/msw-unicode-3.2 -IC:/msys64/ucrt64/include/wx-3.2 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXMSW__ gui_main.cpp -o gui_app.exe -LC:/msys64/ucrt64/lib -pipe -Wl,--subsystem,windows -mwindows -lwx_mswu_core-3.2 -lwx_baseu-3.2 -lwsock32 -liphlpapi"
if exist gui_app.exe (
    echo Сборка завершена успешно!
    echo Запуск приложения...
    start gui_app.exe
) else (
    echo Ошибка сборки!
    pause
)
