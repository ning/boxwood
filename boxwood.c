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
#include "boxwood.h"
#include "case-fold.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>


static byte *default_word_boundary_bytes = NULL;
static byte *default_word_boundary_exceptions = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";
/** Default additional character to define as a word boundary. This
    8-bit character, which is invalid in utf8, is used by bazel as a
    placeholder for stripped html. As this module's implementation
    treats all 8-bit characters as word characters, this token must be
    explicitly set as a boundary. */
static byte default_html_token = '\xc0';

/**
 * Set the entries in the mask indicated by the bytes of text to
 * value.
 */
void set_byte_mask(byte *mask, byte *text, int value) {
    byte *b = text;
    if (mask && text) {
        while (*b) {
            mask[*b] = value;
            b++;
        }
    }
}

/**
 * Allocate and initialize default word boundary bytes to all 7-bit
 * ascii characters except letters, numbers, and underscore.
 */
void bw_initialize_default_word_boundary_bytes() {
    if (default_word_boundary_bytes == NULL) {
        default_word_boundary_bytes = calloc(256, 1);
    }

    /* Initialize word boundary chars: all single bytes < 128 except
       letters, numbers, and underscore */
    memset(default_word_boundary_bytes, 1, 128);
    set_byte_mask(default_word_boundary_bytes, default_word_boundary_exceptions, 0);
    default_word_boundary_bytes[default_html_token] = 1;
}

/**
 * Deallocate default word boundary bytes
 */
void bw_free_default_word_boundary_bytes() {
    if (default_word_boundary_bytes) {
        free(default_word_boundary_bytes);
        default_word_boundary_bytes = NULL;
    }
}

/**
 * Create a word boundary byte mask from the supplied character list
 * and assign it to the trie.
 */
void bw_set_word_boundary_bytes(struct bw_trie_t *trie, byte *text) {
    if (trie->word_boundary_chars && trie->word_boundary_chars != default_word_boundary_bytes) {
        free(trie->word_boundary_chars);
    }
    trie->word_boundary_chars = calloc(256, 1);
    set_byte_mask(trie->word_boundary_chars, text, 1);
}

/**
 * Determine whether a string contains at least one multibyte character.
 * The string must be encoded as UTF-8.
 * @see http://canonical.org/~kragen/strlen-utf8.html
 *
 * @param s the string to scan
 * @param len the length of s in bytes
 * @return integer
 */
static inline int string_is_multibyte(unsigned char *s, int len) {
    int i = 0;
    while (i < len) {
        if ((s[i] & 0x80) > 0) { return 1; }
        i++;
    }
    return 0;
}

/**
 * Compute the length (in bytes) of the UTF-8 character whose
 * first byte is pointed to by s. 
 *
 * @param s byte to check
 * @return integer
 */
static inline int character_length(byte *s) {
    if (s[0] < 0x80) {
        return 1;
    }
    else if ((s[0] & 0xE0) == 0xC0) {
        return 2;
    } 
    else if ((s[0] & 0xF0) == 0xE0) {
        return 3;
    }
    else if ((s[0] & 0xF8) == 0xF0) {
        return 4;
    }
    else {
        // uh oh
        return 1;
    }
}

/**
 * Checks for a word boundary before the current word (at i - 1)
 */
static inline int boundary_before_word(byte *boundaries, int i, byte *bytes) {
    return (! boundaries)            /* no boundaries to check */
        || (i == 0)                  /* string start is a boundary */
        || boundaries[bytes[i - 1]]; /* preceded by a boundary */
}

/**
 * Checks for a word boundary after the current word (at i)
 */
static inline int boundary_after_word(byte *boundaries, int i, int len, byte *bytes) {
    return (! boundaries)        /* no boundaries to check */
        || (i >= len)            /* string end is a boundary */
        || boundaries[bytes[i]]; /* followed by a boundary */
}

/**
 * Does the provided node have a child node for the specified byte?
 *
 * @param node the node to check
 * @param next the possible next byte to check
 * @return int
 */
static inline int bw_node_has_next(struct bw_node_t *node, byte next) {
    return (node->next[next] != NULL);
}

/**
 * Create a new boxwood node. 
 * The new node is marked as a leaf since it starts out having
 * nothing in its "next" array.
 *
 * @return bw_node_t*
 */
static struct bw_node_t *bw_create_node() {
    struct bw_node_t *n = (struct bw_node_t *) calloc(1, sizeof(struct bw_node_t));
    n->has_next = 0;
    n->is_terminal = 0;
    return n;
}

/**
 * Create a new boxwood trie, providing a folding trie if you want the trie
 * to do case-insensitive matching.
 * 
 * @param folding_trie if provided, the replacements will be case-insensitive
 *  as per the mappings in this trie
 * @return bw_trie_t*
 */
struct bw_trie_t *bw_create_trie(struct case_fold_branch_t *folding_trie) {
    struct bw_trie_t *trie = (struct bw_trie_t *) calloc(1, sizeof(struct bw_trie_t));
    trie->root = bw_create_node();
    if (folding_trie) {
        trie->case_insensitive = 1;
        trie->folding_trie = folding_trie;
    } else {
        trie->case_insensitive = 0;
    }

    trie->word_boundary_chars = default_word_boundary_bytes;

    return trie;
}

/**
 * Add a word to the list of words this trie can replace. The text must be a
 * valid UTF-8 string. If the trie has a folding trie, the word will be
 * matched case-insensitively when replacement is done.
 *
 * @param trie the trie to add the word to
 * @param text the text of the word to add
 * @return integer number of nodes added to the trie to support this word
 */
int bw_add_text(struct bw_trie_t *trie, byte *text) {
    byte *text_to_add = (trie->case_insensitive && trie->folding_trie) ? case_fold_lower(trie->folding_trie, text, strlen((char *)text)) : text;
    int added = bw_add_bytes(trie, text_to_add, strlen((char *)text_to_add));
    
    if (text_to_add != text) {
        free(text_to_add);
    }
    
    return added;

}

/**
 * Add a byte sequence to the list of words this trie can replace. Any
 * bytes are allowed and no case folding is applied.
 *
 * @param trie the trie to add the byte sequence to
 * @param bytes the bytes to add
 * @param c the length of the byte sequence
 * @return integer number of nodes added to the trie to support this byte sequence
 */
int bw_add_bytes(struct bw_trie_t *trie, byte *bytes, unsigned int c) {
    unsigned int i = 0, added = 0;
    struct bw_node_t *node = trie->root;

    while (i < c) {
        if (node->next[ bytes[i] ] == NULL) {
            node->next[ bytes[i] ] = bw_create_node();
            added++;
            node->has_next = 1;
        }
        node = node->next[ bytes[i] ];
        i++;
    }
    node->is_terminal = 1;
    return added;
}



/**
 * Helper function for bw_walk_trie, requires
 * explicit level to be specified
 *
 * @param n node to invoke callback on
 * @param level descent level into trie
 * @param callback callback function to invoke on each node
 */
static void bw_walk_trie_proper(struct bw_node_t *n, int level, void (callback)(int, byte)) {
    int i;
    for (i = 0; i < 255; i++) {
        if (n->next[i] != NULL) {
            callback(level, i);
            bw_walk_trie_proper(n->next[i], level + 1, callback);
        }
    }
}

/**
 * Walk a trie, applying a callback to each node
 * 
 * @param trie the trie to walk
 * @param callback the callback to apply
 */
void bw_walk_trie(struct bw_trie_t *trie, void (callback)(int, byte)) {
    bw_walk_trie_proper(trie->root, 0, callback);
}


/**
 * Free the memory allocated for a node and all of its child nodes
 *
 * @param node the node to free
 */
void bw_free_node(struct bw_node_t *node) {
    int i;
    if (NULL == node) {
        return;
    }
    for (i = 0; i < 255; i++) {
        if (node->next[i] != NULL) {
            bw_free_node(node->next[i]);
        }
    }
    free(node);
    return;
}

/**
 * Free the memory allocated for a trie and all of its nodes. This
 * does NOT free the memory allocated for the folding trie the
 * trie may be using, if any.
 *
 * @param trie the trie to free
 */
void bw_free_trie(struct bw_trie_t *trie) {
    if (trie) {
        if (trie->root) {
            bw_free_node(trie->root);
        }
        if (trie->word_boundary_chars && trie->word_boundary_chars != default_word_boundary_bytes) {
            free(trie->word_boundary_chars);
        }
        free(trie);
    }
}
    /*
    start at root of trie and first byte
    if byte is not in trie, go to next byte and repeat
    if byte is in trie, save position as bad_word_start_maybe, advance byte and advance down trie
      while byte is in trie, continue advancing each
      if we get to the point where there is nothing in the next[] array on the trie node, we've reached the end of the
        word, so replace from bad_word_start_maybe+1 to current byte with *s, and resume with root of trie and next byte
      if we get to the point where the next[] array is not empty but does not contain an entry for the current byte
        then we've diverged from the stem that matches, so don't do any replacement and start re-parsing at
        bad_word_start_maybe+1

    */
static int bw_replace_proper(struct bw_node_t *root, byte *modified_bytes, byte *bytes_to_walk, int len, byte replacement, int check_multibyte, byte *word_boundary_chars) {
    int match_start_maybe, i = 0, shrunk_bytes = 0, previous_terminal = 0;
    int endok = 0;
    struct bw_node_t *current_node;

    while (i < len) {
        if ((! bw_node_has_next(root, bytes_to_walk[i])) // no matches at start
            || (! boundary_before_word(word_boundary_chars, i, bytes_to_walk)))
            {
                i++;
            }
        else {
            match_start_maybe = i;
            current_node = root;
            previous_terminal = 0;
            do {
                current_node = current_node->next[bytes_to_walk[i]];
                i++;
                if (current_node->is_terminal
                    && boundary_after_word(word_boundary_chars, i, len, bytes_to_walk))
                    {
                        previous_terminal = i;
                    }
            }
            while (bw_node_has_next(current_node, bytes_to_walk[i]));

            /* If we've stopped advancing because we've reached a character in the
             * text which doesn't match the next byte in the trie -- then check to
             * see if there was a previous node on our traversal that was marked
             * as terminal -- that means even though we didn't match the longer
             * word, there was a previous shorter prefix that we should treat as
             * a match 
             */
            if ((! current_node->is_terminal) && previous_terminal) {
                i = previous_terminal;
            }

            /* if we've reached the end of the word, replace */
            if (((! current_node->has_next) && boundary_after_word(word_boundary_chars, i, len, bytes_to_walk)) || previous_terminal) {
                /* BAZ-23266:
                 * i and match_start_maybe are positions in bytes_to_walk -- the byte array that is not shrunken when
                 * we replace multibyte characters with the single-byte replacement character. So when subtracting one
                 * from the other, we don't need to *also* subtract shrunk_bytes, we just want the difference in position.
                 * shrunk_bytes is only necessary when comparing or correlating positions in bytes_to_walk (which
                 * hasn't been shrunk) and modified_bytes (which may have been shrunk) */
                if (check_multibyte && string_is_multibyte(modified_bytes + match_start_maybe - shrunk_bytes, i - match_start_maybe)) {
                    int j = match_start_maybe - shrunk_bytes + character_length(modified_bytes + match_start_maybe - shrunk_bytes);
                    /* Now j is on the second character */
                    while (j < (i - shrunk_bytes)) {
                        int char_len = character_length(modified_bytes + j);
                        modified_bytes[j] = replacement;
                        memmove(modified_bytes + j + 1, modified_bytes + j + char_len, len - (j + char_len) - shrunk_bytes);
                        j += 1; /* advance j past the 1-byte '*' which is now the current char */
                        shrunk_bytes += char_len - 1;
                    }
                } else {
                    /* BAZ-23266: i and match_start_maybe are both positions in bytes_to_walk -- so there's no need to
                     * subtract shrunk_bytes when calculating the length of the non-multibyte string to replace here */
                    memset(modified_bytes + match_start_maybe + 1 - shrunk_bytes, replacement, i - (match_start_maybe + 1));
                }
            }
            else { /* diverging from stem, start reparsing */
                i = match_start_maybe + 1;
            }
        }
    }

    if (check_multibyte && (shrunk_bytes > 0)) {
        memset(modified_bytes + len - shrunk_bytes, 0, shrunk_bytes);
    }

    return len - shrunk_bytes;

}

  /*
    start at root of trie and first byte
    if byte is not in trie, go to next byte and repeat
    if byte is in trie, save position as bad_word_start_maybe, advance byte and advance down trie
      while byte is in trie, continue advancing each
      if we get to the point where there is nothing in the next[] array on the trie node, we've reached the end of the
        word, so replace from bad_word_start_maybe+1 to current byte with *s, and resume with root of trie and next byte
      if we get to the point where the next[] array is not empty but does not contain an entry for the current byte
        then we've diverged from the stem that matches, so don't do any replacement and start re-parsing at
        bad_word_start_maybe+1

    */
static int bw_exists_proper(struct bw_node_t *root, byte *bytes_to_walk, int len, int check_multibyte, byte *word_boundary_chars) {
    int match_start_maybe, i = 0, shrunk_bytes = 0, previous_terminal = 0;
    int endok = 0;
    struct bw_node_t *current_node;

    while (i < len) {
        if ((! bw_node_has_next(root, bytes_to_walk[i])) // no matches at start
            || (! boundary_before_word(word_boundary_chars, i, bytes_to_walk)))
            {
                i++;
            }
        else {
            match_start_maybe = i;
            current_node = root;
            previous_terminal = 0;
            do {
                current_node = current_node->next[bytes_to_walk[i]];
                i++;
                if (current_node->is_terminal
                    && boundary_after_word(word_boundary_chars, i, len, bytes_to_walk))
                    {
                        previous_terminal = i;
                    }
            }
            while (bw_node_has_next(current_node, bytes_to_walk[i]));

            /* If we've stopped advancing because we've reached a character in the
             * text which doesn't match the next byte in the trie -- then check to
             * see if there was a previous node on our traversal that was marked
             * as terminal -- that means even though we didn't match the longer
             * word, there was a previous shorter prefix that we should treat as
             * a match
             */
            if ((! current_node->is_terminal) && previous_terminal) {
                i = previous_terminal;
            }

            /* if we've reached the end of the word, replace */
            if (((! current_node->has_next) && boundary_after_word(word_boundary_chars, i, len, bytes_to_walk)) || previous_terminal) {
                return 1;
            }
            else { /* diverging from stem, start reparsing */
                i = match_start_maybe + 1;
            }
        }
    }

    return 0;

}

/**
 * Replace sequences in text that match words added to the trie. If the trie
 * has a folding trie, then it is used to make the replacements case-insensitive.
 *
 * @param trie Trie containing words to replace and optional folding trie
 * @param text text to process for replacements
 * @param replacement byte to use for replacements in matches
 * @return byte* newly allocated copy of the text with replacements
 */
byte *bw_replace_text(struct bw_trie_t *trie, byte *text, byte replacement, int wordbound) {
  int text_len = strlen((char *)text);

    byte *modified_text = (byte *) calloc(1, text_len + 1); /* allocate extra byte for trailing null */
    memcpy(modified_text, text, text_len);

    byte *text_to_walk;
    if (trie->case_insensitive && trie->folding_trie) {
        text_to_walk = case_fold_lower(trie->folding_trie, text, text_len);
    } else {
        text_to_walk = text;
    }

    bw_replace_proper(trie->root, modified_text, text_to_walk, text_len, replacement, 1, wordbound ? trie->word_boundary_chars : NULL);

    if (text_to_walk != text) {
        free(text_to_walk);
    }

    return modified_text;

}
/*
* Return 1 is any are found, 0 for false
*/
int bw_exists_text(struct bw_trie_t *trie, byte *text, int wordbound) {
  int text_len = strlen((char *)text);
  int result = 0;

    byte *text_to_walk;
    if (trie->case_insensitive && trie->folding_trie) {
        text_to_walk = case_fold_lower(trie->folding_trie, text, text_len);
    } else {
        text_to_walk = text;
    }

    result = bw_exists_proper(trie->root, text_to_walk, text_len, 1, wordbound ? trie->word_boundary_chars : NULL);

    if (text_to_walk != text) {
        free(text_to_walk);
    }

    return result;
}

/**
 * Replace sub-sequences in the byte sequence that match words added to the trie. No
 * case-sensitivity is applied and the byte sequences are not treated as UTF-8
 * characters.
 *
 * @param trie Trie containing words to replace and optional folding trie
 * @param bytes bytes to process for replacements
 * @param c length of bytes
 * @param replacement byte to use for replacements in matches
 * @return byte* newly allocated copy of the byte sequence with replacements
 */
byte *bw_replace_binary(struct bw_trie_t *trie, byte *bytes, int c, byte replacement, int wordbound) {
    
    byte *modified_bytes = (byte *) calloc(1, c);
    memcpy(modified_bytes, bytes, c);

    bw_replace_proper(trie->root, modified_bytes, bytes, c, replacement, 0, wordbound ? trie->word_boundary_chars : NULL);
    
    return modified_bytes;
}


