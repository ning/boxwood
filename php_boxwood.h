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
#ifndef PHP_BOXWOOD_H
#define PHP_BOXWOOD_H

extern zend_module_entry boxwood_module_entry;
#define phpext_boxwood_ptr &boxwood_module_entry

#ifdef PHP_WIN32
#define PHP_BOXWOOD_API __declspec(dllexport)
#else
#define PHP_BOXWOOD_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#include "boxwood.h"
#define PHP_BOXWOOD_TRIE_RES_NAME "Boxwood"

PHP_MINIT_FUNCTION(boxwood);
PHP_MSHUTDOWN_FUNCTION(boxwood);
PHP_MINFO_FUNCTION(boxwood);

PHP_FUNCTION(boxwood_new);
PHP_FUNCTION(boxwood_add_text);
PHP_FUNCTION(boxwood_replace_text);
PHP_FUNCTION(boxwood_set_word_boundary_bytes);
PHP_FUNCTION(boxwood_exists);

ZEND_BEGIN_MODULE_GLOBALS(boxwood)
struct case_fold_branch_t *folding_trie;
ZEND_END_MODULE_GLOBALS(boxwood)

#ifdef ZTS
#define BOXWOOD_G(v) TSRMG(boxwood_globals_id, zend_boxwood_globals *, v)
#else
#define BOXWOOD_G(v) (boxwood_globals.v)
#endif

#endif	/* PHP_BOXWOOD_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
