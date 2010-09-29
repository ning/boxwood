/** Copyright 2010 Ning, Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#ifndef __BOXWOOD_H__
#define __BOXWOOD_H__

#ifndef __BYTE__
typedef unsigned char byte;
#define __BYTE__
#endif

#include "case-fold.h"

struct bw_node_t {
    byte has_next;
    byte is_terminal;
    struct bw_node_t *next[255];
};

struct bw_trie_t {
    struct bw_node_t *root;
    int case_insensitive;
    struct case_fold_branch_t *folding_trie;
    byte *word_boundary_chars;
};

struct bw_trie_t *bw_create_trie(struct case_fold_branch_t *folding_trie);
int bw_add_text(struct bw_trie_t *trie, byte *word);
int bw_add_bytes(struct bw_trie_t *trie, byte *bytes, unsigned int c);
byte *bw_replace_text(struct bw_trie_t *trie, byte *text, byte replacement, int wordbound);
byte *bw_replace_binary(struct bw_trie_t *trie, byte *bytes, int c, byte replacement, int wordbound);
void bw_walk_trie(struct bw_trie_t *trie, void (callback)(int, byte));
void bw_free_node(struct bw_node_t *node);
void bw_free_trie(struct bw_trie_t *trie);
void bw_initialize_default_word_boundary_bytes();
void bw_free_default_word_boundary_bytes();
void bw_set_word_boundary_bytes(struct bw_trie_t *trie, byte *text);

#endif /* __BOXWOOD_H_ */
