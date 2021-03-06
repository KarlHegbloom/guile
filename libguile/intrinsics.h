/* Copyright (C) 2018 Free Software Foundation, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef _SCM_VM_INTRINSICS_H_
#define _SCM_VM_INTRINSICS_H_

#include <libguile.h>

#ifdef BUILDING_LIBGUILE

#include <libguile/vm.h>

typedef SCM (*scm_t_scm_from_scm_scm_intrinsic) (SCM, SCM);
typedef SCM (*scm_t_scm_from_scm_uimm_intrinsic) (SCM, scm_t_uint8);
typedef void (*scm_t_scm_u64_u64_intrinsic) (SCM, scm_t_uint64, scm_t_uint64);
typedef SCM (*scm_t_scm_from_scm_intrinsic) (SCM);
typedef double (*scm_t_f64_from_scm_intrinsic) (SCM);
typedef scm_t_uint64 (*scm_t_u64_from_scm_intrinsic) (SCM);
typedef scm_t_int64 (*scm_t_s64_from_scm_intrinsic) (SCM);
typedef SCM (*scm_t_scm_from_u64_intrinsic) (scm_t_uint64);
typedef SCM (*scm_t_scm_from_s64_intrinsic) (scm_t_int64);
typedef void (*scm_t_thread_intrinsic) (scm_i_thread*);
typedef void (*scm_t_thread_scm_intrinsic) (scm_i_thread*, SCM);
typedef void (*scm_t_thread_scm_scm_intrinsic) (scm_i_thread*, SCM, SCM);
typedef SCM (*scm_t_scm_from_thread_scm_intrinsic) (scm_i_thread*, SCM);
typedef SCM (*scm_t_scm_from_scm_u64_intrinsic) (SCM, scm_t_uint64);
typedef int (*scm_t_bool_from_scm_scm_intrinsic) (SCM, SCM);
typedef enum scm_compare (*scm_t_compare_from_scm_scm_intrinsic) (SCM, SCM);

#define SCM_FOR_ALL_VM_INTRINSICS(M) \
  M(scm_from_scm_scm, add, "add", ADD) \
  M(scm_from_scm_uimm, add_immediate, "add/immediate", ADD_IMMEDIATE) \
  M(scm_from_scm_scm, sub, "sub", SUB) \
  M(scm_from_scm_uimm, sub_immediate, "sub/immediate", SUB_IMMEDIATE) \
  M(scm_from_scm_scm, mul, "mul", MUL) \
  M(scm_from_scm_scm, div, "div", DIV) \
  M(scm_from_scm_scm, quo, "quo", QUO) \
  M(scm_from_scm_scm, rem, "rem", REM) \
  M(scm_from_scm_scm, mod, "mod", MOD) \
  M(scm_from_scm_scm, logand, "logand", LOGAND) \
  M(scm_from_scm_scm, logior, "logior", LOGIOR) \
  M(scm_from_scm_scm, logxor, "logxor", LOGXOR) \
  M(scm_u64_u64, string_set_x, "string-set!", STRING_SET_X) \
  M(scm_from_scm, string_to_number, "string->number", STRING_TO_NUMBER) \
  M(scm_from_scm, string_to_symbol, "string->symbol", STRING_TO_SYMBOL) \
  M(scm_from_scm, symbol_to_keyword, "symbol->keyword", SYMBOL_TO_KEYWORD) \
  M(scm_from_scm, class_of, "class-of", CLASS_OF) \
  M(f64_from_scm, scm_to_f64, "scm->f64", SCM_TO_F64) \
  M(u64_from_scm, scm_to_u64, "scm->u64", SCM_TO_U64) \
  M(u64_from_scm, scm_to_u64_truncate, "scm->u64/truncate", SCM_TO_U64_TRUNCATE) \
  M(s64_from_scm, scm_to_s64, "scm->s64", SCM_TO_S64) \
  M(scm_from_u64, u64_to_scm, "u64->scm", U64_TO_SCM) \
  M(scm_from_s64, s64_to_scm, "s64->scm", S64_TO_SCM) \
  M(scm_from_scm_scm, logsub, "logsub", LOGSUB) \
  M(thread_scm_scm, wind, "wind", WIND) \
  M(thread, unwind, "unwind", UNWIND) \
  M(thread_scm_scm, push_fluid, "push-fluid", PUSH_FLUID) \
  M(thread, pop_fluid, "pop-fluid", POP_FLUID) \
  M(scm_from_thread_scm, fluid_ref, "fluid-ref", FLUID_REF) \
  M(thread_scm_scm, fluid_set_x, "fluid-set!", FLUID_SET_X) \
  M(thread_scm, push_dynamic_state, "push-dynamic-state", PUSH_DYNAMIC_STATE) \
  M(thread, pop_dynamic_state, "pop-dynamic-state", POP_DYNAMIC_STATE) \
  M(scm_from_scm_u64, lsh, "lsh", LSH) \
  M(scm_from_scm_u64, rsh, "rsh", RSH) \
  M(scm_from_scm_uimm, lsh_immediate, "lsh/immediate", LSH_IMMEDIATE) \
  M(scm_from_scm_uimm, rsh_immediate, "rsh/immediate", RSH_IMMEDIATE) \
  M(bool_from_scm_scm, heap_numbers_equal_p, "heap-numbers-equal?", HEAP_NUMBERS_EQUAL_P) \
  M(compare_from_scm_scm, less_p, "<?", LESS_P) \
  M(bool_from_scm_scm, numerically_equal_p, "=?", NUMERICALLY_EQUAL_P) \
  M(scm_from_scm_uimm, resolve_module, "resolve-module", RESOLVE_MODULE) \
  M(scm_from_scm_scm, lookup, "lookup", LOOKUP) \
  M(scm_from_scm_scm, define_x, "define!", DEFINE_X) \
  /* Add new intrinsics here; also update scm_bootstrap_intrinsics.  */

enum scm_vm_intrinsic
  {
#define DEFINE_ENUM(type, id, name, ID) SCM_VM_INTRINSIC_##ID,
    SCM_FOR_ALL_VM_INTRINSICS(DEFINE_ENUM)
#undef DEFINE_ENUM
    SCM_VM_INTRINSIC_COUNT
  };

SCM_INTERNAL struct scm_vm_intrinsics
{
#define DEFINE_MEMBER(type, id, name, ID) scm_t_##type##_intrinsic id;
    SCM_FOR_ALL_VM_INTRINSICS(DEFINE_MEMBER)
#undef DEFINE_MEMBER
} scm_vm_intrinsics;

#endif /* BUILDING_LIBGUILE  */

SCM_INTERNAL SCM scm_intrinsic_list (void);

SCM_INTERNAL void scm_bootstrap_intrinsics (void);
SCM_INTERNAL void scm_init_intrinsics (void);

#endif /* _SCM_INSTRUCTIONS_H_ */

/*
  Local Variables:
  c-file-style: "gnu"
  End:
*/
