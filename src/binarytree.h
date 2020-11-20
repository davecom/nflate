//
//  binarytree.h
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

#ifndef binarytree_h
#define binarytree_h

#include <stdio.h>
#include <stdint.h>

// Binary Tree
typedef struct bt {
    uint16_t value;
    struct bt *left; // 0
    struct bt *right; // 1
} bt;

bt *create_bt(uint16_t value);

void free_bt(bt *node);

#endif /* binarytree_h */
