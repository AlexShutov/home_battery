#!/bin/bash
# Отправляет устройству текущее точное время через COM-порт прошивки.
# Устройство ждёт строку вида: datetime: 27 SEPT 2026, 14 : 04
# Секунды не передаются: устройство стартует с 00 секунд, поэтому скрипт
# дожидается начала минуты и отправляет строку на первой секунде.

set -e

# Ищем USB-serial порт: на macOS используется callout-устройство /dev/cu.*
# (запись в /dev/tty.* даёт "Device not configured"), на Linux — /dev/ttyACM*.
PORT=$(ls /dev/cu.usbserial* /dev/cu.usbmodem* /dev/ttyACM* /dev/ttyUSB* 2>/dev/null | head -1)
if [ -z "$PORT" ]; then
    echo "Устройство не найдено: нет /dev/cu.usb*, /dev/ttyACM* или /dev/ttyUSB*"
    exit 1
fi

# Скорость порта — 9600 (так прошивка открывает Serial).
if stty -f "$PORT" 9600 2>/dev/null; then
    : # macOS
else
    stty -F "$PORT" 9600 115200 # Linux: baudrate игнорируется для USB-CDC
fi

# Ждём 56-й секунды минуты: после сброса контроллера и загрузки прошивки
# строка уйдёт в начале следующей минуты (секунды устройства стартуют с 00).
SECOND=$(date "+%S")
WAIT=$(( (56 - 10#$SECOND + 60) % 60 ))
if [ "$WAIT" -gt 1 ]; then
    echo "Жду нужного момента (${WAIT} с)..."
    sleep "$WAIT"
fi

# Открытие порта сбрасывает Nano (DTR через конденсатор на RESET): держим
# дескриптор открытым, ждём загрузчика и инициализации прошивки (~4 с) —
# иначе строка уйдёт во время загрузчика и потеряется.
exec 3<>"$PORT"
sleep 4

MONTH=$(date "+%b" | tr '[:lower:]' '[:upper:]')
LINE="datetime: $(date "+%d") $MONTH $(date "+%Y"), $(date "+%H") : $(date "+%M")"

echo "Порт:    $PORT"
echo "Отправка: $LINE"
printf '%s\n' "$LINE" >&3
exec 3<&- 3>&-
echo "Готово."
