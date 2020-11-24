//
//  main.c
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gzipfile.h"
#include "nflate.h"
#include "crc32.h"

static bool has_gz_suffix(const char *str) {
    char *ending = strrchr(str, '.');
    if (ending == NULL) { return false; }
    return !strcmp(ending, ".gz");
}

int main(int argc, const char * argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Need a filename.\n");
        printf("Usage: nflate file_to_be_decompressed.gz [out_file_name]\n");
        return 1;
    }
    gzipfile *gzf = read_gzipfile(argv[1]);
    if (gzf == NULL) {
        fprintf(stderr, "Cound't read gzip file.\n");
        return 1;
    }
    
    size_t uncompressed_length = 0;
    uint8_t *uncompressed = nflate(gzf->data, gzf->data_length, &uncompressed_length);
    printf("%s", uncompressed);

    // CRC is on uncompressed data
    if (!doCRC32Check(uncompressed, uncompressed_length, gzf->CRC32)) {
        fprintf(stderr, "CRC32 check did not pass on data.\n");
    }
    
    // write output file
    FILE *out_file;
    // figure out file name
    char *out_file_name = NULL;
    if (argc > 2) { // if one is specified use it
        size_t name_length = strlen(argv[2]);
        out_file_name = malloc(name_length + 1);
        strncpy(out_file_name, argv[2], name_length + 1);
    } else {
        if (gzf->header.FLG.FNAME) { // if gzipped file specifies out file name
            size_t name_length = strlen(gzf->FNAME);
            out_file_name = malloc(name_length + 1);
            strncpy(out_file_name, gzf->FNAME, name_length + 1);
            //printf("%s", out_file_name);
        } else {
            // if the file ends in .gz, just remove the extension
            if (has_gz_suffix(argv[1])) {
                size_t name_length = strlen(argv[1]) - 3;
                out_file_name = malloc(name_length + 1);
                strncpy(out_file_name, argv[1], name_length);
                out_file_name[name_length] = '\0';
            } else { // otherwise use default "result" name
                out_file_name = malloc(6);
                strncpy(out_file_name, "hello", 6);
            }
        }
    }
    out_file = fopen(out_file_name, "w");
    if (out_file == NULL) {
        perror ("The following error occurred\n");
    }
    size_t written_bytes = fwrite(uncompressed, 1, uncompressed_length, out_file);
    if (written_bytes != uncompressed_length) {
        printf("Expected to write %zu bytes, but fwrite returned %zu.\n", uncompressed_length, written_bytes);
    }
    
    if (ferror(out_file)) {
        perror ("Error writing to file.\n");
    }
    fflush(out_file);
    fclose(out_file);
    
    free(out_file_name);
    free(uncompressed);
    free_gzfipfile(gzf);
    return 0;
}
