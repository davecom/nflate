//
//  binarytree.c
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

#include "binarytree.h"
#include <stdlib.h>

bt *create_bt(uint16_t value) {
    bt *node = malloc(sizeof(bt));
    if (node == NULL) {
        fprintf(stderr, "Error allocating memory for bt.\n");
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
