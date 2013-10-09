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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_boxwood.h"

ZEND_DECLARE_MODULE_GLOBALS(boxwood)

/* True global resources - no need for thread safety here */
static int le_bw_trie;

/* {{{ boxwood_functions[]
 *
 * Every user visible function must have an entry in boxwood_functions[].
 */
zend_function_entry boxwood_functions[] = {
    PHP_FE(boxwood_new, NULL)
    PHP_FE(boxwood_add_text, NULL)
    PHP_FE(boxwood_replace_text, NULL)
    PHP_FE(boxwood_set_word_boundary_bytes, NULL)
    PHP_FE(boxwood_exists, NULL)
	{NULL, NULL, NULL}	/* Must be the last line in boxwood_functions[] */
};
/* }}} */

/* {{{ boxwood_module_entry
 */
zend_module_entry boxwood_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"boxwood",
	boxwood_functions,
	PHP_MINIT(boxwood),
	PHP_MSHUTDOWN(boxwood),
    NULL,
	NULL,
	PHP_MINFO(boxwood),
#if ZEND_MODULE_API_NO >= 20010901
	"1.0",
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_BOXWOOD
ZEND_GET_MODULE(boxwood)
#endif

static void php_bw_trie_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    struct bw_trie_t *trie = (struct bw_trie_t *) rsrc->ptr;
    bw_free_trie(trie);
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(boxwood)
{
    le_bw_trie = zend_register_list_destructors_ex(php_bw_trie_dtor, NULL, PHP_BOXWOOD_TRIE_RES_NAME, module_number);
    BOXWOOD_G(folding_trie) = calloc(1, sizeof(struct case_fold_branch_t));
    case_fold_map_load((struct case_fold_branch_t *) BOXWOOD_G(folding_trie));
    bw_initialize_default_word_boundary_bytes();

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(boxwood)
{
    bw_free_default_word_boundary_bytes();
    case_fold_map_free((struct case_fold_branch_t *)  BOXWOOD_G(folding_trie));
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(boxwood)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "boxwood support", "enabled");
	php_info_print_table_end();

}
/* }}} */


PHP_FUNCTION(boxwood_new)
{
    zend_bool case_sensitive = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &case_sensitive) == FAILURE) {
        RETURN_FALSE;
    }

    struct bw_trie_t *trie = bw_create_trie((struct case_fold_branch_t *) (case_sensitive ? NULL : BOXWOOD_G(folding_trie)));
    ZEND_REGISTER_RESOURCE(return_value, trie, le_bw_trie);
}

PHP_FUNCTION(boxwood_add_text)
{
    struct bw_trie_t *trie;
    zval *znode;
    char *text;
    int text_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &znode, &text, &text_len) == FAILURE) {
        RETURN_FALSE;
    }
    
    ZEND_FETCH_RESOURCE(trie, struct bw_trie_t*, &znode, -1, PHP_BOXWOOD_TRIE_RES_NAME, le_bw_trie);

    int added = bw_add_text(trie, (byte *) text);
    RETURN_LONG(added);

 }

PHP_FUNCTION(boxwood_replace_text)
{
    struct bw_trie_t *trie;
    zval *znode;
    zval *ztext;
    zval *zreplacement;
    zval *zwordbound;

    byte *replacement;
    int replacement_len;
    char *result;
    int wordbound = 0;

    int num_args = ZEND_NUM_ARGS();

    if (num_args == 3) {
        if (zend_parse_parameters(num_args TSRMLS_CC, "zzz", &znode, &ztext, &zreplacement) == FAILURE) {
            WRONG_PARAM_COUNT;
        }
    } else if (num_args == 4) {
        if (zend_parse_parameters(num_args TSRMLS_CC, "zzzz", &znode, &ztext, &zreplacement, &zwordbound) == FAILURE) {
            WRONG_PARAM_COUNT;
        }
    } else {
        WRONG_PARAM_COUNT;
    }

    if (Z_TYPE_P(znode) != IS_RESOURCE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Parameter mismatch: first argument must be a Boxwood resource");
        RETURN_FALSE;
    }
    if ((Z_TYPE_P(ztext) != IS_STRING) && (Z_TYPE_P(ztext) != IS_ARRAY)) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Parameter mismatch: second argument must be a string or array");
        RETURN_FALSE;
    }
    if (Z_TYPE_P(zreplacement) != IS_STRING) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Parameter mismatch: third argument must be a string");
        RETURN_FALSE;
    }
    if (num_args == 4) {
        if (Z_TYPE_P(zwordbound) != IS_BOOL) {
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "Parameter mismatch: fourth argument must be a boolean");
            RETURN_FALSE;
        }
        wordbound = Z_BVAL_P(zwordbound) ? 1 : 0;
    }

    /*SEPARATE_ZVAL(&znode);
    SEPARATE_ZVAL(&ztext);
    SEPARATE_ZVAL(&zreplacement); */

    replacement = (byte *) Z_STRVAL_P(zreplacement);
    replacement_len = Z_STRLEN_P(zreplacement);

    if (replacement_len > 1) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s is longer than one byte (%d)", replacement, replacement_len);
    }

    if (replacement[0] > 0x80) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "%s is not a valid 1-byte UTF-8 character", replacement);
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(trie, struct bw_trie_t*, &znode, -1, PHP_BOXWOOD_TRIE_RES_NAME, le_bw_trie);

    if (Z_TYPE_P(ztext) == IS_STRING) {
        result = (char *) bw_replace_text(trie, (byte *) Z_STRVAL_P(ztext), replacement[0], wordbound);
        RETVAL_STRINGL(result, strlen(result), 1);
        free(result);
    }
    else {
        array_init(return_value);
        zval **one_text;
        HashTable *arr = Z_ARRVAL_P(ztext);
        HashPosition ptr;
		for (zend_hash_internal_pointer_reset_ex(arr, &ptr);
             zend_hash_get_current_data_ex(arr, (void **) &one_text, &ptr) == SUCCESS;
             zend_hash_move_forward_ex(arr, &ptr)) {
            result = (char *) bw_replace_text(trie, (byte *) Z_STRVAL_PP(one_text), replacement[0], wordbound);
            add_next_index_string(return_value, result, 1);
            free(result);
        }
    }
}

PHP_FUNCTION(boxwood_set_word_boundary_bytes)
{
    struct bw_trie_t *trie;
    zval *znode;
    char *text;
    int text_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &znode, &text, &text_len) == FAILURE) {
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(trie, struct bw_trie_t*, &znode, -1, PHP_BOXWOOD_TRIE_RES_NAME, le_bw_trie);

    bw_set_word_boundary_bytes(trie, text);
}

PHP_FUNCTION(boxwood_exists)
{
    struct bw_trie_t *trie;
    zval *znode;
    zval *ztext;
    zval *zwordbound;
    int result = 0;

    int wordbound = 0;

    int num_args = ZEND_NUM_ARGS();

    if (num_args == 2) {
        if (zend_parse_parameters(num_args TSRMLS_CC, "zz", &znode, &ztext) == FAILURE) {
            WRONG_PARAM_COUNT;
        }
    } else if (num_args == 3) {
        if (zend_parse_parameters(num_args TSRMLS_CC, "zzz", &znode, &ztext,&zwordbound) == FAILURE) {
            WRONG_PARAM_COUNT;
        }
    } else {
        WRONG_PARAM_COUNT;
    }

    if (Z_TYPE_P(znode) != IS_RESOURCE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Parameter mismatch: first argument must be a Boxwood resource");
        RETURN_FALSE;
    }
    if (Z_TYPE_P(ztext) != IS_STRING) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Parameter mismatch: second argument must be a string");
        RETURN_FALSE;
    }

    if (num_args == 3) {
        if (Z_TYPE_P(zwordbound) != IS_BOOL) {
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "Parameter mismatch: fourth argument must be a boolean");
            RETURN_FALSE;
        }
        wordbound = Z_BVAL_P(zwordbound) ? 1 : 0;
    }

    ZEND_FETCH_RESOURCE(trie, struct bw_trie_t*, &znode, -1, PHP_BOXWOOD_TRIE_RES_NAME, le_bw_trie);

    if (Z_TYPE_P(ztext) == IS_STRING) {
        result = bw_exists_text(trie, (byte *) Z_STRVAL_P(ztext), wordbound);
        if(result == 1) {
            RETURN_TRUE;
        }
        else {
            RETURN_FALSE;
        }
    }

    RETURN_FALSE;
}
