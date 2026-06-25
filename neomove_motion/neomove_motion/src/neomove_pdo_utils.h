// copyright 2025 YottaImage. All rights reserved.
#ifndef NEOMOVE_PDO_UTILS_H_
#define NEOMOVE_PDO_UTILS_H_

#include <array>

#include "NeoMove CPlusPlus.h"

namespace neomove_pdo {

constexpr unsigned int kMasterIndex = 0;
constexpr unsigned int kMinScratchWords = 2;

inline int ReadWords(int controller_index, unsigned int u_index,
                     unsigned int g_index, unsigned short* words,
                     unsigned int word_count) {
  if (words == nullptr || word_count == 0 || word_count > kMinScratchWords) {
    return 1;
  }

  // NeoMove SDK may touch more than one 16-bit stack slot for a len=1 PDO.
  // Always use scratch space with at least two words before copying out.
  std::array<unsigned short, kMinScratchWords> pdo_words = {};
  int ret = NM_EtherCATReadPDO(controller_index, kMasterIndex, u_index,
                               g_index, pdo_words.data(), word_count);
  for (unsigned int i = 0; i < word_count; ++i) {
    words[i] = pdo_words[i];
  }
  return ret;
}

inline int WriteWords(int controller_index, unsigned int u_index,
                      unsigned int g_index, const unsigned short* words,
                      unsigned int word_count) {
  if (words == nullptr || word_count == 0 || word_count > kMinScratchWords) {
    return 1;
  }

  std::array<unsigned short, kMinScratchWords> pdo_words = {};
  for (unsigned int i = 0; i < word_count; ++i) {
    pdo_words[i] = words[i];
  }
  return NM_EtherCATWritePDO(controller_index, kMasterIndex, u_index, g_index,
                             pdo_words.data(), word_count);
}

inline int ReadWord(int controller_index, unsigned int u_index,
                    unsigned int g_index, unsigned short* word) {
  return ReadWords(controller_index, u_index, g_index, word, 1);
}

inline int WriteWord(int controller_index, unsigned int u_index,
                     unsigned int g_index, unsigned short word) {
  unsigned short words[1] = {word};
  return WriteWords(controller_index, u_index, g_index, words, 1);
}

}  // namespace neomove_pdo

#endif  // NEOMOVE_PDO_UTILS_H_
