set OUTPUT=output_win64
set TARGET_PPREFIX=wsl x86_64-linux-gnu-
set HOST_PPREFIX=x86_64-w64-mingw32-


if not exist %OUTPUT% md %OUTPUT%

%TARGET_PPREFIX%gcc -fPIC -nostdlib -shared -Wl,-N blink_led_app/app.c -o %OUTPUT%/app.so || goto error

%TARGET_PPREFIX%readelf -a -W %OUTPUT%/app.so > %OUTPUT%/eflinfo.txt

%HOST_PPREFIX%gcc runner/main.c runner/api.c  ../src/libelf.c -o %OUTPUT%/runner.exe || goto error


:error

pause