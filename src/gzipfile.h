//
//  gzipfile.h
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

// Based on RFC 1952
// https://tools.ietf.org/html/rfc1952

#ifndef gzipfile_h
#define gzipfile_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>


typedef struct {
    bool FTEXT : 1;
    bool FHCRC : 1;
    bool FEXTRA : 1;
    bool FNAME : 1;
    bool FCOMMENT : 1;
    uint8_t reserved: 3;
} gzipflags;

typedef struct {
    uint8_t ID1;
    uint8_t ID2;
    uint8_t CM;
    gzipflags FLG;
    uint32_t MTIME;
    uint8_t XFL;
    uint8_t OS;
} gzipheader;

typedef struct {
    gzipheader header;
    char *FEXTRA;
    char *FNAME;
    char *FCOMMENT;
    uint16_t FHCRC;
    uint8_t *data;
    size_t data_length;
    uint32_t CRC32;
    uint32_t ISIZE;
} gzipfile;

void free_gzfipfile(gzipfile *gzf);

gzipfile *read_gzipfile(const char *name);

#endif /* gzipfile_h */
