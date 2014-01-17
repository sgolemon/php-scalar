/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2006 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.0 of the PHP license,       |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_0.txt.                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Sara Golemon <pollita@php.net>                               |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "php_scalar.h"

#if ZEND_MODULE_API_NO > 20131217
#define ZEND_ENGINE_2_7
#endif
#if ZEND_MODULE_API_NO > 20131105
#define ZEND_ENGINE_2_6
#endif
#if ZEND_MODULE_API_NO > 20121211
#define ZEND_ENGINE_2_5
#endif
#if ZEND_MODULE_API_NO > 20100524
#define ZEND_ENGINE_2_4
#endif
#if ZEND_MODULE_API_NO > 20090625
#define ZEND_ENGINE_2_3
#endif
#if ZEND_MODULE_API_NO > 20050922
#define ZEND_ENGINE_2_2
#endif
#if ZEND_MODULE_API_NO > 20050921
#define ZEND_ENGINE_2_1
#endif

#define SCALAR_UNKNOWN  0
#define SCALAR_BOOL     1
#define SCALAR_INT      2
#define SCALAR_FLOAT    3
#define SCALAR_NUMBER   4
#define SCALAR_STRING   5
#define SCALAR_SCALAR   6
#define SCALAR_OBJECT   7
#define SCALAR_RESOURCE 8

inline zend_uchar scalar_get_type(zend_arg_info *info) {
	if (!info) return SCALAR_UNKNOWN;
#define X(ptype, stype) \
	if (((sizeof(#ptype) - 1) == info->class_name_len) && \
	    (!strcasecmp(#ptype, info->class_name))) { \
		return stype; \
	}

	X(bool, SCALAR_BOOL)
	X(int, SCALAR_INT)
	X(float, SCALAR_FLOAT)
	X(num, SCALAR_NUMBER)
	X(string, SCALAR_STRING)
	X(scalar, SCALAR_SCALAR)
	X(object, SCALAR_OBJECT)
	X(resource, SCALAR_RESOURCE)
#undef X
	return SCALAR_UNKNOWN;
}

#if defined(ZEND_ENGINE_2_6)
#define PHP_SCALAR_OPCODE_COUNT 165
#elif defined(ZEND_ENGINE_2_5)
#define PHP_SCALAR_OPCODE_COUNT 163
#elif defined(ZEND_ENGINE_2_4)
#define PHP_SCALAR_OPCODE_COUNT 158
#elif defined(ZEND_ENGINE_2_3)
#define PHP_SCALAR_OPCODE_COUNT 153
#elif defined(ZEND_ENGINE_2_1)
#define PHP_SCALAR_OPCODE_COUNT 150
#else
# error "Upgrade PHP, damnit"
#endif

#define PHP_SCALAR_OPHANDLER_COUNT                            ((25 * (PHP_SCALAR_OPCODE_COUNT + 1)) + 1)
#define PHP_SCALAR_REPLACE_OPCODE(opname)                     { int i; for(i = 5; i < 25; i++) if (php_scalar_opcode_handlers[(opname*25) + i]) php_scalar_opcode_handlers[(opname*25) + i] = php_scalar_op_##opname; }

static opcode_handler_t *php_scalar_original_opcode_handlers;
static opcode_handler_t php_scalar_opcode_handlers[PHP_SCALAR_OPHANDLER_COUNT];

static zval **scalar_vm_stack_get_arg(int requested_arg TSRMLS_DC) {
	zend_execute_data *ex = EG(current_execute_data)->prev_execute_data;
	void **p = ex->function_state.arguments;
	int arg_count = (int)(zend_uintptr_t) *p;

	if (requested_arg > arg_count) {
		return NULL;
	}
	return (zval**)p - arg_count + requested_arg - 1;
}

static zend_bool scalar_validate_type(zval *param, zend_uchar exp) {
	switch (exp) {
		case SCALAR_BOOL:
			return (Z_TYPE_P(param) == IS_BOOL);
		case SCALAR_INT:
			return (Z_TYPE_P(param) == IS_LONG);
		case SCALAR_FLOAT:
			return (Z_TYPE_P(param) == IS_DOUBLE);
		case SCALAR_NUMBER:
			return (Z_TYPE_P(param) == IS_LONG) ||
			       (Z_TYPE_P(param) == IS_DOUBLE);
		case SCALAR_STRING:
			return (Z_TYPE_P(param) == IS_STRING);
		case SCALAR_SCALAR:
			return (Z_TYPE_P(param) == IS_BOOL) ||
			       (Z_TYPE_P(param) == IS_LONG) ||
			       (Z_TYPE_P(param) == IS_DOUBLE) ||
			       (Z_TYPE_P(param) == IS_STRING);
		case SCALAR_OBJECT:
			return (Z_TYPE_P(param) == IS_OBJECT);
		case SCALAR_RESOURCE:
			return (Z_TYPE_P(param) == IS_RESOURCE);
	}
	return 0;
}

static int scalar_decode(zend_op *opline) {
	int ret = opline->opcode * 25;
	switch (opline->op1_type) {
		case IS_CONST:                          break;
		case IS_TMP_VAR:        ret += 5;       break;
		case IS_VAR:            ret += 10;      break;
		case IS_UNUSED:         ret += 15;      break;
		case IS_CV:             ret += 20;      break;
	}
	switch (opline->op2_type) {
		case IS_CONST:                          break;
		case IS_TMP_VAR:        ret += 1;       break;
		case IS_VAR:            ret += 2;       break;
		case IS_UNUSED:         ret += 3;       break;
		case IS_CV:             ret += 4;       break;
	}
	return ret;
}

static int ZEND_FASTCALL php_scalar_op_ZEND_RECV(ZEND_OPCODE_HANDLER_ARGS) {
	zend_op *opline = execute_data->opline;
	zend_uint arg_num = opline->op1.num;
	zend_uchar exp = scalar_get_type( (arg_num <= EG(active_op_array)->num_args)
	                    ? &EG(active_op_array)->arg_info[arg_num-1] : NULL );
	zval **param = scalar_vm_stack_get_arg(arg_num TSRMLS_CC);

	if (!param || !scalar_validate_type(*param, exp) || (exp == SCALAR_UNKNOWN)) {
		return php_scalar_original_opcode_handlers[scalar_decode(opline)](ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
	} else {
		/* Hide the type hint, but otherwise pass-through - JANKY */
		zend_arg_info *info = &EG(active_op_array)->arg_info[arg_num-1];
		const char *class_name = info->class_name;
		int class_name_len = info->class_name_len;
		int type_hint = info->type_hint, ret;
		info->class_name = NULL;
		info->class_name_len = 0;
		info->type_hint = 0;
		ret = php_scalar_original_opcode_handlers[scalar_decode(opline)](ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
		info->class_name = class_name;
		info->class_name_len = class_name_len;
		info->type_hint = type_hint;
		return ret;
	}
}

static int ZEND_FASTCALL php_scalar_op_ZEND_RECV_INIT(ZEND_OPCODE_HANDLER_ARGS) {
	/* For passthroughs to the original, opline will decode itself correctly */
	return php_scalar_op_ZEND_RECV(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
}

#ifdef ZEND_ENGINE_2_6
static int ZEND_FASTCALL php_scalar_op_ZEND_RECV_VARIADIC(ZEND_OPCODE_HANDLER_ARGS) {
	zend_op *opline = execute_data->opline;
	zend_uint arg_num = opline->op1.num;
	zend_uint arg_count = scalar_vm_stack_get_args_count(TSRMLS_C);
	zend_uchar exp = scalar_get_type( (arg_num <= EG(active_op_array)->num_args)
	                    ? &EG(active_op_array)->arg_info[arg_num-1] : NULL );
	zend_bool valid = 1;

	if (exp != SCALAR_UNKNOWN) {
		for(;arg_num < arg_count; ++arg_num) {
			zval **param = scalar_vm_stack_get_arg(arg_num TSRMLS_CC);
			if (!(valid = param && scalar_validate_type(*param, exp))) {
				break;
			}
		}
	}
	if (!valid || (exp == SCALAR_UNKNOWN)) {
		return php_scalar_original_opcode_handlers[scalar_decode(opline)](ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
	} else {
		/* Hide the type hint, but otherwise pass-through - JANKY */
		zend_arg_info *info = &EG(active_op_array)->arg_info[opline->op1.num-1];
		const char *class_name = info->class_name;
		int class_name_len = info->class_name_len, ret;
		info->class_name = NULL;
		info->class_name_len = 0;
		ret = php_scalar_original_opcode_handlers[scalar_decode(opline)](ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
		class_name = info->class_name;
		class_name_len = info->class_name_len;
		return ret;
	}
}
#endif /* ZEND_ENGINE_2_6 */

/* {{{ MINFO */
static PHP_MINIT_FUNCTION(scalar) {
	memcpy(php_scalar_opcode_handlers, zend_opcode_handlers, sizeof(php_scalar_opcode_handlers));
	php_scalar_original_opcode_handlers = zend_opcode_handlers;
	zend_opcode_handlers = php_scalar_opcode_handlers;

	PHP_SCALAR_REPLACE_OPCODE(ZEND_RECV);
	PHP_SCALAR_REPLACE_OPCODE(ZEND_RECV_INIT);
#ifdef ZEND_ENGINE_2_6
	PHP_SCALAR_REPLACE_OPCODE(ZEND_RECV_VARIADIC);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ scalar_module_entry
 */
zend_module_entry scalar_module_entry = {
	STANDARD_MODULE_HEADER,
	"scalar",
	NULL, /* functions */
	PHP_MINIT(scalar),
	NULL, /* MSHUTDOWN */
	NULL, /* RINIT */
	NULL, /* RSHUTDOWN */
	NULL, /* MINFO */
	NO_VERSION_YET,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SCALAR
ZEND_GET_MODULE(scalar)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
