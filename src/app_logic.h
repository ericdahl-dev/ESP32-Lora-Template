#pragma once

#include <stdint.h>
#include <string>

enum class ButtonAction {
  Ignore,
  ToggleMode,
  CycleSF,
  CycleBW
};

ButtonAction classifyPress(uint32_t pressDurationMs);

int cycleIndex(int currentIndex, int size);

std::string formatTxMessage(uint32_t sequenceNumber);
