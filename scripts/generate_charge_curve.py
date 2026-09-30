#!/usr/bin/env python3
"""Генерация кривой разряда LiFePO4 для расчёта процента зарядки.

Источник данных — открытый научный датасет Национального университета
науки и технологий POLITEHNICA Bucharest (IEEE DataPort, DOI
10.21227/cm0f-jg66, CC BY 4.0): кривые заряда/разряда десяти ячеек
LiFePO4. Используется самый медленный профиль разряда DC3 (C/3) пяти
ячеек BSE (1500 мА*ч, номинал 3.2 В, заряд до 3.65 В — как в нашей
8s-сборке с максимумом 29.2 В).

Скрипт строит усреднённую по пяти ячейкам зависимость «процент
зарядки — напряжение ячейки» на сетке 0..100 % с шагом 5 % и
записывает два файла:

  src/device/battery/charge_curve.csv      — точки кривой (исходные данные);
  src/device/battery/charge_curve_table.h  — таблица для линейной
                                             интерполяции в прошивке.

Использование:
  python3 scripts/generate_charge_curve.py <путь-к-csv-папке-датасета>

Датасет: git clone https://github.com/BulyK47/battery-datasets
(папка charge-discharge-voltage-curves/csv).
"""

import csv
import os
import sys

# Сетка процента зарядки, % (шаг 5 % -> 21 точка).
SOC_STEP_PERCENT = 5

# Обрабатываемые ячейки: BSE 001-005 (LiFePO4, 3.65 В заряда, как в 8s-сборке).
BSE_CELLS = ["bse001", "bse002", "bse003", "bse004", "bse005"]

# Префикс имени файла разряда в датасете.
DCHG_PREFIX = "charge_discharge_voltage_curves__dchg_"

# Расширение имени файла.
CSV_EXT = ".csv"


# Читает пару (время [ч], напряжение [В]) профиля DC3 одной ячейки.
def read_dc3_curve(csv_dir, cell):
    path = os.path.join(csv_dir, DCHG_PREFIX + cell + CSV_EXT)
    times = []
    volts = []
    with open(path, newline="") as handle:
        reader = csv.reader(handle)
        header = next(reader)
        # Нужны колонки «Timestamp_3 [h]» и «*_DC3 [V]»: время DC3 всегда
        # идёт парой с напряжением DC3 (колонки t1,v1,t2,v2,t3,v3).
        t_col = None
        v_col = None
        for idx, name in enumerate(header):
            if name.strip() == "Timestamp_3 [h]":
                t_col = idx
            if name.strip().endswith("_DC3 [V]"):
                v_col = idx
        if t_col is None or v_col is None:
            raise RuntimeError("нет колонок DC3 в " + path)
        for row in reader:
            # Кривые разной длины: пропускаем строки без данных DC3.
            if len(row) <= max(t_col, v_col):
                continue
            t_raw = row[t_col].strip()
            v_raw = row[v_col].strip()
            if not t_raw or not v_raw:
                continue
            times.append(float(t_raw))
            volts.append(float(v_raw))
    return times, volts


# Линейно интерполирует напряжение в заданный момент времени.
def voltage_at(times, volts, t):
    if t <= times[0]:
        return volts[0]
    if t >= times[-1]:
        return volts[-1]
    lo = 0
    hi = len(times) - 1
    while hi - lo > 1:
        mid = (lo + hi) // 2
        if times[mid] <= t:
            lo = mid
        else:
            hi = mid
    span = times[hi] - times[lo]
    if span <= 0:
        return volts[lo]
    frac = (t - times[lo]) / span
    return volts[lo] + frac * (volts[hi] - volts[lo])


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)
    csv_dir = sys.argv[1]

    # Напряжения на сетке SOC для каждой ячейки.
    cell_curves = []
    for cell in BSE_CELLS:
        times, volts = read_dc3_curve(csv_dir, cell)
        # Разряд постоянным током: SOC = 100 * (1 - t / t_end),
        # t_end — полный цикл разряда до отсечки 2.5 В.
        t_end = times[-1]
        curve = []
        for soc in range(0, 101, SOC_STEP_PERCENT):
            t = t_end * (1.0 - soc / 100.0)
            curve.append(voltage_at(times, volts, t))
        cell_curves.append(curve)
        print("%s: t=%.2f ч, U(100%%)=%.3f В, U(0%%)=%.3f В" %
              (cell, t_end, curve[-1], curve[0]))

    # Среднее по пяти ячейкам.
    count = len(cell_curves)
    points = []
    for idx, soc in enumerate(range(0, 101, SOC_STEP_PERCENT)):
        avg = sum(curve[idx] for curve in cell_curves) / count
        points.append((float(soc), avg))

    repo = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    # CSV: кривая разряда от 100 % к 0 %.
    csv_path = os.path.join(repo, "src", "device", "battery", "charge_curve.csv")
    with open(csv_path, "w", newline="") as handle:
        handle.write("soc_percent,cell_voltage_v\n")
        handle.write("# LiFePO4 BSE 1500 mAh, профиль DC3 (C/3), среднее по 5 ячейкам\n")
        handle.write("# Источник: IEEE DataPort DOI 10.21227/cm0f-jg66, CC BY 4.0\n")
        for soc, volt in reversed(points):
            handle.write("%.1f,%.4f\n" % (soc, volt))
    print("записан " + csv_path)

    # Таблица для прошивки: напряжение по возрастанию (требование бинарного
    # поиска интерполяции), процент — соответствующий напряжению.
    ascending = sorted(points, key=lambda item: item[1])
    table_path = os.path.join(repo, "src", "device", "battery",
                              "charge_curve_table.h")
    with open(table_path, "w") as handle:
        handle.write("// Сгенерировано скриптом scripts/generate_charge_curve.py — вручную не редактировать.\n")
        handle.write("// Кривая разряда LiFePO4 (BSE 1500 мА*ч, C/3), среднее по 5 ячейкам.\n")
        handle.write("// Источник данных: IEEE DataPort DOI 10.21227/cm0f-jg66 (CC BY 4.0).\n")
        handle.write("#pragma once\n\n")
        handle.write("#include <stddef.h>\n\n")
        handle.write("// Число точек кривой.\n")
        handle.write("static const size_t CHARGE_CURVE_POINTS = %d;\n\n" % len(ascending))
        handle.write("// Напряжение ячейки, В (по возрастанию).\n")
        handle.write("static const float CHARGE_CURVE_CELL_VOLTAGE[CHARGE_CURVE_POINTS] = {\n")
        for idx in range(0, len(ascending), 3):
            chunk = ascending[idx:idx + 3]
            handle.write("    " + " ".join("%.4ff," % volt for _, volt in chunk) + "\n")
        handle.write("};\n\n")
        handle.write("// Оставшийся процент зарядки, % (соответствует напряжению).\n")
        handle.write("static const float CHARGE_CURVE_PERCENT[CHARGE_CURVE_POINTS] = {\n")
        for idx in range(0, len(ascending), 3):
            chunk = ascending[idx:idx + 3]
            handle.write("    " + " ".join("%.1ff," % soc for soc, _ in chunk) + "\n")
        handle.write("};\n")
    print("записан " + table_path)


if __name__ == "__main__":
    main()
