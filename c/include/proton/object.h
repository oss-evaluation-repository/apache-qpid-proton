#ifndef PROTON_OBJECT_H
#define PROTON_OBJECT_H 1

/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */

#include <proton/cid.h>
#include <proton/types.h>
#include <stdarg.h>
#include <proton/type_compat.h>
#include <stddef.h>
#include <proton/import_export.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @cond INTERNAL
 */

typedef const void* pn_handle_t;
typedef intptr_t pn_shandle_t;

typedef struct pn_class_t pn_class_t;
typedef struct pn_string_t pn_string_t;
typedef struct pn_list_t pn_list_t;
typedef struct pn_map_t pn_map_t;
typedef struct pn_hash_t pn_hash_t;
typedef void *(*pn_iterator_next_t)(void *state);
typedef struct pn_iterator_t pn_iterator_t;
typedef struct pn_record_t pn_record_t;

struct pn_class_t {
  const char *name;
  const pn_cid_t cid;
  void *(*newinst)(const pn_class_t *, size_t);
  void (*initialize)(void *);
  void (*incref)(void *);
  void (*decref)(void *);
  int (*refcount)(void *);
  void (*finalize)(void *);
  void (*free)(void *);
  uintptr_t (*hashcode)(void *);
  intptr_t (*compare)(void *, void *);
  int (*inspect)(void *, pn_string_t *);
};

/* Hack alert: Declare these as arrays so we can treat the name of the single
   object as the address */
PN_EXTERN extern const pn_class_t PN_DEFAULT[];
PN_EXTERN extern const pn_class_t PN_OBJECT[];
PN_EXTERN extern const pn_class_t PN_VOID[];
PN_EXTERN extern const pn_class_t PN_WEAKREF[];

PN_EXTERN void pn_object_incref(void *object);

#define PN_CLASSCLASS(PREFIX) PREFIX ## __class

#define PN_CLASS(PREFIX) {                      \
    #PREFIX,                                    \
    CID_ ## PREFIX,                             \
    NULL,                                       \
    PREFIX ## _initialize,                      \
    NULL,                                       \
    NULL,                                       \
    NULL,                                       \
    PREFIX ## _finalize,                        \
    NULL,                                       \
    PREFIX ## _hashcode,                        \
    PREFIX ## _compare,                         \
    PREFIX ## _inspect                          \
}

#define PN_METACLASS(PREFIX) {                  \
    #PREFIX,                                    \
    CID_ ## PREFIX,                             \
    PREFIX ## _new,                             \
    PREFIX ## _initialize,                      \
    PREFIX ## _incref,                          \
    PREFIX ## _decref,                          \
    PREFIX ## _refcount,                        \
    PREFIX ## _finalize,                        \
    PREFIX ## _free,                            \
    PREFIX ## _hashcode,                        \
    PREFIX ## _compare,                         \
    PREFIX ## _inspect                          \
}

PN_EXTERN void *pn_void_new(const pn_class_t *clazz, size_t size);
PN_EXTERN void pn_void_incref(void *object);
PN_EXTERN void pn_void_decref(void *object);
PN_EXTERN int pn_void_refcount(void *object);

/* Class to identify a plain C struct in a pn_event_t. No refcounting or memory management. */
#define PN_STRUCT_CLASSDEF(PREFIX)                  \
const pn_class_t PN_CLASSCLASS(PREFIX)[] = {{       \
  #PREFIX,                                          \
  CID_ ## PREFIX,                                   \
  NULL, /*_new*/                                    \
  NULL, /*_initialize*/                             \
  pn_void_incref,                                   \
  pn_void_decref,                                   \
  pn_void_refcount,                                 \
  NULL, /* _finalize */                             \
  NULL, /* _free */                                 \
  NULL, /* _hashcode */                             \
  NULL, /* _compare */                              \
  NULL, /* _inspect */                              \
}};                                                 \

PN_EXTERN pn_cid_t pn_class_id(const pn_class_t *clazz);
PN_EXTERN const char *pn_class_name(const pn_class_t *clazz);
PN_EXTERN void *pn_class_new(const pn_class_t *clazz, size_t size);

PN_EXTERN void *pn_class_incref(const pn_class_t *clazz, void *object);
PN_EXTERN int pn_class_refcount(const pn_class_t *clazz, void *object);
PN_EXTERN int pn_class_decref(const pn_class_t *clazz, void *object);

PN_EXTERN void pn_class_free(const pn_class_t *clazz, void *object);

PN_EXTERN intptr_t pn_class_compare(const pn_class_t *clazz, void *a, void *b);
PN_EXTERN bool pn_class_equals(const pn_class_t *clazz, void *a, void *b);
PN_EXTERN int pn_class_inspect(const pn_class_t *clazz, void *object, pn_string_t *dst);

/* pn_incref, pn_decref and pn_refcount are for internal use by the proton
 *   library, the should not be called by application code. Application code
 *   should use the appropriate pn_*_free function (pn_link_free, pn_session_free
 *   etc.) when it is finished with a proton value. Proton values should only be
 *   used when handling a pn_event_t that refers to them.
 */
PN_EXTERN void *pn_incref(void *object);
PN_EXTERN int pn_decref(void *object);
PN_EXTERN int pn_refcount(void *object);
PN_EXTERN void pn_free(void *object);
PN_EXTERN const pn_class_t *pn_class(void* object);
PN_EXTERN uintptr_t pn_hashcode(void *object);
PN_EXTERN intptr_t pn_compare(void *a, void *b);
PN_EXTERN bool pn_equals(void *a, void *b);
PN_EXTERN int pn_inspect(void *object, pn_string_t *dst);

PN_EXTERN pn_list_t *pn_list(const pn_class_t *clazz, size_t capacity);
PN_EXTERN size_t pn_list_size(pn_list_t *list);
PN_EXTERN void *pn_list_get(pn_list_t *list, int index);
PN_EXTERN void pn_list_set(pn_list_t *list, int index, void *value);
PN_EXTERN int pn_list_add(pn_list_t *list, void *value);
PN_EXTERN void *pn_list_pop(pn_list_t *list);
PN_EXTERN ssize_t pn_list_index(pn_list_t *list, void *value);
PN_EXTERN bool pn_list_remove(pn_list_t *list, void *value);
PN_EXTERN void pn_list_del(pn_list_t *list, int index, int n);
PN_EXTERN void pn_list_clear(pn_list_t *list);
PN_EXTERN void pn_list_iterator(pn_list_t *list, pn_iterator_t *iter);
PN_EXTERN void pn_list_minpush(pn_list_t *list, void *value);
PN_EXTERN void *pn_list_minpop(pn_list_t *list);

PN_EXTERN pn_map_t *pn_map(const pn_class_t *key, const pn_class_t *value,
                           size_t capacity, float load_factor);
PN_EXTERN size_t pn_map_size(pn_map_t *map);
PN_EXTERN int pn_map_put(pn_map_t *map, void *key, void *value);
PN_EXTERN void *pn_map_get(pn_map_t *map, void *key);
PN_EXTERN void pn_map_del(pn_map_t *map, void *key);
PN_EXTERN pn_handle_t pn_map_head(pn_map_t *map);
PN_EXTERN pn_handle_t pn_map_next(pn_map_t *map, pn_handle_t entry);
PN_EXTERN void *pn_map_key(pn_map_t *map, pn_handle_t entry);
PN_EXTERN void *pn_map_value(pn_map_t *map, pn_handle_t entry);

PN_EXTERN pn_hash_t *pn_hash(const pn_class_t *clazz, size_t capacity, float load_factor);
PN_EXTERN size_t pn_hash_size(pn_hash_t *hash);
PN_EXTERN int pn_hash_put(pn_hash_t *hash, uintptr_t key, void *value);
PN_EXTERN void *pn_hash_get(pn_hash_t *hash, uintptr_t key);
PN_EXTERN void pn_hash_del(pn_hash_t *hash, uintptr_t key);
PN_EXTERN pn_handle_t pn_hash_head(pn_hash_t *hash);
PN_EXTERN pn_handle_t pn_hash_next(pn_hash_t *hash, pn_handle_t entry);
PN_EXTERN uintptr_t pn_hash_key(pn_hash_t *hash, pn_handle_t entry);
PN_EXTERN void *pn_hash_value(pn_hash_t *hash, pn_handle_t entry);

PN_EXTERN pn_string_t *pn_string(const char *bytes);
PN_EXTERN pn_string_t *pn_stringn(const char *bytes, size_t n);
PN_EXTERN const char *pn_string_get(pn_string_t *string);
PN_EXTERN size_t pn_string_size(pn_string_t *string);
PN_EXTERN int pn_string_set(pn_string_t *string, const char *bytes);
PN_EXTERN int pn_string_setn(pn_string_t *string, const char *bytes, size_t n);
PN_EXTERN ssize_t pn_string_put(pn_string_t *string, char *dst);
PN_EXTERN void pn_string_clear(pn_string_t *string);
PN_EXTERN int pn_string_format(pn_string_t *string, const char *format, ...)
#ifdef __GNUC__
  __attribute__ ((format (printf, 2, 3)))
#endif
    ;
PN_EXTERN int pn_string_vformat(pn_string_t *string, const char *format, va_list ap);
PN_EXTERN int pn_string_addf(pn_string_t *string, const char *format, ...)
#ifdef __GNUC__
  __attribute__ ((format (printf, 2, 3)))
#endif
    ;
PN_EXTERN int pn_string_vaddf(pn_string_t *string, const char *format, va_list ap);
PN_EXTERN int pn_string_grow(pn_string_t *string, size_t capacity);
PN_EXTERN char *pn_string_buffer(pn_string_t *string);
PN_EXTERN size_t pn_string_capacity(pn_string_t *string);
PN_EXTERN int pn_string_resize(pn_string_t *string, size_t size);
PN_EXTERN int pn_string_copy(pn_string_t *string, pn_string_t *src);

PN_EXTERN pn_iterator_t *pn_iterator(void);
PN_EXTERN void *pn_iterator_start(pn_iterator_t *iterator,
                                  pn_iterator_next_t next, size_t size);
PN_EXTERN void *pn_iterator_next(pn_iterator_t *iterator);

#define PN_LEGCTX ((pn_handle_t) 0)

/**
   PN_HANDLE is a trick to define a unique identifier by using the address of a static variable.
   You MUST NOT use it in a .h file, since it must be defined uniquely in one compilation unit.
   Your .h file can provide access to the handle (if needed) via a function. For example:

       /// my_thing.h
       pn_handle_t get_my_thing(void);

       /// my_thing.c
       PN_HANDLE(MY_THING);
       pn_handle_t get_my_thing(void) { return MY_THING; }

   Note that the name "MY_THING" is not exported and is not required to be
   unique except in the .c file. The linker will guarantee that the *address* of
   MY_THING, as returned by get_my_thing() *is* unique across the entire linked
   executable.
 */
#define PN_HANDLE(name) \
  static const char _PN_HANDLE_ ## name = 0; \
  static const pn_handle_t name = ((pn_handle_t) &_PN_HANDLE_ ## name);

PN_EXTERN pn_record_t *pn_record(void);
PN_EXTERN void pn_record_def(pn_record_t *record, pn_handle_t key, const pn_class_t *clazz);
PN_EXTERN bool pn_record_has(pn_record_t *record, pn_handle_t key);
PN_EXTERN void *pn_record_get(pn_record_t *record, pn_handle_t key);
PN_EXTERN void pn_record_set(pn_record_t *record, pn_handle_t key, void *value);
PN_EXTERN void pn_record_clear(pn_record_t *record);

/**
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* object.h */
