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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "case-fold.h"

/**
 * Add an uppercase -> lowercase mapping to a case folding trie.
 *
 * @param root the case folding trie to operate on
 * @param upper uppercase byte sequence
 * @param lower corresponding lowercase byte sequence
 */
void case_fold_map_add(struct case_fold_branch_t *root, byte *upper, byte *lower) {
    int i;
    int upper_length = strlen((char *) upper);
    int lower_length = strlen((char *) lower);

    assert(upper_length <= 4);
    assert(lower_length <= 4);

    for (i = 0; i < upper_length; i++) {
        if (root->next[upper[i]] == NULL) {
            root->next[upper[i]] = (struct case_fold_branch_t *) calloc(1, sizeof(struct case_fold_branch_t));
        }
        root = root->next[upper[i]];
    }
    /* Now we're at the last branch, add the leaf info */
    root->leaf = (struct case_fold_leaf_t *) calloc(1, sizeof(struct case_fold_leaf_t));
    root->leaf->folded_length = lower_length;
    memcpy(root->leaf->folded, lower, lower_length);
}


/**
 * Find a leaf that holds the lowercase byte sequence corresponding to the uppercase
 * sequence that starts at char_start
 *
 * @param root the case folding trie to operate on
 * @param char_start the start of the uppercase sequence
 * @return NULL if there's no match
 * @return case_fold_leaf_t containing lowercase sequence and length if there's a match
 */
struct case_fold_leaf_t *case_fold_find(struct case_fold_branch_t *root, byte *char_start) {
    while (1) {
        if (root->next[*char_start]) {
            root = root->next[*char_start];
            char_start++;
        }
        else {
            if (root->leaf) {
                return root->leaf;
            } 
            else {
                return NULL;
            }
        }
    }

}

/**
 * Return a copy of a word, converted to lowercase
 *
 * @param root the case folding trie to operate on
 * @param word the word to convert
 * @param length word length (in bytes)
 * @return copy of word, lowercased
 */
byte *case_fold_lower(struct case_fold_branch_t *root, byte *word, int length) {
    byte *out = (byte *) calloc(1, length + 1); /* allocate an extra byte for trailing null */
    int i = 0;
    while (i < length) {
        struct case_fold_leaf_t *leaf = case_fold_find(root, word + i);
        if (leaf == NULL) {
            out[i] = word[i];
            i++;
        }
        else {
            /* @todo: adjust length when leaf->folded_length is different
             * than the byte-length of the char that starts at out + i */
            memcpy(out + i, leaf->folded, leaf->folded_length);
            i += leaf->folded_length;
        }
    }
    return out;
}

/**
 * Free a case folding trie and all its children
 *
 * @param root the case folding trie to free
 */
void case_fold_map_free(struct case_fold_branch_t *root) {
    int i;
    for (i = 0; i < 255; i++) {
        if (root->next[i] != NULL) {
            case_fold_map_free(root->next[i]);
        }
    }
    if (root->leaf) {
        free(root->leaf);
    }
    free(root);
}
