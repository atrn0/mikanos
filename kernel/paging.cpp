#include "paging.hpp"

#include <array>

#include "asmfunc.h"

/**
 * 階層ページング
 *
 * pmt4_table (64GiB * 1) ->
 * pdp_table (1GiB * 64 = 64GiB) ->
 * page_directory (2MiB * 512 = 1GiB) ->
 * page (2MiB)
 */

/** 無名namespaceを切るとprivateになる */
namespace {
const uint64_t kPageSize4K = 4096;
const uint64_t kPageSize2M = 512 * kPageSize4K;
const uint64_t kPageSize1G = 512 * kPageSize2M;

// MEMO: alignasとは
alignas(kPageSize4K) std::array<uint64_t, 512> pml4_table;
alignas(kPageSize4K) std::array<uint64_t, 512> pdp_table;
alignas(kPageSize4K)
    std::array<std::array<uint64_t, 512>, kPageDirectoryCount> page_directory;
}  // namespace

/** @brief setup identity mapping page table
 * identity mappingとは、liner addressと
 * physical addressが同じようなメモリアドレスのマッピング
 */
void SetupIdentityPageTable() {
  // pmt4_tableの先頭要素にpdp_tableの先頭アドレスを設定する
  pml4_table[0] = reinterpret_cast<uint64_t>(&pdp_table[0]) | 0x003;
  for (int i_pdpt = 0; i_pdpt < page_directory.size(); ++i_pdpt) {
    pdp_table[i_pdpt] =
        reinterpret_cast<uint64_t>(&page_directory[i_pdpt]) | 0x003;
    for (int i_pd = 0; i_pd < 512; ++i_pd) {
      page_directory[i_pdpt][i_pd] =
          i_pdpt * kPageSize1G + i_pd * kPageSize2M | 0x083;
    }
  }
}
