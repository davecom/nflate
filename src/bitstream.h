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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


typedef struct {
    uint8_t *data;
    size_t byteLength;
    uint64_t bitIndex;
} bitstream;

bitstream *create_bitstream(uint8_t *data, size_t length);

// read from LSB to MSB along byte boundaries
bool read_bit(bitstream *bs);

//read up to 64 bits at a time
uint64_t read_bits(bitstream *bs, int n);

// reversed, so in same ordering as originally in within the byte
uint64_t read_bits_rev(bitstream *bs, int n);


#endif /* bitstream_h */
