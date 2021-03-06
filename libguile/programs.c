/* Copyright (C) 2001, 2009, 2010, 2011, 2012, 2013, 2014, 2017 Free Software Foundation, Inc.
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include "_scm.h"
#include "instructions.h"
#include "modules.h"
#include "programs.h"
#include "procprop.h" /* scm_sym_name */
#include "vm.h"


static SCM write_program = SCM_BOOL_F;

SCM_DEFINE (scm_program_code, "program-code", 1, 0, 0,
            (SCM program),
            "")
#define FUNC_NAME s_scm_program_code
{
  SCM_VALIDATE_PROGRAM (1, program);

  return scm_from_uintptr_t ((scm_t_uintptr) SCM_PROGRAM_CODE (program));
}
#undef FUNC_NAME

SCM
scm_i_program_name (SCM program)
{
  static SCM program_name = SCM_BOOL_F;

  if (SCM_PRIMITIVE_P (program))
    return SCM_SUBR_NAME (program);

  if (scm_is_false (program_name) && scm_module_system_booted_p)
    program_name =
        scm_c_private_variable ("system vm program", "program-name");

  return scm_call_1 (scm_variable_ref (program_name), program);
}

SCM
scm_i_program_documentation (SCM program)
{
  static SCM program_documentation = SCM_BOOL_F;

  if (SCM_PRIMITIVE_P (program))
    return SCM_BOOL_F;

  if (scm_is_false (program_documentation) && scm_module_system_booted_p)
    program_documentation =
      scm_c_private_variable ("system vm program", "program-documentation");

  return scm_call_1 (scm_variable_ref (program_documentation), program);
}

SCM
scm_i_program_properties (SCM program)
{
  static SCM program_properties = SCM_BOOL_F;

  if (SCM_PRIMITIVE_P (program))
    {
      SCM name = scm_i_program_name (program);
      if (scm_is_false (name))
        return SCM_EOL;
      return scm_acons (scm_sym_name, name, SCM_EOL);
    }

  if (scm_is_false (program_properties) && scm_module_system_booted_p)
    program_properties =
      scm_c_private_variable ("system vm program", "program-properties");

  return scm_call_1 (scm_variable_ref (program_properties), program);
}

void
scm_i_program_print (SCM program, SCM port, scm_print_state *pstate)
{
  static int print_error = 0;

  if (scm_is_false (write_program) && scm_module_system_booted_p)
    write_program = scm_c_private_variable ("system vm program",
                                            "write-program");
  
  if (SCM_PROGRAM_IS_CONTINUATION (program))
    {
      /* twingliness */
      scm_puts ("#<continuation ", port);
      scm_uintprint (SCM_UNPACK (program), 16, port);
      scm_putc ('>', port);
    }
  else if (SCM_PROGRAM_IS_PARTIAL_CONTINUATION (program))
    {
      /* twingliness */
      scm_puts ("#<partial-continuation ", port);
      scm_uintprint (SCM_UNPACK (program), 16, port);
      scm_putc ('>', port);
    }
  else if (scm_is_false (write_program) || print_error)
    {
      scm_puts ("#<program ", port);
      scm_uintprint (SCM_UNPACK (program), 16, port);
      scm_putc (' ', port);
      scm_uintprint ((scm_t_uintptr) SCM_PROGRAM_CODE (program), 16, port);
      scm_putc ('>', port);
    }
  else
    {
      print_error = 1;
      scm_call_2 (SCM_VARIABLE_REF (write_program), program, port);
      print_error = 0;
    }
}


/*
 * Scheme interface
 */

SCM_DEFINE (scm_program_p, "program?", 1, 0, 0,
	    (SCM obj),
	    "")
#define FUNC_NAME s_scm_program_p
{
  return scm_from_bool (SCM_PROGRAM_P (obj));
}
#undef FUNC_NAME

SCM_DEFINE (scm_primitive_code_p, "primitive-code?", 1, 0, 0,
	    (SCM code),
	    "")
#define FUNC_NAME s_scm_primitive_code_p
{
  const scm_t_uint32 * ptr = (const scm_t_uint32 *) scm_to_uintptr_t (code);

  return scm_from_bool (scm_i_primitive_code_p (ptr));
}
#undef FUNC_NAME

SCM_DEFINE (scm_primitive_call_ip, "primitive-call-ip", 1, 0, 0,
	    (SCM prim),
	    "")
#define FUNC_NAME s_scm_primitive_call_ip
{
  SCM_MAKE_VALIDATE (1, prim, PRIMITIVE_P);

  return scm_from_uintptr_t (scm_i_primitive_call_ip (prim));
}
#undef FUNC_NAME

SCM
scm_find_source_for_addr (SCM ip)
{
  static SCM source_for_addr = SCM_BOOL_F;

  if (scm_is_false (source_for_addr)) {
    if (!scm_module_system_booted_p)
      return SCM_BOOL_F;

    source_for_addr =
      scm_c_private_variable ("system vm program", "source-for-addr");
  }

  return scm_call_1 (scm_variable_ref (source_for_addr), ip);
}

SCM
scm_program_address_range (SCM program)
{
  static SCM program_address_range = SCM_BOOL_F;

  if (scm_is_false (program_address_range) && scm_module_system_booted_p)
    program_address_range =
      scm_c_private_variable ("system vm program", "program-address-range");

  return scm_call_1 (scm_variable_ref (program_address_range), program);
}

SCM_DEFINE (scm_program_num_free_variables, "program-num-free-variables", 1, 0, 0,
	    (SCM program),
	    "")
#define FUNC_NAME s_scm_program_num_free_variables
{
  SCM_VALIDATE_PROGRAM (1, program);

  return scm_from_ulong (SCM_PROGRAM_NUM_FREE_VARIABLES (program));
}
#undef FUNC_NAME

SCM_DEFINE (scm_program_free_variable_ref, "program-free-variable-ref", 2, 0, 0,
	    (SCM program, SCM i),
	    "")
#define FUNC_NAME s_scm_program_free_variable_ref
{
  unsigned long idx;

  SCM_VALIDATE_PROGRAM (1, program);
  SCM_VALIDATE_ULONG_COPY (2, i, idx);
  if (idx >= SCM_PROGRAM_NUM_FREE_VARIABLES (program))
    SCM_OUT_OF_RANGE (2, i);
  return SCM_PROGRAM_FREE_VARIABLE_REF (program, idx);
}
#undef FUNC_NAME

SCM_DEFINE (scm_program_free_variable_set_x, "program-free-variable-set!", 3, 0, 0,
	    (SCM program, SCM i, SCM x),
	    "")
#define FUNC_NAME s_scm_program_free_variable_set_x
{
  unsigned long idx;

  SCM_VALIDATE_PROGRAM (1, program);
  SCM_VALIDATE_ULONG_COPY (2, i, idx);
  if (idx >= SCM_PROGRAM_NUM_FREE_VARIABLES (program))
    SCM_OUT_OF_RANGE (2, i);
  SCM_PROGRAM_FREE_VARIABLE_SET (program, idx, x);
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

/* It's hacky, but it manages to cover all of the non-keyword cases.  */
static int
try_parse_arity (SCM program, int *req, int *opt, int *rest)
{
  scm_t_uint32 *code = SCM_PROGRAM_CODE (program);
  scm_t_uint32 slots, min;

  switch (code[0] & 0xff) {
  case scm_op_assert_nargs_ee:
    slots = code[0] >> 8;
    *req = slots - 1;
    *opt = 0;
    *rest = 0;
    return 1;
  case scm_op_assert_nargs_ee_locals:
    slots = (code[0] >> 8) & 0xfff;
    *req = slots - 1;
    *opt = 0;
    *rest = 0;
    return 1;
  case scm_op_assert_nargs_le:
    slots = code[0] >> 8;
    *req = 0;
    *opt = slots - 1;
    *rest = 0;
    return 1;
  case scm_op_bind_rest:
    slots = code[0] >> 8;
    *req = 0;
    *opt = slots - 1;
    *rest = 1;
    return 1;
  case scm_op_assert_nargs_ge:
    min = code[0] >> 8;
    switch (code[1] & 0xff) {
    case scm_op_assert_nargs_le:
      slots = code[1] >> 8;
      *req = min - 1;
      *opt = slots - 1 - *req;
      *rest = 0;
      return 1;
    case scm_op_bind_rest:
      slots = code[1] >> 8;
      *req = min - 1;
      *opt = slots - min;
      *rest = 1;
      return 1;
    default:
      return 0;
    }
  case scm_op_continuation_call:
  case scm_op_compose_continuation:
    *req = 0;
    *opt = 0;
    *rest = 1;
    return 1;
  default:
    return 0;
  }
}

int
scm_i_program_arity (SCM program, int *req, int *opt, int *rest)
{
  static SCM program_minimum_arity = SCM_BOOL_F;
  SCM l;

  if (try_parse_arity (program, req, opt, rest))
    return 1;

  if (scm_is_false (program_minimum_arity) && scm_module_system_booted_p)
    program_minimum_arity =
        scm_c_private_variable ("system vm program", "program-minimum-arity");

  l = scm_call_1 (scm_variable_ref (program_minimum_arity), program);
  if (scm_is_false (l))
    return 0;

  *req = scm_to_int (scm_car (l));
  *opt = scm_to_int (scm_cadr (l));
  *rest = scm_is_true (scm_caddr (l));

  return 1;
}



void
scm_bootstrap_programs (void)
{
  scm_c_register_extension ("libguile-" SCM_EFFECTIVE_VERSION,
                            "scm_init_programs",
                            (scm_t_extension_init_func)scm_init_programs, NULL);
}

void
scm_init_programs (void)
{
#ifndef SCM_MAGIC_SNARFER
#include "libguile/programs.x"
#endif
}

/*
  Local Variables:
  c-file-style: "gnu"
  End:
*/
