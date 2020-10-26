PPREFIX=
OUTPUT=output_linux
[ -d ${OUTPUT} ] || mkdir ${OUTPUT}

${PPREFIX}gcc -fPIC -nostdlib -shared -Wl,-N blink_led_app/app.c -o ${OUTPUT}/app.so || goto error

${PPREFIX}readelf -a -W ${OUTPUT}/app.so > ${OUTPUT}/eflinfo.txt

${PPREFIX}gcc runner/main.c runner/api.c  ../src/libelf.c -o ${OUTPUT}/runner || goto error
