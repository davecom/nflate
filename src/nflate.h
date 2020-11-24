//
//  nflate.h
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

#ifndef nflate_h
#define nflate_h

#include <stdlib.h>
#include <stdint.h>

// Based on RFC 1951
// https://tools.ietf.org/html/rfc1951

// *compressed* is the DEFLATE compressed data to be inflated
// *length* is the length of that data in bytes
// *result_length* is a pointer to a place to hold the length of the uncompressed data in bytes
// returns the uncompressed data as a byte pointer
uint8_t *nflate(uint8_t *compressed, size_t length, size_t *result_length);

#endif /* nflate_h */
