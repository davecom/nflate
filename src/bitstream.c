//
//  bitstream.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitstream.h"

bitstream *create_bitstream(uint8_t *data, size_t length) {
    bitstream *bs = calloc(1, sizeof(bitstream));
    if (bs == NULL) {
        fprintf(stderr, "Error allocating memory for bitsream.");
    }
    bs->data = data;
    bs->byteLength = length;
    return bs;
}

// READ FROM LSB TO MSB along BYTE boundaries
bool read_bit(bitstream *bs) {
    bool answer = ((bs->data[bs->bitIndex / 8]) >> (bs->bitIndex % 8)) & 1;
    bs->bitIndex++;
    return answer;
}

//read up to 64 bits at a time
uint64_t read_bits(bitstream *bs, int n) {
    uint64_t bits = 0;
    for (int i = 0; i < n; i++) {
        bits = (bits << 1) | read_bit(bs);
    }
    return bits;
}

// reversed, so in same ordering as originally in within the byte
uint64_t read_bits_rev(bitstream *bs, int n) {
    uint64_t bits = 0;
    for (int i = 0; i < n; i++) {
        bits |= (read_bit(bs) << i);
    }
    return bits;
}

// read bytes directly into *dest*
void read_bytes(bitstream *bs, uint8_t *dest, size_t length) {
    memcpy(dest, bs->data + (bs->bitIndex / 8), length);
    bs->bitIndex += (length * 8);
}

// go to next byte boundary if not already on one
void move_to_boundary(bitstream *bs) {
    // are we on a boundary?
    if (bs->bitIndex % 8 != 0) {
        // skip any bits until the next byte boundary
        bs->bitIndex = bs->bitIndex + (8 - (bs->bitIndex % 8));
    }
}
