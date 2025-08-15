// Keep this test minimal and host-friendly
#include <unity.h>
#include "app_logic.h"

void test_classifyPress() {
  // Test boundary values
  TEST_ASSERT_EQUAL_INT((int)ButtonAction::Ignore, (int)classifyPress(0));
  TEST_ASSERT_EQUAL_INT((int)ButtonAction::Ignore, (int)classifyPress(50));
  TEST_ASSERT_EQUAL_INT((int)ButtonAction::Ignore, (int)classifyPress(99));

  // Test ToggleMode range
  TEST_ASSERT_EQUAL_INT((int)ButtonAction::ToggleMode, (int)classifyPress(100));
  TEST_ASSERT_EQUAL_INT((int)ButtonAction::ToggleMode, (int)classifyPress(150));
  TEST_ASSERT_EQUAL_INT((int)ButtonAction::ToggleMode, (int)classifyPress(999));

  // Test CycleSF range
  TEST_ASSERT_EQUAL_INT((int)ButtonAction::CycleSF, (int)classifyPress(1000));
  TEST_ASSERT_EQUAL_INT((int)ButtonAction::CycleSF, (int)classifyPress(1500));
  TEST_ASSERT_EQUAL_INT((int)ButtonAction::CycleSF, (int)classifyPress(2999));

  // Test CycleBW range
  TEST_ASSERT_EQUAL_INT((int)ButtonAction::CycleBW, (int)classifyPress(3000));
  TEST_ASSERT_EQUAL_INT((int)ButtonAction::CycleBW, (int)classifyPress(10000));
  TEST_ASSERT_EQUAL_INT((int)ButtonAction::CycleBW, (int)classifyPress(UINT32_MAX));
}

void test_cycleIndex() {
  // Normal cycling
  TEST_ASSERT_EQUAL_INT(1, cycleIndex(0, 3));
  TEST_ASSERT_EQUAL_INT(2, cycleIndex(1, 3));
  TEST_ASSERT_EQUAL_INT(0, cycleIndex(2, 3));

  // Edge cases
  TEST_ASSERT_EQUAL_INT(0, cycleIndex(5, 0));  // size 0
  TEST_ASSERT_EQUAL_INT(0, cycleIndex(-1, 3)); // negative index
  TEST_ASSERT_EQUAL_INT(0, cycleIndex(0, 1));  // size 1
  TEST_ASSERT_EQUAL_INT(0, cycleIndex(10, 1)); // index > size, size 1

  // Large values
  TEST_ASSERT_EQUAL_INT(0, cycleIndex(999, 10));
}

void test_formatTxMessage() {
  // Normal cases
  TEST_ASSERT_EQUAL_STRING("PING seq=0", formatTxMessage(0).c_str());
  TEST_ASSERT_EQUAL_STRING("PING seq=42", formatTxMessage(42).c_str());
  TEST_ASSERT_EQUAL_STRING("PING seq=1", formatTxMessage(1).c_str());

  // Large sequence numbers
  TEST_ASSERT_EQUAL_STRING("PING seq=4294967295", formatTxMessage(UINT32_MAX).c_str());
  TEST_ASSERT_EQUAL_STRING("PING seq=1000000", formatTxMessage(1000000).c_str());
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_classifyPress);
  RUN_TEST(test_cycleIndex);
  RUN_TEST(test_formatTxMessage);
  return UNITY_END();
}
