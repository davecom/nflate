//
//  gzipfile.c
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

#include "gzipfile.h"
#include <stdlib.h>

void free_gzfipfile(gzipfile *gzf) {
    if (gzf->FEXTRA != NULL) {
        free(gzf->FEXTRA);
    }
    if (gzf->FNAME != NULL) {
        free(gzf->FNAME);
    }
    if (gzf->FCOMMENT != NULL) {
        free(gzf->FCOMMENT);
    }
    free(gzf);
}

gzipfile *read_gzipfile(const char *name) {
    FILE *input = fopen(name, "rb");
    if (!input) {
        fprintf(stderr, "Can't open %s\n", name);
        return NULL;
    }
    
    gzipfile *gzf = calloc(1, sizeof(gzipfile));
    
    // IDs must be write to be valid gzip file
    gzf->header.ID1 = fgetc(input);
    if (gzf->header.ID1 != 31) { goto error; }
    gzf->header.ID2 = fgetc(input);
    if (gzf->header.ID2 != 139) { goto error; }
    gzf->header.CM = fgetc(input);
    uint8_t flags = fgetc(input);
    gzf->header.FLG.FTEXT = flags & 1;
    gzf->header.FLG.FHCRC = flags & 2;
    gzf->header.FLG.FEXTRA = flags & 4;
    gzf->header.FLG.FNAME = flags & 8;
    gzf->header.FLG.FCOMMENT = flags & 16;
    fread(&gzf->header.MTIME, 4, 1, input);
    gzf->header.XFL = fgetc(input);
    gzf->header.OS = fgetc(input);
    
    if (gzf->header.FLG.FEXTRA) {
        uint16_t XLEN = 0;
        fread(&XLEN, 2, 1, input);
        gzf->FEXTRA = calloc(1, XLEN);
        fread(gzf->FEXTRA, 1, XLEN, input);
    }
    
    if (gzf->header.FLG.FNAME) {
        char temp;
        size_t length = 1;
        char *buffer = malloc(length);
        size_t i = 0;
        do {
            if (i >= length) {
                length *= 2;
                buffer = realloc(buffer, length);
            }
            temp = fgetc(input);
            if (temp == EOF) {
                fprintf(stderr, "Unexpectedly found EOF while reading FNAME.");
                goto error;
            }
            buffer[i] = temp;
            i++;
        } while(temp != '\0');
        gzf->FNAME = realloc(buffer, i);
    }
    
    if (gzf->header.FLG.FCOMMENT) {
        char temp;
        size_t length = 1;
        char *buffer = malloc(length);
        size_t i = 0;
        do {
            if (i >= length) {
                length *= 2;
                buffer = realloc(buffer, length);
            }
            temp = fgetc(input);
            if (temp == EOF) {
                fprintf(stderr, "Unexpectedly found EOF while reading FCOMMENT.");
                goto error;
            }
            buffer[i] = temp;
            i++;
        } while(temp != '\0');
        gzf->FCOMMENT = realloc(buffer, i);
    }
    
    if (gzf->header.FLG.FHCRC) {
        fread(&gzf->FHCRC, 2, 1, input);
    }
    
    long int data_start = ftell(input);
    if (fseek(input, -8, SEEK_END) != 0){
        fprintf(stderr, "Error seeking to end of data block.");
    }
    long int data_end = ftell(input);
    if (fseek(input, data_start, SEEK_SET) != 0) {
        fprintf(stderr, "Error seeking to start of data block.");
    }
    size_t data_size = data_end - data_start;
    gzf->data = malloc(data_size);
    if (!gzf->data) {
        fprintf(stderr, "Error allocating memory for data.");
        goto error;
    }
    gzf->data_length = data_size;
    size_t amountRead = fread(gzf->data, 1, data_size, input);
    if (amountRead != data_size) {
        fprintf(stderr, "Error reading data from file.");
        goto error;
    }
    fread(&gzf->CRC32, 4, 1, input);
    fread(&gzf->ISIZE, 4, 1, input);
    
    fclose(input);
    
    return gzf;
    
error:
    fclose(input);
    free_gzfipfile(gzf);
    return NULL;
}
