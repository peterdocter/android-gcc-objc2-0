/* GNU Objective C Runtime message lookup 
   Copyright (C) 1993, 1995, 1996, 1997, 1998,
   2001, 2002, 2004 Free Software Foundation, Inc.
   Contributed by Kresten Krab Thorup

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2, or (at your option) any later version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License along with
GCC; see the file COPYING.  If not, write to the Free Software
Foundation, 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.  */

/* As a special exception, if you link this library with files compiled with
   GCC to produce an executable, this does not cause the resulting executable
   to be covered by the GNU General Public License. This exception does not
   however invalidate any other reasons why the executable file might be
   covered by the GNU General Public License.  */

/* FIXME: This file has no business including tm.h.  */
/* FIXME: This should be using libffi instead of __builtin_apply
   and friends.  */

#include "tconfig.h"
#include "coretypes.h"
#include "tm.h"
#include "objc-private.h"
#include "objc/runtime.h"
#include "objc/sarray.h"
#include "objc/encoding.h"
#include "runtime-info.h"
#include "objc-runtime-new.h"

/* This is how we hack STRUCT_VALUE to be 1 or 0.   */
#define gen_rtx(args...) 1
#define gen_rtx_MEM(args...) 1
#define gen_rtx_REG(args...) 1

#if ! defined (STRUCT_VALUE) || STRUCT_VALUE == 0
#define INVISIBLE_STRUCT_RETURN 1
#else
#define INVISIBLE_STRUCT_RETURN 0
#endif

/* The uninstalled dispatch table */
struct sarray *__objc_uninstalled_dtable = 0;   /* !T:MUTEX */

/* Hook for method forwarding. If it is set, is invoked to return a
   function that performs the real forwarding. Otherwise the libgcc
   based functions (__builtin_apply and friends) are used. */
IMP (*__objc_msg_forward) (SEL) = NULL;

/* Send +initialize to class */
static void __objc_send_initialize (Class);

static void __objc_install_dispatch_table_for_class (Class);

/* Forward declare some functions */
static void __objc_init_install_dtable (id, SEL);

/* Various forwarding functions that are used based upon the
   return type for the selector.
   __objc_block_forward for structures.
   __objc_double_forward for floats/doubles.
   __objc_word_forward for pointers or types that fit in registers.
   */
static double __objc_double_forward (id, SEL, ...);
static id __objc_word_forward (id, SEL, ...);
typedef struct { id many[8]; } __big;
#if INVISIBLE_STRUCT_RETURN 
static __big 
#else
static id
#endif
__objc_block_forward (id, SEL, ...);
static method_t search_for_method_in_hierarchy (Class class, SEL sel);
method_t search_for_method_in_list (method_list_t list, SEL op);
id nil_method (id, SEL);

/* Given a selector, return the proper forwarding implementation. */
inline
IMP
__objc_get_forward_imp (SEL sel)
{
  /* If a custom forwarding hook was registered, try getting a forwarding
   * function from it.  */
  if (__objc_msg_forward)
    {
      IMP result;
      if ((result = __objc_msg_forward (sel)) != NULL) 
        return result;
    }

  /* In all other cases, use the default forwarding functions built using
   * __builtin_apply and friends.  */
    {
      const char *t = sel->sel_types;

      if (t && (*t == '[' || *t == '(' || *t == '{')
#ifdef OBJC_MAX_STRUCT_BY_VALUE
          && objc_sizeof_type (t) > OBJC_MAX_STRUCT_BY_VALUE
#endif
          )
        return (IMP)__objc_block_forward;
      else if (t && (*t == 'f' || *t == 'd'))
        return (IMP)__objc_double_forward;
      else
        return (IMP)__objc_word_forward;
    }
}
int method_get_sizeof_arguments (Method *);

retval_t
objc_msg_sendv (id object, SEL op, arglist_t arg_frame)
{
  struct objc_method *m = class_get_instance_method (object->isa, op);
  const char *type;
  *((id *) method_get_first_argument (m, arg_frame, &type)) = object;
  *((SEL *) method_get_next_argument (arg_frame, &type)) = op;
  return __builtin_apply ((apply_t) m->method_imp, 
			  arg_frame,
			  method_get_sizeof_arguments (m));
}


/* This function is installed in the dispatch table for all methods which are
   not implemented.  Thus, it is called when a selector is not recognized. */
static retval_t
__objc_forward (id object, SEL sel, arglist_t args)
{
  IMP imp;
  static SEL frwd_sel = 0;                      /* !T:SAFE2 */
  SEL err_sel;

  /* first try if the object understands forward:: */
  if (! frwd_sel)
    frwd_sel = sel_get_any_uid ("forward::");

  if (__objc_responds_to (object, frwd_sel))
    {
      imp = class_getMethodImplementation (object->isa, frwd_sel);
      return (*imp) (object, frwd_sel, sel, args);
    }

  /* If the object recognizes the doesNotRecognize: method then we're going
     to send it. */
  err_sel = sel_get_any_uid ("doesNotRecognize:");
  if (__objc_responds_to (object, err_sel))
    {
      imp = class_getMethodImplementation (object->isa, err_sel);
      return (*imp) (object, err_sel, sel);
    }
  
  /* The object doesn't recognize the method.  Check for responding to
     error:.  If it does then sent it. */
  {
    char msg[256 + strlen ((const char *) sel_get_name (sel))
             + strlen ((const char *) object_getClassName(object))];

    sprintf (msg, "(%s) %s does not recognize %s",
	     (_class_isMetaClass (object->isa)
	      ? "class"
	      : "instance" ),
             object_getClassName(object), sel_get_name (sel));

    err_sel = sel_get_any_uid ("error:");
    if (__objc_responds_to (object, err_sel))
      {
	imp = class_getMethodImplementation (object->isa, err_sel);
	return (*imp) (object, sel_get_any_uid ("error:"), msg);
      }

    /* The object doesn't respond to doesNotRecognize: or error:;  Therefore,
       a default action is taken. */
    objc_error (object, OBJC_ERR_UNIMPLEMENTED, "%s\n", msg);

    return 0;
  }
}