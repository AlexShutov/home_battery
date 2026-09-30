#pragma once

#include <stdint.h>

// Номинальные токи зарядок, А. Номер зарядки соответствует номеру реле.
#define CHARGER_1_CURRENT 25
#define CHARGER_2_CURRENT 25
#define CHARGER_3_CURRENT 15

// Четвёртая зарядка — резерв: номинала нет, расчётом не включается.
#define CHARGER_4_CURRENT 0
