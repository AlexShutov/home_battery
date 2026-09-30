#include "charge_target_calculator.h"

#include <stdint.h>

#include "charger_currents.h"
#include "relay_state.h"

// Запас минимального тока, А: суммарный ток включённых зарядок должен
// превышать ток батареи не менее чем на эту величину.
static const float MIN_CURRENT_MARGIN = 5.0f;

// Номиналы зарядок по номерам реле, А. Резервная (нулевая) зарядка расчётом
// минимального тока не включается: добавляет к сумме 0 А.
static const uint8_t CHARGER_CURRENTS[RelayState::NUM_RELAYS] = {
    CHARGER_1_CURRENT, CHARGER_2_CURRENT, CHARGER_3_CURRENT, CHARGER_4_CURRENT};

// Обнуляет все флаги активных зарядок.
static void clearActiveRelays(ChargeControlState& state) {
  for (uint8_t i = 0; i < RelayState::NUM_RELAYS; ++i) {
    state.activeRelays[i] = 0;
  }
}

// Заполняет состояние «все зарядки с ненулевым номиналом включены».
static void activateAllChargers(ChargeControlState& state) {
  for (uint8_t i = 0; i < RelayState::NUM_RELAYS; ++i) {
    state.activeRelays[i] = (CHARGER_CURRENTS[i] != 0) ? 1 : 0;
  }
}

// Включает зарядки по битовой маске: бит i — зарядка i.
static void activateByMask(uint8_t mask, ChargeControlState& state) {
  for (uint8_t i = 0; i < RelayState::NUM_RELAYS; ++i) {
    state.activeRelays[i] = ((mask >> i) & 0x1u) ? 1 : 0;
  }
}

// Суммарный номинальный ток комбинации, А.
static float maskCurrent(uint8_t mask) {
  float total = 0.0f;
  for (uint8_t i = 0; i < RelayState::NUM_RELAYS; ++i) {
    if ((mask >> i) & 0x1u) {
      total += (float)CHARGER_CURRENTS[i];
    }
  }
  return total;
}

// Подбор минимального достаточного тока: полный перебор комбинаций зарядок,
// выбирается комбинация с наименьшей суммой, покрывающей порог. Комбинации
// с резервной (нулевой) зарядкой не рассматриваются: от них суммы не
// прибавляется, реле включать незачем.
static uint8_t findMinimalSufficientMask(float threshold) {
  uint8_t bestMask = 0;
  float bestSum = -1.0f;
  for (uint8_t mask = 0; mask < (1u << RelayState::NUM_RELAYS); ++mask) {
    bool usesReserve = ((mask >> 3) & 0x1u) != 0;
    if (usesReserve) {
      continue;
    }
    float sum = maskCurrent(mask);
    if (sum >= threshold && (bestSum < 0.0f || sum < bestSum)) {
      bestSum = sum;
      bestMask = mask;
    }
  }
  // Ни одна комбинация не покрывает порог — включаем все доступные зарядки.
  if (bestSum < 0.0f) {
    return (1u << 3) - 1u;
  }
  return bestMask;
}

void calculateTargetState(const BatteryState& battery,
                          ChargeControlState& out,
                          bool isMinimalCurrent) {
  // Перегрев: зарядка запрещена, все флаги снимаются.
  if (!battery.isTemperatureOk) {
    out.isChargeOn = false;
    clearActiveRelays(out);
    return;
  }

  if (!isMinimalCurrent) {
    // Полная мощность без подбора: включаем все зарядки. Ток не важен:
    // при токе <= 0 идёт зарядка от внешней зарядки, которой нет в списке
    // реле, — наши зарядки всё равно включаем.
    out.isChargeOn = true;
    activateAllChargers(out);
    return;
  }

  // Минимальный достаточный ток: комбинация с наименьшей суммой номиналов,
  // покрывающая ток батареи с запасом. При разряде (ток < 0) порог
  // отрицательный и достаточна пустая комбинация — зарядки выключены.
  float threshold = battery.current + MIN_CURRENT_MARGIN;
  uint8_t mask = findMinimalSufficientMask(threshold);
  activateByMask(mask, out);
  out.isChargeOn = (mask != 0);
}
