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
#include "gzipfile.h"
#include "nflate.h"
#include "crc32.h"

int main(int argc, const char * argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Need a filename.");
        return 1;
    }
    gzipfile *gzf = read_gzipfile(argv[1]);
    if (gzf == NULL) {
        return 1;
    }
    
    size_t uncompressed_length = 0;
    uint8_t *uncompressed = nflate(gzf->data, gzf->data_length, &uncompressed_length);
    printf("%s", uncompressed);

    // CRC is on uncompressed data
    if (!doCRC32Check(uncompressed, uncompressed_length, gzf->CRC32)) {
        fprintf(stderr, "CRC32 check did not pass on data.");
    }
    
    // write output file
    FILE *out_file;
    out_file = fopen (gzf->FNAME, "w");
    if (out_file == NULL) {
        perror ("The following error occurred");
    }
    size_t written_bytes = fwrite(uncompressed, 1, uncompressed_length, out_file);
    if (written_bytes != uncompressed_length) {
        printf("Expected to write %zu bytes, but fwrite returned %zu.", uncompressed_length, written_bytes);
    }
    
    if (ferror(out_file)) {
        perror ("Error writing to file.");
    }
    fflush(out_file);
    fclose(out_file);
    
    free_gzfipfile(gzf);
    return 0;
}
