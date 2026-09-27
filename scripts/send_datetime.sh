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

# Ждём начала минуты, чтобы секунды (стартующие с 00) совпали с реальными.
SECOND=$(date "+%S")
WAIT=$(( (60 - 10#$SECOND + 1) % 60 ))
if [ "$WAIT" -gt 1 ]; then
    echo "Жду начала минуты (${WAIT} с)..."
    sleep "$WAIT"
fi

MONTH=$(date "+%b" | tr '[:lower:]' '[:upper:]')
LINE="datetime: $(date "+%d") $MONTH $(date "+%Y"), $(date "+%H") : $(date "+%M")"

echo "Порт:    $PORT"
echo "Отправка: $LINE"
printf '%s\n' "$LINE" > "$PORT"
echo "Готово."
