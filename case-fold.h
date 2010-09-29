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
#ifndef __CASE_FOLD_H__
#define __CASE_FOLD_H__

#ifndef __BYTE__
typedef unsigned char byte;
#define __BYTE__
#endif


/* The nature of UTF-8 means that every node in the trie is either
 * branch or a leaf, there are no branches that also are leaves. */

struct case_fold_leaf_t {
    int folded_length;
    byte folded[4];
};

struct case_fold_branch_t {
    struct case_fold_branch_t *next[255];
    struct case_fold_leaf_t *leaf;
};


void case_fold_map_load(struct case_fold_branch_t *root);
void case_fold_map_add(struct case_fold_branch_t *root, byte *upper, byte *lower);
byte *case_fold_lower(struct case_fold_branch_t *root, byte *word, int length);
struct case_fold_leaf_t *case_fold_find(struct case_fold_branch_t *root, byte *char_start);
void case_fold_map_free(struct case_fold_branch_t *root);

#endif // __CASE_FOLD_H__
