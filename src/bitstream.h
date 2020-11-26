//
//  bitstream.h
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

#ifndef bitstream_h
#define bitstream_h

#include <stdbool.h>
#include <stdint.h>


typedef struct {
    uint8_t *data;
    size_t byteLength;
    uint64_t bitIndex;
} bitstream;

bitstream *create_bitstream(uint8_t *data, size_t length);

// read from LSB to MSB along byte boundaries
bool bs_read_bit(bitstream *bs);

//read up to 64 bits at a time
uint64_t bs_read_bits(bitstream *bs, int n);

// reversed, so in same ordering as originally in within the byte
uint64_t bs_read_bits_rev(bitstream *bs, int n);

// read bytes, incrementing bitIndex with as many as needed
void bs_read_bytes(bitstream *bs, uint8_t *dest, size_t length);

// go to next byte boundary if not already on one
void bs_move_to_boundary(bitstream *bs);

#endif /* bitstream_h */
