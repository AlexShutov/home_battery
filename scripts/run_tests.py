# Прогон нативных юнит-тестов перед сборкой прошивки (pre-скрипт для [env:nanoatmega328]).
#
# Правило проекта: прошивка не собирается, если хотя бы один нативный
# юнит-тест (pio test -e native) провален. Скрипт вызывает PlatformIO тем же
# интерпретатором, что и текущая сборка, поэтому работает и в консоли, и в IDE
# независимо от того, есть ли pio в PATH.

import subprocess
import sys

Import("env")

# Служебная цель clean (scons -c) не требует тестов — пропускаем прогон.
skip = bool(env.GetOption("clean"))

if not skip:
    print("Прогон нативных юнит-тестов перед сборкой прошивки...")

    result = subprocess.call(
        [sys.executable, "-m", "platformio", "test", "-e", "native"]
    )

    if result != 0:
        sys.stderr.write("Юнит-тесты провалены — сборка прошивки прервана\n")
        env.Exit(1)
