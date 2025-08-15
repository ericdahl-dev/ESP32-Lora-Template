#include "app_logic.h"
#include <cstdio>

ButtonAction classifyPress(uint32_t pressDurationMs) {
  if (pressDurationMs < 100) return ButtonAction::Ignore;
  if (pressDurationMs < 1000) return ButtonAction::ToggleMode;
  if (pressDurationMs < 3000) return ButtonAction::CycleSF;
  return ButtonAction::CycleBW;
}

int cycleIndex(int currentIndex, int size) {
  if (size <= 0) return 0;
  int next = currentIndex + 1;
  if (next >= size) next = 0;
  return next;
}

std::string formatTxMessage(uint32_t sequenceNumber) {
  char buffer[48];
  snprintf(buffer, sizeof(buffer), "PING seq=%lu", (unsigned long)sequenceNumber);
  return std::string(buffer);
}
