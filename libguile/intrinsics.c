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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "_scm.h"
#include "intrinsics.h"


struct scm_vm_intrinsics scm_vm_intrinsics;

SCM_DEFINE (scm_intrinsic_list, "intrinsic-list", 0, 0, 0,
	    (void),
	    "")
#define FUNC_NAME s_scm_intrinsic_list
{
  SCM list = SCM_EOL;

#define ADD_INTRINSIC(type, id, name, ID)                   \
  if (name)                                                 \
    list = scm_acons (scm_from_latin1_symbol (name),        \
                      scm_from_int (SCM_VM_INTRINSIC_##ID), \
                      list);
  SCM_FOR_ALL_VM_INTRINSICS (ADD_INTRINSIC);
#undef ADD_INTRINSIC

  return list;
}
#undef FUNC_NAME

static SCM
add_immediate (SCM a, scm_t_uint8 b)
{
  return scm_sum (a, scm_from_uint8 (b));
}

static SCM
sub_immediate (SCM a, scm_t_uint8 b)
{
  return scm_difference (a, scm_from_uint8 (b));
}

static void
string_set_x (SCM str, scm_t_uint64 idx, scm_t_uint64 ch)
{
  str = scm_i_string_start_writing (str);
  scm_i_string_set_x (str, idx, ch);
  scm_i_string_stop_writing ();
}

static SCM
string_to_number (SCM str)
{
  return scm_string_to_number (str, SCM_UNDEFINED /* radix = 10 */);
}

static scm_t_uint64
scm_to_uint64_truncate (SCM x)
{
  if (SCM_I_INUMP (x))
    return (scm_t_uint64) SCM_I_INUM (x);
  else
    return scm_to_uint64 (scm_logand (x, scm_from_uint64 ((scm_t_uint64) -1)));
}

static SCM
logsub (SCM x, SCM y)
{
  if (SCM_I_INUMP (x) && SCM_I_INUMP (y))
    {
      scm_t_signed_bits a, b;

      a = SCM_I_INUM (x);
      b = SCM_I_INUM (y);

      return SCM_I_MAKINUM (a & ~b);
    }

  return scm_logand (x, scm_lognot (y));
}

static void
wind (scm_i_thread *thread, SCM winder, SCM unwinder)
{
  scm_dynstack_push_dynwind (&thread->dynstack, winder, unwinder);
}

static void
unwind (scm_i_thread *thread)
{
  scm_dynstack_pop (&thread->dynstack);
}

static void
push_fluid (scm_i_thread *thread, SCM fluid, SCM value)
{
  scm_dynstack_push_fluid (&thread->dynstack, fluid, value,
                           thread->dynamic_state);
}

static void
pop_fluid (scm_i_thread *thread)
{
  scm_dynstack_unwind_fluid (&thread->dynstack, thread->dynamic_state);
}

static SCM
fluid_ref (scm_i_thread *thread, SCM fluid)
{
  struct scm_cache_entry *entry;

  /* If we find FLUID in the cache, then it is indeed a fluid.  */
  entry = scm_cache_lookup (&thread->dynamic_state->cache, fluid);
  if (SCM_LIKELY (scm_is_eq (SCM_PACK (entry->key), fluid)
                  && !SCM_UNBNDP (SCM_PACK (entry->value))))
    return SCM_PACK (entry->value);

  return scm_fluid_ref (fluid);
}

static void
fluid_set_x (scm_i_thread *thread, SCM fluid, SCM value)
{
  struct scm_cache_entry *entry;

  /* If we find FLUID in the cache, then it is indeed a fluid.  */
  entry = scm_cache_lookup (&thread->dynamic_state->cache, fluid);
  if (SCM_LIKELY (scm_is_eq (SCM_PACK (entry->key), fluid)))
    entry->value = SCM_UNPACK (value);
  else
    scm_fluid_set_x (fluid, value);
}

static void
push_dynamic_state (scm_i_thread *thread, SCM state)
{
  scm_dynstack_push_dynamic_state (&thread->dynstack, state,
                                   thread->dynamic_state);
}

static void
pop_dynamic_state (scm_i_thread *thread)
{
  scm_dynstack_unwind_dynamic_state (&thread->dynstack,
                                     thread->dynamic_state);
}

static SCM
lsh (SCM a, scm_t_uint64 b)
{
  if (SCM_LIKELY (SCM_I_INUMP (a))
      && b < (scm_t_uint64) (SCM_I_FIXNUM_BIT - 1)
      && ((scm_t_bits)
          (SCM_SRS (SCM_I_INUM (a), (SCM_I_FIXNUM_BIT-1 - b)) + 1)
          <= 1))
    {
      scm_t_signed_bits nn = SCM_I_INUM (a);
      return SCM_I_MAKINUM (nn < 0 ? -(-nn << b) : (nn << b));
    }
  else
    return scm_ash (a, scm_from_uint64 (b));
}

static SCM
rsh (SCM a, scm_t_uint64 b)
{
  if (SCM_LIKELY (SCM_I_INUMP (a)))
    {
      if (b > (scm_t_uint64) (SCM_I_FIXNUM_BIT - 1))
        b = SCM_I_FIXNUM_BIT - 1;
      return SCM_I_MAKINUM (SCM_SRS (SCM_I_INUM (a), b));
    }
  else
    return scm_ash (a, scm_difference (SCM_INUM0, scm_from_uint64 (b)));
}

static SCM
lsh_immediate (SCM a, scm_t_uint8 b)
{
  return lsh (a, b);
}

static SCM
rsh_immediate (SCM a, scm_t_uint8 b)
{
  return rsh (a, b);
}

static enum scm_compare
less_p (SCM a, SCM b)
{
  if (scm_is_true (scm_nan_p (a)) || scm_is_true (scm_nan_p (b)))
    return SCM_F_COMPARE_INVALID;
  else if (scm_is_true (scm_less_p (a, b)))
    return SCM_F_COMPARE_LESS_THAN;
  else
    return SCM_F_COMPARE_NONE;
}

static int
numerically_equal_p (SCM a, SCM b)
{
  return scm_is_true (scm_num_eq_p (a, b));
}

static SCM
resolve_module (SCM name, scm_t_uint8 public_p)
{
  SCM mod;

  if (!scm_module_system_booted_p)
    return SCM_BOOL_F;

  mod = scm_maybe_resolve_module (name);
  if (scm_is_false (mod))
    scm_misc_error (NULL, "Module named ~s does not exist",
                    scm_list_1 (name));

  if (public_p)
    {
      mod = scm_module_public_interface (mod);

      if (scm_is_false (mod))
        scm_misc_error (NULL, "Module named ~s has no public interface",
                        scm_list_1 (name));
    }

  return mod;
}

static SCM
lookup (SCM module, SCM name)
{
  /* If MODULE was captured before modules were booted, use the root
     module.  Not so nice, but hey...  */
  if (scm_is_false (module))
    module = scm_the_root_module ();

  return scm_module_variable (module, name);
}

void
scm_bootstrap_intrinsics (void)
{
  scm_vm_intrinsics.add = scm_sum;
  scm_vm_intrinsics.add_immediate = add_immediate;
  scm_vm_intrinsics.sub = scm_difference;
  scm_vm_intrinsics.sub_immediate = sub_immediate;
  scm_vm_intrinsics.mul = scm_product;
  scm_vm_intrinsics.div = scm_divide;
  scm_vm_intrinsics.quo = scm_quotient;
  scm_vm_intrinsics.rem = scm_remainder;
  scm_vm_intrinsics.mod = scm_modulo;
  scm_vm_intrinsics.logand = scm_logand;
  scm_vm_intrinsics.logior = scm_logior;
  scm_vm_intrinsics.logxor = scm_logxor;
  scm_vm_intrinsics.string_set_x = string_set_x;
  scm_vm_intrinsics.string_to_number = string_to_number;
  scm_vm_intrinsics.string_to_symbol = scm_string_to_symbol;
  scm_vm_intrinsics.symbol_to_keyword = scm_symbol_to_keyword;
  scm_vm_intrinsics.class_of = scm_class_of;
  scm_vm_intrinsics.scm_to_f64 = scm_to_double;
  scm_vm_intrinsics.scm_to_u64 = scm_to_uint64;
  scm_vm_intrinsics.scm_to_u64_truncate = scm_to_uint64_truncate;
  scm_vm_intrinsics.scm_to_s64 = scm_to_int64;
  scm_vm_intrinsics.u64_to_scm = scm_from_uint64;
  scm_vm_intrinsics.s64_to_scm = scm_from_int64;
  scm_vm_intrinsics.logsub = logsub;
  scm_vm_intrinsics.wind = wind;
  scm_vm_intrinsics.unwind = unwind;
  scm_vm_intrinsics.push_fluid = push_fluid;
  scm_vm_intrinsics.pop_fluid = pop_fluid;
  scm_vm_intrinsics.fluid_ref = fluid_ref;
  scm_vm_intrinsics.fluid_set_x = fluid_set_x;
  scm_vm_intrinsics.push_dynamic_state = push_dynamic_state;
  scm_vm_intrinsics.pop_dynamic_state = pop_dynamic_state;
  scm_vm_intrinsics.lsh = lsh;
  scm_vm_intrinsics.rsh = rsh;
  scm_vm_intrinsics.lsh_immediate = lsh_immediate;
  scm_vm_intrinsics.rsh_immediate = rsh_immediate;
  scm_vm_intrinsics.heap_numbers_equal_p = scm_i_heap_numbers_equal_p;
  scm_vm_intrinsics.less_p = less_p;
  scm_vm_intrinsics.numerically_equal_p = numerically_equal_p;
  scm_vm_intrinsics.resolve_module = resolve_module;
  scm_vm_intrinsics.lookup = lookup;
  scm_vm_intrinsics.define_x = scm_module_ensure_local_variable;

  scm_c_register_extension ("libguile-" SCM_EFFECTIVE_VERSION,
                            "scm_init_intrinsics",
                            (scm_t_extension_init_func)scm_init_intrinsics,
                            NULL);
}

void
scm_init_intrinsics (void)
{
#ifndef SCM_MAGIC_SNARFER
#include "libguile/intrinsics.x"
#endif
}

/*
  Local Variables:
  c-file-style: "gnu"
  End:
*/
