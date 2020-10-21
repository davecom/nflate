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
#include <string.h>

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
    size_t data_length;
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

typedef struct {
    uint8_t *data;
    size_t byteLength;
    uint64_t bitIndex;
} bitstream;

bitstream *create_bitstream(uint8_t *data, size_t length) {
    bitstream *bs = calloc(1, sizeof(bitstream));
    bs->data = data;
    bs->byteLength = length;
    return bs;
}

// READ FROM LSB TO MSB along BYTE boundaries
bool read_bit(bitstream *bs) {
    return ((bs->data[bs->bitIndex / 8]) >> (bs->bitIndex++ % 8)) & 1;
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


// Binary Tree
typedef struct bt {
    uint16_t value;
    struct bt *left; // 0
    struct bt *right; // 1
} bt;

bt *create_bt(uint16_t value) {
    bt *node = malloc(sizeof(bt));
    if (node == NULL) {
        fprintf(stderr, "Error allocating memory for bt.");
    }
    node->left = NULL;
    node->right = NULL;
    node->value = value;
    return node;
}

void free_bt(bt *node) {
    if (node->left != NULL) {
        free_bt(node->left);
    }
    if (node->right != NULL) {
        free_bt(node->right);
    }
    free(node);
}

#define MAX_BITS 16
#define NUM_LIT_LEN_SYMBOLS 288
#define NO_SYMBOL 65535

// generate huffman code tree
// code adated from RFC 1951 section 3.2.2
// https://tools.ietf.org/html/rfc1951
void generate_tree(uint8_t *code_lengths, bt *tree_root, int num_symbols) {
    int bl_count[MAX_BITS] = {0};
    for (int i = 0; i < num_symbols; i++) {
        bl_count[code_lengths[i]]++;
    }
    
    uint16_t code = 0;
    bl_count[0] = 0;
    uint16_t next_code[MAX_BITS + 1];
    for (uint16_t bits = 1; bits <= MAX_BITS; bits++) {
        code = (code + bl_count[bits-1]) << 1;
        next_code[bits] = code;
    }
    
    // symbol_to_code maps symbol (0-num_symbols) -> code
//    uint16_t *symbol_to_code = calloc(num_symbols, 2);
//    for (int n = 0; n < num_symbols; n++) {
//        uint8_t len = code_lengths[n];
//        if (len != 0) {
//            symbol_to_code[n] = next_code[len];
//            next_code[len]++;
//        }
//    }
    
//    *code_to_symbol = calloc(65536, 2);
//    if (*code_to_symbol == NULL) {
//        fprintf(stderr, "Error allocating memory for code_to_symbol.");
//    }
    
    
//    memset(*code_to_symbol, NO_SYMBOL, 65536 * 2);
    
    
    for (int n = 0; n < num_symbols; n++) {
        uint8_t len = code_lengths[n];
        if (len != 0) {
            uint16_t code = next_code[len];
            // add node to tree
            bt *current = tree_root;
            for (int i = (len - 1); i >= 0; i--) {
                bool bit = (code & (1 << i));
                if (bit) { // 1, right
                    if (current->right == NULL) {
                        if (i == 0) {
                            current->right = create_bt(n);
                        } else {
                            current->right = create_bt(NO_SYMBOL);
                        }
                    }
                    current = current->right;
                } else { // 0, left
                    if (current->left == NULL) {
                        if (i == 0) {
                            current->left = create_bt(n);
                        } else {
                            current->left = create_bt(NO_SYMBOL);
                        }
                    }
                    current = current->left;
                }
            }
            printf("%d \t %d \t %d\n", code, n, current->value);
            next_code[len]++;
        }
    }
    //printf("%d \t %d\n", 64, (*code_to_symbol)[64]);
    
    // free(symbol_to_code);
}

#define END_OF_BLOCK 256

uint16_t get_symbol(bitstream *bs, bt *root) {
    bt *current = root;
    do {
        current = read_bit(bs) ? current->right : current->left;
    } while ((current->left != NULL) || (current->right != NULL));
    return current->value;
}

uint8_t *expand(bitstream *bs, bt *len_lit_tree, bool fixed_distances, size_t *block_length, bt *dist_tree) {
    uint8_t *output = NULL;
    // expand huffman codes based on code_to_symbol
    // read 1 bit at a time and if the item is in the table, you found it
    // you did not find it if you found NO_SYMBOL
    
    size_t buffer_length = 10; // initial amount
    output = malloc(buffer_length);
    size_t actual_length = 0;
    uint16_t last_symbol = 0;
    do {
//        uint16_t bits = 0;
//        // commented out sections are if reversed
////        uint8_t num_bits = 0;
//        do {
////            bits |= (read_bit(bs) << num_bits);
////            num_bits++;
//            bits = (bits << 1) | read_bit(bs);
//            last_symbol = code_to_symbol[bits];
//        } while (last_symbol == NO_SYMBOL);
        
        last_symbol = get_symbol(bs, len_lit_tree);
        
        if (last_symbol < 256) { // literal
            if (actual_length == buffer_length) {
                buffer_length *= 2;
                output = realloc(output, buffer_length);
            }
            output[actual_length] = (uint8_t)last_symbol;
            actual_length++;
        } else if (last_symbol == 256) {
            break; // END_OF_BLOCK symbol
        } else if (last_symbol < 286) { // length, distance pair
            // figure out length
            int length = 0;
            if (last_symbol < 265) {
                length = last_symbol - 257 + 3;
            } else if (last_symbol < 285) {
                int difference = last_symbol - 257;
                int extra_bits = (difference / 4) - 1;
                // length = 2 ^ (extra_bits + 2) + (2 ^ extra_bits * (difference % 4)) + 3
                int additional_amount = (int)read_bits_rev(bs, extra_bits);
                int starter_length = (1 << (extra_bits + 2)) + ((1 << extra_bits) * (difference % 4)) + 3;
                length = starter_length + additional_amount;
            } else if (last_symbol == 285) {
                length = 258;
            } else {
                fprintf(stderr, "Error, found unexpected symbol > 285.");
            }
            
            // figure out distance
            uint16_t distance_code = 0;
            int distance = 0;
            if (fixed_distances) {
                distance_code = (uint8_t)read_bits_rev(bs, 5);
            } else { // dynamic distance must be read
//                uint16_t dist_bits = 0;
//                do {
//        //            bits |= (read_bit(bs) << num_bits);
//        //            num_bits++;
//                    dist_bits = (dist_bits << 1) | read_bit(bs);
//                    distance_code = dist_code_to_symbol[dist_bits];
//                } while (distance_code == NO_SYMBOL);
                distance_code = get_symbol(bs, dist_tree);
            }
            if (distance_code < 4) {
                distance = distance_code + 1;
            } else if (distance_code < 30) {
                int extra_bits = ((distance_code / 2)) - 1;
                int additional_amount = (int)read_bits_rev(bs, extra_bits);
                // distance = 2 ^ (extra_bits + 1) + (2 ^ extra_bits * (distance_code % 2)) + 1
                int starter_distance = (1 << (extra_bits + 1)) + ((1 << extra_bits) * (distance_code % 2)) + 1;
                distance = starter_distance + additional_amount;
            } else {
                fprintf(stderr, "Error, found unexpected distance code > 29.");
            }
            
            // make sure we have enough room
            if ((actual_length + length) >= buffer_length) {
                buffer_length *= 2;
                buffer_length += length;
                output = realloc(output, buffer_length);
            }
            
            // copy over bytes
            for (int i = 0; i < length; i++) {
                output[actual_length] = output[actual_length - distance];
                actual_length++;
            }
            
        } else {
            fprintf(stderr, "Error, found unexpected symbol > 285.");
            break;
        }
        
    } while (last_symbol != END_OF_BLOCK);
    
    *block_length = actual_length;
    output = realloc(output, actual_length);
    
    return output;
}

uint8_t *nflate_fixed_block(bitstream *bs, size_t *block_length) {
    uint8_t *output = NULL;
    
    uint8_t *code_lengths = calloc(NUM_LIT_LEN_SYMBOLS, 1);
    // build fixed table
    for (int i = 0; i < NUM_LIT_LEN_SYMBOLS; i++) {
        if (i < 144) {
            code_lengths[i] = 8;
        } else if (i < 256) {
            code_lengths[i] = 9;
        } else if (i < 280) {
            code_lengths[i] = 7;
        } else { // i < 288
            code_lengths[i] = 8;
        }
    }
    

    bt *lit_len_tree_root = create_bt(NO_SYMBOL);
    generate_tree(code_lengths, lit_len_tree_root, NUM_LIT_LEN_SYMBOLS);
    
//    for (int i = 0; i < 32768; i++) {
//        printf("%d\n", code_to_symbol[i]);
//    }
    
    output = expand(bs, lit_len_tree_root, true, block_length, NULL);
    
    
    free(code_lengths);
    free_bt(lit_len_tree_root);
    return output;
}

uint8_t *process_dynamic_huffman_code_lengths(bitstream *bs, bt *huffman_tree, int alphabet_size) {
    uint8_t *alphabet = calloc(alphabet_size, 1);
    uint16_t last_symbol = 0;
    uint16_t symbol = 0;
//    uint16_t bits = 0;
    int num_processed = 0;
    do {
        symbol = get_symbol(bs, huffman_tree);
//        do {
//            bits = (bits << 1) | read_bit(bs);
//            symbol = code_to_symbol[bits];
//        } while (symbol != 0);
        if (symbol < 16) {
            alphabet[num_processed] = symbol;
            num_processed++;
        } else if (symbol == 16) {
            int extra = ((int)read_bits_rev(bs, 2)) + 3;
            for (int i = 0; i < extra; i++) {
                alphabet[num_processed] = last_symbol;
                num_processed++;
            }
        } else if (symbol == 17) {
            int extra = ((int)read_bits_rev(bs, 3)) + 3;
            for (int i = 0; i < extra; i++) {
                alphabet[num_processed] = 0;
                num_processed++;
            }
        } else if (symbol == 18) {
            int extra = ((int)read_bits_rev(bs, 7)) + 11;
            for (int i = 0; i < extra; i++) {
                alphabet[num_processed] = 0;
                num_processed++;
            }
        } else {
            fprintf(stderr, "Error, found unexpected symbol > 18 reading lit/length table.");
        }
        
        last_symbol = symbol;
    } while (num_processed < alphabet_size);
    
    return alphabet;
}

uint8_t *nflate_dynamic_block(bitstream *bs, size_t *block_length) {
    uint8_t *output = NULL;
    
    // build dynamic tables
    int HLIT = ((int)read_bits_rev(bs, 5)) + 257;
    int HDIST = ((int)read_bits_rev(bs, 5)) + 1;
    int HCLEN = ((int)read_bits_rev(bs, 4)) + 4;
    
    // build code length alphabet
    uint8_t *code_lengths = calloc(19, 1);
    int code_length_indices[19] = {16, 17, 18,
        0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    for (int i = 0; i < HCLEN; i++) {
        uint8_t code_length = read_bits_rev(bs, 3);
        code_lengths[code_length_indices[i]] = code_length;
    }
    
    for (int i = 0; i < 19; i++) {
        printf("%d \t %d\n", i, code_lengths[i]);
    }

    bt *huffman_tree = create_bt(NO_SYMBOL);
    
    generate_tree(code_lengths, huffman_tree, 19);
        
    // build literal/length alphabet
    uint8_t *lit_len_code_lengths = process_dynamic_huffman_code_lengths(bs, huffman_tree, HLIT);
    bt *lit_len_tree = create_bt(NO_SYMBOL);
    generate_tree(lit_len_code_lengths, lit_len_tree, HLIT);
    
    // build distance alphabet
    uint8_t *dist_code_lengths = process_dynamic_huffman_code_lengths(bs, huffman_tree, HDIST);
    bt *dist_tree = create_bt(NO_SYMBOL);
    generate_tree(dist_code_lengths, dist_tree, HDIST);
    
//    for (int i = 0; i < 32768; i++) {
//        printf("%d\n", code_to_symbol[i]);
//    }
    
    output = expand(bs, lit_len_tree, false, block_length, dist_tree);
    
    
    free(code_lengths);
    free_bt(huffman_tree);
    free(lit_len_code_lengths);
    free_bt(lit_len_tree);
    free(dist_code_lengths);
    free_bt(dist_tree);
    return output;
}


uint8_t *nflate(uint8_t *compressed, size_t length, size_t *result_length) {
    uint8_t *reconstituted = NULL;
    
    bitstream *bs = create_bitstream(compressed, length);
    
    bool BFINAL;
    do {
        // read block header
        BFINAL = read_bit(bs); // is this the last block?
        uint64_t BTYPE = read_bits_rev(bs, 2);
        size_t block_length = 0;
    
        uint8_t *block_result = NULL;
        switch (BTYPE) {
            case 0b00: // uncompressed
                break;
            case 0b01: // fixed huffman codes
            {
                block_result = nflate_fixed_block(bs, &block_length);
                
                printf("%s", block_result);
                break;
            }
            case 0b10: // dynamic huffman codes
                block_result = nflate_dynamic_block(bs, &block_length);
                
                printf("%s", block_result);
                break;
            case 0b11: // reserved
                fprintf(stderr, "Error, improper block header.");
                break;
        }
        
        if (reconstituted == NULL) {
            reconstituted = malloc(block_length);
        } else {
            reconstituted = realloc(reconstituted, (*result_length + block_length));
        }
        memcpy(reconstituted + *result_length, block_result, block_length);
        *result_length += block_length;
        free(block_result);
    } while(BFINAL != true);
    
    return reconstituted;
}

// https://en.wikipedia.org/wiki/Cyclic_redundancy_check#CRC-32_algorithm
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
    for (int i = 0; i < length; i++) {
        int lookup_index = (crc32 ^ data[i]) & 0xFF;
        crc32 = (crc32 >> 8) ^ lookup_table[lookup_index];
    }
    crc32 ^= 0xFFFFFFFF;
    return crc32 == crc_check;
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
    if (fseek(input, data_start, SEEK_SET) != 0) {
        fprintf(stderr, "Error seeking to start of data block.");
    }
    long int data_size = data_end - data_start;
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
    
//    uint8_t data[2] = {0b10001000, 0b10001111};
//    bitstream *bs = create_bitstream(data, 2);
//    uint64_t bits1 = read_bits(bs, 3);
//    uint64_t bits2 = read_bits(bs, 3);
//    uint64_t bits3 = read_bits(bs, 3);
//    uint64_t bits4 = read_bits(bs, 4);
//    uint64_t bits5 = read_bits(bs, 3);
//    bs->bitIndex = 0;
//    uint64_t bits6 = read_bits(bs, 4);
//    uint64_t bits7 = read_bits(bs, 10);
    // CRC is on uncompressed data
//    if (!doCRC32Check(gzf->data, gzf->data_length, gzf->CRC32)) {
//        fprintf(stderr, "CRC32 check did not pass on data.");
//    }
    
    free_gzfipfile(gzf);
    return 0;
}
