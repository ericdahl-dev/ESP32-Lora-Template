#include <unity.h>
#include "../src/hardware/hardware_abstraction.h"

using namespace HardwareAbstraction;

void setUp(void) {
    // Initialize before each test
    initialize();
}

void tearDown(void) {
    // Clean up after each test
    deinitialize();
}

// Test hardware initialization
void test_hardware_initialization() {
    deinitialize(); // Start clean
    
    TEST_ASSERT_FALSE(isInitialized());
    TEST_ASSERT_EQUAL(Result::SUCCESS, initialize());
    TEST_ASSERT_TRUE(isInitialized());
    
    // Test double initialization
    TEST_ASSERT_EQUAL(Result::SUCCESS, initialize());
    TEST_ASSERT_TRUE(isInitialized());
}

void test_result_to_string() {
    TEST_ASSERT_EQUAL_STRING("SUCCESS", resultToString(Result::SUCCESS));
    TEST_ASSERT_EQUAL_STRING("ERROR_INIT_FAILED", resultToString(Result::ERROR_INIT_FAILED));
    TEST_ASSERT_EQUAL_STRING("ERROR_NOT_INITIALIZED", resultToString(Result::ERROR_NOT_INITIALIZED));
    TEST_ASSERT_EQUAL_STRING("ERROR_INVALID_PARAMETER", resultToString(Result::ERROR_INVALID_PARAMETER));
    TEST_ASSERT_EQUAL_STRING("ERROR_TIMEOUT", resultToString(Result::ERROR_TIMEOUT));
    TEST_ASSERT_EQUAL_STRING("ERROR_COMMUNICATION_FAILED", resultToString(Result::ERROR_COMMUNICATION_FAILED));
    TEST_ASSERT_EQUAL_STRING("ERROR_HARDWARE_FAULT", resultToString(Result::ERROR_HARDWARE_FAULT));
}

// GPIO Tests
void test_gpio_pin_mode() {
    TEST_ASSERT_EQUAL(Result::SUCCESS, GPIO::pinMode(2, GPIO::Mode::MODE_OUTPUT));
    TEST_ASSERT_EQUAL(Result::SUCCESS, GPIO::pinMode(4, GPIO::Mode::MODE_INPUT));
    TEST_ASSERT_EQUAL(Result::SUCCESS, GPIO::pinMode(5, GPIO::Mode::MODE_INPUT_PULLUP));
    TEST_ASSERT_EQUAL(Result::SUCCESS, GPIO::pinMode(6, GPIO::Mode::MODE_INPUT_PULLDOWN));
    
    // Test invalid pin
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, GPIO::pinMode(99, GPIO::Mode::MODE_OUTPUT));
}

void test_gpio_pin_mode_not_initialized() {
    deinitialize();
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, GPIO::pinMode(2, GPIO::Mode::MODE_OUTPUT));
}

void test_gpio_digital_write() {
    GPIO::pinMode(2, GPIO::Mode::MODE_OUTPUT);
    
    TEST_ASSERT_EQUAL(Result::SUCCESS, GPIO::digitalWrite(2, GPIO::Level::LEVEL_HIGH));
    TEST_ASSERT_EQUAL(Result::SUCCESS, GPIO::digitalWrite(2, GPIO::Level::LEVEL_LOW));
    
    // Test invalid pin
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, GPIO::digitalWrite(99, GPIO::Level::LEVEL_HIGH));
}

void test_gpio_digital_write_not_initialized() {
    deinitialize();
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, GPIO::digitalWrite(2, GPIO::Level::LEVEL_HIGH));
}

void test_gpio_digital_read() {
    GPIO::pinMode(4, GPIO::Mode::MODE_INPUT);
    
    // Digital read should work (returns mock value in test environment)
    GPIO::Level level = GPIO::digitalRead(4);
    TEST_ASSERT_TRUE(level == GPIO::Level::LEVEL_LOW || level == GPIO::Level::LEVEL_HIGH);
    
    // Test invalid pin (should return LEVEL_LOW)
    TEST_ASSERT_EQUAL(GPIO::Level::LEVEL_LOW, GPIO::digitalRead(99));
}

void test_gpio_digital_read_not_initialized() {
    deinitialize();
    TEST_ASSERT_EQUAL(GPIO::Level::LEVEL_LOW, GPIO::digitalRead(4));
}

void test_gpio_interrupt() {
    auto dummy_callback = []() {};
    
    TEST_ASSERT_EQUAL(Result::SUCCESS, GPIO::attachInterrupt(0, dummy_callback, 1));
    TEST_ASSERT_EQUAL(Result::SUCCESS, GPIO::detachInterrupt(0));
    
    // Test invalid parameters
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, GPIO::attachInterrupt(99, dummy_callback, 1));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, GPIO::attachInterrupt(0, nullptr, 1));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, GPIO::detachInterrupt(99));
}

void test_gpio_interrupt_not_initialized() {
    deinitialize();
    auto dummy_callback = []() {};
    
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, GPIO::attachInterrupt(0, dummy_callback, 1));
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, GPIO::detachInterrupt(0));
}

// I2C Tests
void test_i2c_initialize() {
    TEST_ASSERT_EQUAL(Result::SUCCESS, I2C::initialize(17, 18, 100000));
    
    // Test invalid pins
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, I2C::initialize(99, 18, 100000));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, I2C::initialize(17, 99, 100000));
}

void test_i2c_initialize_not_initialized() {
    deinitialize();
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, I2C::initialize(17, 18, 100000));
}

void test_i2c_communication() {
    I2C::initialize(17, 18, 100000);
    
    TEST_ASSERT_EQUAL(Result::SUCCESS, I2C::beginTransmission(0x3C));
    TEST_ASSERT_EQUAL(Result::SUCCESS, I2C::write(0x42));
    
    uint8_t data[] = {0x01, 0x02, 0x03};
    TEST_ASSERT_EQUAL(Result::SUCCESS, I2C::write(data, sizeof(data)));
    TEST_ASSERT_EQUAL(Result::SUCCESS, I2C::endTransmission(true));
    
    TEST_ASSERT_EQUAL(Result::SUCCESS, I2C::requestFrom(0x3C, 4));
    
    // Test invalid parameters
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, I2C::beginTransmission(0xFF));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, I2C::write(nullptr, 5));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, I2C::write(data, 0));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, I2C::requestFrom(0xFF, 4));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, I2C::requestFrom(0x3C, 0));
}

void test_i2c_communication_not_initialized() {
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, I2C::beginTransmission(0x3C));
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, I2C::write(0x42));
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, I2C::endTransmission(true));
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, I2C::requestFrom(0x3C, 4));
    TEST_ASSERT_EQUAL(0, I2C::available());
    TEST_ASSERT_EQUAL(-1, I2C::read());
}

void test_i2c_reset() {
    I2C::initialize(17, 18, 100000);
    I2C::reset();
    
    // After reset, operations should fail
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, I2C::beginTransmission(0x3C));
}

// SPI Tests
void test_spi_initialize() {
    TEST_ASSERT_EQUAL(Result::SUCCESS, SPI::initialize());
    
    // Test double initialization
    TEST_ASSERT_EQUAL(Result::SUCCESS, SPI::initialize());
}

void test_spi_initialize_not_initialized() {
    deinitialize();
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, SPI::initialize());
}

void test_spi_transaction() {
    SPI::initialize();
    
    SPI::Settings settings = {1000000, 0, 0};
    TEST_ASSERT_EQUAL(Result::SUCCESS, SPI::beginTransaction(settings));
    
    uint8_t result = SPI::transfer(0x42);
    TEST_ASSERT_EQUAL(0x42, result); // Mock echo
    
    uint8_t data[] = {0x01, 0x02, 0x03};
    SPI::transfer(data, sizeof(data));
    
    TEST_ASSERT_EQUAL(Result::SUCCESS, SPI::endTransaction());
}

void test_spi_transaction_not_initialized() {
    SPI::Settings settings = {1000000, 0, 0};
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, SPI::beginTransaction(settings));
    TEST_ASSERT_EQUAL(0, SPI::transfer(0x42));
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, SPI::endTransaction());
}

// PWM Tests
void test_pwm_initialize() {
    TEST_ASSERT_EQUAL(Result::SUCCESS, PWM::initialize(2, 1000));
    
    // Test invalid pin
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, PWM::initialize(99, 1000));
}

void test_pwm_initialize_not_initialized() {
    deinitialize();
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, PWM::initialize(2, 1000));
}

void test_pwm_duty_cycle() {
    PWM::initialize(2, 1000);
    
    TEST_ASSERT_EQUAL(Result::SUCCESS, PWM::setDutyCycle(2, 512));
    TEST_ASSERT_EQUAL(Result::SUCCESS, PWM::setDutyCycle(2, 0));
    TEST_ASSERT_EQUAL(Result::SUCCESS, PWM::setDutyCycle(2, 1023));
    
    // Test invalid parameters
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, PWM::setDutyCycle(99, 512));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, PWM::setDutyCycle(2, 1024));
}

void test_pwm_duty_cycle_not_initialized() {
    deinitialize();
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, PWM::setDutyCycle(2, 512));
}

void test_pwm_stop() {
    PWM::initialize(2, 1000);
    
    TEST_ASSERT_EQUAL(Result::SUCCESS, PWM::stop(2));
    
    // Test invalid pin
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, PWM::stop(99));
}

void test_pwm_stop_not_initialized() {
    deinitialize();
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, PWM::stop(2));
}

// ADC Tests
void test_adc_initialize() {
    TEST_ASSERT_EQUAL(Result::SUCCESS, ADC::initialize());
    
    // Test double initialization
    TEST_ASSERT_EQUAL(Result::SUCCESS, ADC::initialize());
}

void test_adc_initialize_not_initialized() {
    deinitialize();
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, ADC::initialize());
}

void test_adc_read() {
    ADC::initialize();
    
    uint16_t value;
    TEST_ASSERT_EQUAL(Result::SUCCESS, ADC::read(1, value));
    TEST_ASSERT_EQUAL(2048, value); // Mock value
    
    // Test invalid pin
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, ADC::read(99, value));
}

void test_adc_read_not_initialized() {
    uint16_t value;
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, ADC::read(1, value));
}

void test_adc_read_voltage() {
    ADC::initialize();
    
    float voltage;
    TEST_ASSERT_EQUAL(Result::SUCCESS, ADC::readVoltage(1, voltage));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 1.65f, voltage); // Mock calculation
    
    // Test invalid pin
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, ADC::readVoltage(99, voltage));
}

void test_adc_read_voltage_not_initialized() {
    float voltage;
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, ADC::readVoltage(1, voltage));
}

void test_adc_set_resolution() {
    ADC::initialize();
    
    TEST_ASSERT_EQUAL(Result::SUCCESS, ADC::setResolution(12));
    TEST_ASSERT_EQUAL(Result::SUCCESS, ADC::setResolution(10));
    TEST_ASSERT_EQUAL(Result::SUCCESS, ADC::setResolution(9));
    
    // Test invalid resolution
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, ADC::setResolution(8));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, ADC::setResolution(13));
}

void test_adc_set_resolution_not_initialized() {
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, ADC::setResolution(12));
}

// Timer Tests
void test_timer_initialize() {
    TEST_ASSERT_EQUAL(Result::SUCCESS, Timer::initialize());
    
    // Test double initialization
    TEST_ASSERT_EQUAL(Result::SUCCESS, Timer::initialize());
}

void test_timer_initialize_not_initialized() {
    deinitialize();
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, Timer::initialize());
}

void test_timer_time_functions() {
    Timer::initialize();
    
    uint32_t time1 = Timer::millis();
    uint32_t time2 = Timer::millis();
    TEST_ASSERT_GREATER_OR_EQUAL(time1, time2);
    
    uint32_t utime1 = Timer::micros();
    uint32_t utime2 = Timer::micros();
    TEST_ASSERT_GREATER_OR_EQUAL(utime1, utime2);
    
    // Test delay functions (non-blocking in test environment)
    Timer::delay(1);
    Timer::delayMicroseconds(1000);
}

static bool s_timer_called = false;
static void test_timer_callback() {
    s_timer_called = true;
}

void test_timer_software_timers() {
    Timer::initialize();
    s_timer_called = false;
    
    Timer::Handle* timer = Timer::createTimer(10, false, test_timer_callback);
    TEST_ASSERT_NOT_NULL(timer);
    
    TEST_ASSERT_EQUAL(Result::SUCCESS, Timer::startTimer(timer));
    TEST_ASSERT_EQUAL(Result::SUCCESS, Timer::stopTimer(timer));
    TEST_ASSERT_EQUAL(Result::SUCCESS, Timer::deleteTimer(timer));
    
    // Test invalid parameters
    TEST_ASSERT_NULL(Timer::createTimer(10, false, nullptr));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, Timer::startTimer(nullptr));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, Timer::stopTimer(nullptr));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, Timer::deleteTimer(nullptr));
}

void test_timer_software_timers_not_initialized() {
    deinitialize(); // Ensure timer system is not initialized
    TEST_ASSERT_NULL(Timer::createTimer(10, false, test_timer_callback));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, Timer::startTimer(nullptr));
}

// Power Management Tests
void test_power_vext_control() {
    TEST_ASSERT_EQUAL(Result::SUCCESS, Power::enableVext());
    TEST_ASSERT_EQUAL(Result::SUCCESS, Power::disableVext());
}

void test_power_vext_control_not_initialized() {
    deinitialize();
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, Power::enableVext());
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, Power::disableVext());
}

void test_power_sleep() {
    TEST_ASSERT_EQUAL(Result::SUCCESS, Power::sleep(Power::Mode::NORMAL, 1));
    TEST_ASSERT_EQUAL(Result::SUCCESS, Power::sleep(Power::Mode::LIGHT_SLEEP, 0));
    TEST_ASSERT_EQUAL(Result::SUCCESS, Power::sleep(Power::Mode::DEEP_SLEEP, 0));
    
    TEST_ASSERT_EQUAL(Result::SUCCESS, Power::wakeup());
}

void test_power_sleep_not_initialized() {
    deinitialize();
    TEST_ASSERT_EQUAL(Result::ERROR_NOT_INITIALIZED, Power::sleep(Power::Mode::NORMAL, 1));
    TEST_ASSERT_EQUAL(Result::SUCCESS, Power::wakeup()); // Always succeeds
}

void test_power_battery() {
    float voltage = Power::getBatteryVoltage();
    TEST_ASSERT_GREATER_OR_EQUAL(0.0f, voltage);
    
    uint8_t percent = Power::getBatteryPercent();
    TEST_ASSERT_LESS_OR_EQUAL(100, percent);
}

// Memory Management Tests
void test_memory_allocation() {
    void* ptr = Memory::allocate(1024);
    TEST_ASSERT_NOT_NULL(ptr);
    
    Memory::deallocate(ptr);
    Memory::deallocate(nullptr); // Should not crash
}

void test_memory_heap_info() {
    size_t free_heap = Memory::getFreeHeap();
    size_t min_free_heap = Memory::getMinFreeHeap();
    size_t max_alloc_heap = Memory::getMaxAllocHeap();
    
    TEST_ASSERT_GREATER_THAN(0, free_heap);
    TEST_ASSERT_GREATER_THAN(0, min_free_heap);
    TEST_ASSERT_GREATER_THAN(0, max_alloc_heap);
    TEST_ASSERT_LESS_OR_EQUAL(free_heap, min_free_heap);
}

void test_memory_nvs() {
    TEST_ASSERT_EQUAL(Result::SUCCESS, Memory::nvs_open("test"));
    
    uint8_t write_data[] = {0x01, 0x02, 0x03, 0x04};
    TEST_ASSERT_EQUAL(Result::SUCCESS, Memory::nvs_set("key1", write_data, sizeof(write_data)));
    TEST_ASSERT_EQUAL(Result::SUCCESS, Memory::nvs_commit());
    
    uint8_t read_data[4];
    size_t length = sizeof(read_data);
    TEST_ASSERT_EQUAL(Result::SUCCESS, Memory::nvs_get("key1", read_data, length));
    
    TEST_ASSERT_EQUAL(Result::SUCCESS, Memory::nvs_close());
    
    // Test invalid parameters
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, Memory::nvs_open(nullptr));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, Memory::nvs_set(nullptr, write_data, sizeof(write_data)));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, Memory::nvs_set("key", nullptr, sizeof(write_data)));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, Memory::nvs_get(nullptr, read_data, length));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, Memory::nvs_get("key", nullptr, length));
}

void test_memory_nvs_not_initialized() {
    deinitialize();
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, Memory::nvs_open("test"));
}

void test_memory_nvs_not_open() {
    uint8_t data[4];
    size_t length = sizeof(data);
    
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, Memory::nvs_set("key", data, length));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, Memory::nvs_get("key", data, length));
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, Memory::nvs_commit());
    TEST_ASSERT_EQUAL(Result::ERROR_INVALID_PARAMETER, Memory::nvs_close());
}

// System Information Tests
void test_system_info() {
    System::Info info;
    System::getSystemInfo(info);
    
    TEST_ASSERT_NOT_NULL(info.chipModel);
    TEST_ASSERT_GREATER_THAN(0, info.chipRevision);
    TEST_ASSERT_GREATER_THAN(0, info.chipId);
    TEST_ASSERT_GREATER_THAN(0, info.flashSize);
    TEST_ASSERT_GREATER_THAN(0, info.freeHeap);
    TEST_ASSERT_GREATER_OR_EQUAL(0, info.uptime);
    TEST_ASSERT_GREATER_THAN(0.0f, info.cpuFreq);
}

void test_system_watchdog() {
    System::enableWatchdog(5000);
    System::feedWatchdog();
    System::disableWatchdog();
    
    // Should not crash
}

void test_system_restart() {
    // Note: This test cannot actually restart in test environment
    // Just verify the function exists and can be called
    // System::restart(); // Commented out to avoid actual restart
}

// PlatformIO Unity integration
void process() {
    // Hardware initialization tests
    RUN_TEST(test_hardware_initialization);
    RUN_TEST(test_result_to_string);

    // GPIO tests
    RUN_TEST(test_gpio_pin_mode);
    RUN_TEST(test_gpio_pin_mode_not_initialized);
    RUN_TEST(test_gpio_digital_write);
    RUN_TEST(test_gpio_digital_write_not_initialized);
    RUN_TEST(test_gpio_digital_read);
    RUN_TEST(test_gpio_digital_read_not_initialized);
    RUN_TEST(test_gpio_interrupt);
    RUN_TEST(test_gpio_interrupt_not_initialized);

    // I2C tests
    RUN_TEST(test_i2c_initialize);
    RUN_TEST(test_i2c_initialize_not_initialized);
    RUN_TEST(test_i2c_communication);
    RUN_TEST(test_i2c_communication_not_initialized);
    RUN_TEST(test_i2c_reset);

    // SPI tests
    RUN_TEST(test_spi_initialize);
    RUN_TEST(test_spi_initialize_not_initialized);
    RUN_TEST(test_spi_transaction);
    RUN_TEST(test_spi_transaction_not_initialized);

    // PWM tests
    RUN_TEST(test_pwm_initialize);
    RUN_TEST(test_pwm_initialize_not_initialized);
    RUN_TEST(test_pwm_duty_cycle);
    RUN_TEST(test_pwm_duty_cycle_not_initialized);
    RUN_TEST(test_pwm_stop);
    RUN_TEST(test_pwm_stop_not_initialized);

    // ADC tests
    RUN_TEST(test_adc_initialize);
    RUN_TEST(test_adc_initialize_not_initialized);
    RUN_TEST(test_adc_read);
    RUN_TEST(test_adc_read_not_initialized);
    RUN_TEST(test_adc_read_voltage);
    RUN_TEST(test_adc_read_voltage_not_initialized);
    RUN_TEST(test_adc_set_resolution);
    RUN_TEST(test_adc_set_resolution_not_initialized);

    // Timer tests
    RUN_TEST(test_timer_initialize);
    RUN_TEST(test_timer_initialize_not_initialized);
    RUN_TEST(test_timer_time_functions);
    RUN_TEST(test_timer_software_timers);
    RUN_TEST(test_timer_software_timers_not_initialized);

    // Power management tests
    RUN_TEST(test_power_vext_control);
    RUN_TEST(test_power_vext_control_not_initialized);
    RUN_TEST(test_power_sleep);
    RUN_TEST(test_power_sleep_not_initialized);
    RUN_TEST(test_power_battery);

    // Memory management tests
    RUN_TEST(test_memory_allocation);
    RUN_TEST(test_memory_heap_info);
    RUN_TEST(test_memory_nvs);
    RUN_TEST(test_memory_nvs_not_initialized);
    RUN_TEST(test_memory_nvs_not_open);

    // System information tests
    RUN_TEST(test_system_info);
    RUN_TEST(test_system_watchdog);
    RUN_TEST(test_system_restart);
}

#ifdef UNIT_TEST
int main(int argc, char **argv) {
    UNITY_BEGIN();
    process();
    return UNITY_END();
}
#endif