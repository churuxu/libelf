set OUTPUT=output_stm32
set TARGET_PPREFIX=arm-none-eabi-

rem -Wl,-N
if not exist %OUTPUT% md %OUTPUT%

%TARGET_PPREFIX%gcc  "-mcpu=cortex-m3" -mthumb -fPIC -nostdlib -shared -Wl,-N blink_led_app/app.c -o %OUTPUT%/app.so || goto error

%TARGET_PPREFIX%readelf -a -W %OUTPUT%/app.so > %OUTPUT%/eflinfo.txt
%TARGET_PPREFIX%objdump -d -W %OUTPUT%/app.so > %OUTPUT%/dasm.txt
cd runner_stm32

make -j4 || goto error

cd ..

copy /b /y runner_stm32\build\runner.bin %OUTPUT%\runner.bin

truncate -s 16384 %OUTPUT%\runner.bin

copy /b /y %OUTPUT%\runner.bin + %OUTPUT%\app.so %OUTPUT%\app.bin

rem copy /b /y %OUTPUT%\runner.bin + %OUTPUT%\test.bin %OUTPUT%\app.bin

:error

pause