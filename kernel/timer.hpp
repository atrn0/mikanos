#pragma once

#include <cstdint>

// Local APIC Timer

void InitializeLAPICTimer();
void StartLAPICTimer();
uint32_t LAPICTimerElapsed();
void StopLAPICTimer();
