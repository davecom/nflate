//
//  nflate.c
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
#include "nflate.h"
#include "bitstream.h"
#include "binarytree.h"

// Based on RFC 1951
// https://tools.ietf.org/html/rfc1951


#define MAX_BITS 16
#define NUM_LIT_LEN_SYMBOLS 288
#define NO_SYMBOL 65535
#define END_OF_BLOCK 256

// generate huffman code tree
// code prior to tree creation adapted from RFC 1951 section 3.2.2
// https://tools.ietf.org/html/rfc1951
static void generate_tree(uint8_t *code_lengths, bt *tree_root, int num_symbols) {
    int bl_count[MAX_BITS] = {0};
    for (int i = 0; i < num_symbols; i++) {
        bl_count[code_lengths[i]]++;
    }
    
    uint16_t code = 0;
    bl_count[0] = 0;
    uint16_t next_code[MAX_BITS + 1] = {0};
    for (uint16_t bits = 1; bits <= MAX_BITS; bits++) {
        code = (code + bl_count[bits-1]) << 1;
        next_code[bits] = code;
    }
    
    for (uint16_t n = 0; n < num_symbols; n++) {
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
                            current->right = bt_create(n);
                        } else {
                            current->right = bt_create(NO_SYMBOL);
                        }
                    }
                    current = current->right;
                } else { // 0, left
                    if (current->left == NULL) {
                        if (i == 0) {
                            current->left = bt_create(n);
                        } else {
                            current->left = bt_create(NO_SYMBOL);
                        }
                    }
                    current = current->left;
                }
            }
            //printf("%d \t %d \t %d\n", code, n, current->value);
            next_code[len]++;
        }
    }
}

static uint16_t get_symbol(bitstream *bs, bt *root) {
    bt *current = root;
    do {
        current = bs_read_bit(bs) ? current->right : current->left;
    } while ((current->left != NULL) || (current->right != NULL));
    return current->value;
}

static void expand(bitstream *bs, bt *len_lit_tree, bt *dist_tree, uint8_t **output_buffer, size_t *output_buffer_length) {
    // must have some starting space; arbitrarily start with 1024 bytes
    size_t insert_location = 0;
    if (*output_buffer == NULL) {
        *output_buffer = realloc(*output_buffer, 1024);
        *output_buffer_length = 1024;
    } else {
        insert_location = *output_buffer_length;
    }
    
    
    uint16_t last_symbol = 0;
    do {
        
        last_symbol = get_symbol(bs, len_lit_tree);
        
        if (last_symbol < 256) { // literal
            if (insert_location == *output_buffer_length) {
                *output_buffer_length *= 2;
                *output_buffer = realloc(*output_buffer, *output_buffer_length);
            }
            (*output_buffer)[insert_location] = (uint8_t)last_symbol;
            insert_location++;
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
                int additional_amount = (int)bs_read_bits_rev(bs, extra_bits);
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
            if (dist_tree == NULL) { // must be fixed distances of 5 bit each (fixed block)
                distance_code = (uint8_t)bs_read_bits_rev(bs, 5);
            } else { // dynamic distance must be read
                distance_code = get_symbol(bs, dist_tree);
            }
            if (distance_code < 4) {
                distance = distance_code + 1;
            } else if (distance_code < 30) {
                int extra_bits = ((distance_code / 2)) - 1;
                int additional_amount = (int)bs_read_bits_rev(bs, extra_bits);
                // distance = 2 ^ (extra_bits + 1) + (2 ^ extra_bits * (distance_code % 2)) + 1
                int starter_distance = (1 << (extra_bits + 1)) + ((1 << extra_bits) * (distance_code % 2)) + 1;
                distance = starter_distance + additional_amount;
            } else {
                fprintf(stderr, "Error, found unexpected distance code > 29.");
            }
            
            // make sure we have enough room
            if ((insert_location + length) >= *output_buffer_length) {
                *output_buffer_length *= 2;
                *output_buffer_length += length;
                *output_buffer = realloc(*output_buffer, *output_buffer_length);
            }
            
            // copy over bytes
            for (int i = 0; i < length; i++) {
                (*output_buffer)[insert_location] = (*output_buffer)[insert_location - distance];
                insert_location++;
            }
            
        } else {
            fprintf(stderr, "Error, found unexpected symbol > 285.");
            break;
        }
        
    } while (last_symbol != END_OF_BLOCK);
    
    // get rid of excess
    *output_buffer_length = insert_location;
    *output_buffer = realloc(*output_buffer, *output_buffer_length);
}

// this is specified by RFC 1951 section 3.2.6
static void nflate_fixed_block(bitstream *bs, uint8_t **output_buffer, size_t *output_buffer_length) {
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
    

    bt *lit_len_tree_root = bt_create(NO_SYMBOL);
    generate_tree(code_lengths, lit_len_tree_root, NUM_LIT_LEN_SYMBOLS);
    
    expand(bs, lit_len_tree_root, NULL, output_buffer, output_buffer_length);
    
    
    free(code_lengths);
    bt_free(lit_len_tree_root);
}

// this is specified by RFC 1951 section 3.2.7
static uint8_t *process_dynamic_huffman_code_lengths(bitstream *bs, bt *huffman_tree, int alphabet_size) {
    uint8_t *code_lengths = calloc(alphabet_size, 1);
    uint16_t symbol = 0;
    int num_processed = 0;
    do {
        symbol = get_symbol(bs, huffman_tree);
        if (symbol < 16) {
            code_lengths[num_processed] = symbol;
            num_processed++;
        } else if (symbol == 16) {
            int extra = ((int)bs_read_bits_rev(bs, 2)) + 3;
            for (int i = 0; i < extra; i++) {
                code_lengths[num_processed] = code_lengths[num_processed-1];
                num_processed++;
            }
        } else if (symbol == 17) {
            int extra = ((int)bs_read_bits_rev(bs, 3)) + 3;
            for (int i = 0; i < extra; i++) {
                code_lengths[num_processed] = 0;
                num_processed++;
            }
        } else if (symbol == 18) {
            int extra = ((int)bs_read_bits_rev(bs, 7)) + 11;
            for (int i = 0; i < extra; i++) {
                code_lengths[num_processed] = 0;
                num_processed++;
            }
        } else {
            fprintf(stderr, "Error, found unexpected symbol > 18 reading lit/length table.\n");
        }
        
    } while (num_processed < alphabet_size);
    
    return code_lengths;
}

// this is specified by RFC 1951 section 3.2.7
static void nflate_dynamic_block(bitstream *bs, uint8_t **output_buffer, size_t *output_buffer_length) {
    // build dynamic tables
    int HLIT = ((int)bs_read_bits_rev(bs, 5)) + 257; // name comes from RFC 1951
    int HDIST = ((int)bs_read_bits_rev(bs, 5)) + 1; // name comes from RFC 1951
    int HCLEN = ((int)bs_read_bits_rev(bs, 4)) + 4; // name comes from RFC 1951
    
    // build code length alphabet
    uint8_t *code_lengths = calloc(19, 1);
    int code_length_indices[19] = {16, 17, 18,
        0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    for (int i = 0; i < HCLEN; i++) {
        uint8_t code_length = bs_read_bits_rev(bs, 3);
        code_lengths[code_length_indices[i]] = code_length;
    }
    
//    for (int i = 0; i < 19; i++) {
//        printf("%d \t %d\n", i, code_lengths[i]);
//    }

    bt *huffman_tree = bt_create(NO_SYMBOL);
    
    generate_tree(code_lengths, huffman_tree, 19);
        
    // build literal/length tree
    uint8_t *lit_len_code_lengths = process_dynamic_huffman_code_lengths(bs, huffman_tree, HLIT);
    bt *lit_len_tree = bt_create(NO_SYMBOL);
    generate_tree(lit_len_code_lengths, lit_len_tree, HLIT);

    // build distance tree
    uint8_t *dist_code_lengths = process_dynamic_huffman_code_lengths(bs, huffman_tree, HDIST);
    bt *dist_tree = bt_create(NO_SYMBOL);
    generate_tree(dist_code_lengths, dist_tree, HDIST);
    
    expand(bs, lit_len_tree, dist_tree, output_buffer, output_buffer_length);
    
    free(code_lengths);
    bt_free(huffman_tree);
    free(lit_len_code_lengths);
    bt_free(lit_len_tree);
    free(dist_code_lengths);
    bt_free(dist_tree);
}

// this is specified by RFC 1951 section 3.2.4
static void nflate_uncompressed_block(bitstream *bs, uint8_t **output_buffer, size_t *output_buffer_length) {
    bs_move_to_boundary(bs);
    // LEN and NLEN are two bytes each, NLEN is 1-complement of LEN
    uint16_t LEN = (uint16_t)bs_read_bits_rev(bs, 16);
    uint16_t NLEN = (uint16_t)bs_read_bits_rev(bs, 16);
    if ((LEN ^ NLEN) != 0xFFFF) {
        fprintf(stderr, "LEN is not the one's complement of NLEN.\n");
    }
    // copy bytes over
    // make room for them
    size_t insert_location = 0;
    if (*output_buffer == NULL) {
        *output_buffer = realloc(*output_buffer, LEN);
        *output_buffer_length = LEN;
    } else {
        insert_location = *output_buffer_length;
        *output_buffer = realloc(*output_buffer, ((*output_buffer_length) + LEN));
        *output_buffer_length = ((*output_buffer_length) + LEN);
    }
    bs_read_bytes(bs, (*output_buffer) + insert_location, LEN);
}

// *compressed* is the DEFLATE compressed data to be inflated
// *length* is the length of that data in bytes
// *result_length* is a pointer to a place to hold the length of the uncompressed data in bytes
// returns the uncompressed data as a byte pointer
uint8_t *nflate(uint8_t *compressed, size_t length, size_t *result_length) {
    uint8_t *reconstituted = NULL;
    
    bitstream *bs = create_bitstream(compressed, length);
    
    bool BFINAL; // name comes from RFC 1951
    do {
        // read block header
        BFINAL = bs_read_bit(bs); // is this the last block?
        uint64_t BTYPE = bs_read_bits_rev(bs, 2); // name comes from RFC 1951
        
        switch (BTYPE) {
            case 0: // uncompressed
                nflate_uncompressed_block(bs, &reconstituted, result_length);
                break;
            case 1: // fixed huffman codes
            {
                nflate_fixed_block(bs, &reconstituted, result_length);
                
//                printf("%s", reconstituted);
                break;
            }
            case 2: // dynamic huffman codes
                nflate_dynamic_block(bs, &reconstituted, result_length);
                
//                printf("%s", reconstituted);
                break;
            case 3: // reserved
                fprintf(stderr, "Error, improper block header.\n");
                break;
        }
        
    } while(BFINAL != true);
    
    return reconstituted;
}
