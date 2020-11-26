//
//  crc32.c
//  nflate
//
//  Copyright (c) 2020 David Kopec
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stddef.h>
#include "crc32.h"

// This code is adapted from both RFC 1952 and Wikipedia's page on CRC
// https://tools.ietf.org/html/rfc1952#section-8.1.1.6.2
// https://en.wikipedia.org/wiki/Cyclic_redundancy_check#CRC-32_algorithm
// The table code is originally Copyright 1996 L. Peter Deutsch and released under
// a permissive license:
//   Copyright (c) 1996 L. Peter Deutsch
//
//   Permission is granted to copy and distribute this document for any
//   purpose and without charge, including translations into other
//   languages and incorporation into compilations, provided that the
//   copyright notice and this notice are preserved, and that any
//   substantive changes or deletions from the original are clearly
//   marked.
bool doCRC32Check(uint8_t *data, size_t length, uint32_t crc_check) {
    uint32_t crc32 = 0xFFFFFFFF;
    uint32_t lookup_table[256];
    
    // code from RFC 1952 for table
    // https://tools.ietf.org/html/rfc1952#section-8.1.1.6.2
    uint32_t c;
    int n, k;
    for (n = 0; n < 256; n++) {
        c = (uint32_t) n;
        for (k = 0; k < 8; k++) {
            if (c & 1) {
                c = 0xedb88320 ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        lookup_table[n] = c;
    }
    
    // based on Wikipedia pseudocode
    for (size_t i = 0; i < length; i++) {
        int lookup_index = (crc32 ^ data[i]) & 0xFF;
        crc32 = (crc32 >> 8) ^ lookup_table[lookup_index];
    }
    crc32 ^= 0xFFFFFFFF;
    return crc32 == crc_check;
}
