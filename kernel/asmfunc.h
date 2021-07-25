#pragma once

#include <stdint.h>

extern "C" {
void IoOut32(uint16_t addr, uint32_t data);
uint32_t IoIn32(uint16_t addr);
uint16_t GetCS(void);
void LoadIDT(uint16_t limit, uint64_t offset);
/** @brief Load Global Descriptor Table */
void LoadGDT(uint16_t limit, uint64_t offset);
/** @brief Set Code Segment and Stack Segment */
void SetCSSS(uint16_t cs, uint16_t ss);
/** @brief
 * x86_64の64bitモードでは使われない
 * Segment Registerの設定
 * DS, ES, FS, GS
 */
void SetDSAll(uint16_t value);
/** @brief CR3 は PML4 テーブルの物理アドレスを保持するレジスタ */
void SetCR3(uint64_t value);
}