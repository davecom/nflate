//
//  main.c
//  dflate
//
//  Created by David Kopec on 9/27/20.
//

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// https://tools.ietf.org/html/rfc1951
// https://tools.ietf.org/html/rfc1952

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
    uint32_t CRC32;
    uint32_t ISIZE;
} gzipfile;

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

uint8_t *dflate(void *original) {
    uint8_t *compressed = NULL;
    
    return compressed;
}

uint8_t *nflate(void *compressed) {
    uint8_t *reconstituted = NULL;
    
    return reconstituted;
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
        int i = 0;
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
        int i = 0;
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
    fseek(input, 0, (int)data_start);
    long int data_size = data_end - data_start;
    gzf->data = malloc(data_size);
    fread(gzf->data, data_size, 1, input);
    fread(&gzf->CRC32, 4, 1, input);
    fread(&gzf->ISIZE, 4, 1, input);
    
    fclose(input);
    
    return gzf;
    
error:
    fclose(input);
    free_gzfipfile(gzf);
    return NULL;
}

int main(int argc, const char * argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Need a filename.");
        return 1;
    }
    gzipfile *gzf = read_gzipfile(argv[1]);
    if (gzf == NULL) {
        return 1;
    }
    
    
    free_gzfipfile(gzf);
    return 0;
}
