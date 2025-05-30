# Анализатор IP-диапазонов

Приложение для анализа диапазонов IP-адресов с консольным и графическим интерфейсом.

## Функциональность

- Анализ диапазона IP-адресов
- Вычисление сетевой маски
- Определение адреса сети и broadcast-адреса
- Получение MAC-адреса устройства
- Графический интерфейс на основе wxWidgets

## Сборка

#### Требования
- MSYS2 с установленным wxWidgets:
```bash
pacman -S mingw-w64-ucrt-x86_64-wxwidgets3.2-msw
```

#### Сборка через batch-файл (рекомендуется)
```cmd
build_gui.bat
```

#### Ручная сборка
```bash
C:\msys64\msys2_shell.cmd -ucrt64 -defterm -here -no-start -c "g++ -std=c++17 -Wall -Wextra -O2 -IC:/msys64/ucrt64/lib/wx/include/msw-unicode-3.2 -IC:/msys64/ucrt64/include/wx-3.2 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXMSW__ gui_main.cpp -o gui_app.exe -LC:/msys64/ucrt64/lib -pipe -Wl,--subsystem,windows -mwindows -lwx_mswu_core-3.2 -lwx_baseu-3.2 -lwsock32 -liphlpapi"
```

## Файлы проекта

- `gui_main.cpp` - GUI версия
- `Makefile.gui` - для сборки GUI версии
- `build_gui.bat` - упрощенная сборка GUI версии