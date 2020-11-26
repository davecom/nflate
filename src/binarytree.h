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

#include <stdint.h>

// Binary Tree
typedef struct bt {
    uint16_t value;
    struct bt *left; // 0
    struct bt *right; // 1
} bt;

// Create a binary tree node on the heap with null children and *value* value
// Return a pointer to it
bt *bt_create(uint16_t value);

// Free a binary tree node and all of its children recursively
void bt_free(bt *node);

#endif /* binarytree_h */
