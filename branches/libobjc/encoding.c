/*
 * Copyright (c) 1999-2007 Apple Inc.  All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/***********************************************************************
 * objc-typeencoding.m
 * Parsing of old-style type strings.
 **********************************************************************/

#include "objc-private.h"
#include "objc/runtime.h"

#define _C_CONST	'r'
#define _C_IN		'n'
#define _C_INOUT	'N'
#define _C_OUT      	'o'
#define _C_BYCOPY	'O'
#define _C_BYREF	'R'
#define _C_ONEWAY	'V'
#define _C_GCINVISIBLE	'!'
#define _C_COMPLEX   'j'

#define _F_CONST	0x01
#define _F_IN		0x01
#define _F_OUT		0x02
#define _F_INOUT	0x03
#define _F_BYCOPY	0x04
#define _F_BYREF	0x08
#define _F_ONEWAY	0x10
#define _F_GCINVISIBLE	0x20


inline const char *objc_skip_offset (const char *type);
const char *objc_skip_argspec (const char *type);
inline const char *objc_skip_type_qualifiers (const char *type);
const char *objc_skip_typespec (const char *type);

/*
 Skip an offset as part of a method encoding.  This is prepended by a
 '+' if the argument is passed in registers.
 */
inline const char *
objc_skip_offset (const char *type)
{
	if (*type == '+')
		type++;
	while (isdigit ((unsigned char) *++type))
		;
	return type;
}

/*
 Skip an argument specification of a method encoding.
 */
const char *
objc_skip_argspec (const char *type)
{
	type = objc_skip_typespec (type);
	type = objc_skip_offset (type);
	return type;
}

inline const char *
objc_skip_type_qualifiers (const char *type)
{
	while (*type == _C_CONST
		   || *type == _C_IN
		   || *type == _C_INOUT
		   || *type == _C_OUT
		   || *type == _C_BYCOPY
		   || *type == _C_BYREF
		   || *type == _C_ONEWAY
		   || *type == _C_GCINVISIBLE)
    {
		type += 1;
    }
	return type;
}


const char *
objc_skip_typespec (const char *type)
{
	/* Skip the variable name if any */
	if (*type == '"')
    {
		for (type++; *type++ != '"';)
		/* do nothing */;
    }
	
	type = objc_skip_type_qualifiers (type);
	
	switch (*type) {
			
		case _C_ID:
			/* An id may be annotated by the actual type if it is known
			 with the @"ClassName" syntax */
			
			if (*++type != '"')
				return type;
			else
			{
				while (*++type != '"')
				/* do nothing */;
				return type + 1;
			}
			
			/* The following are one character type codes */
		case _C_CLASS:
		case _C_SEL:
		case _C_CHR:
		case _C_UCHR:
		case _C_CHARPTR:
		case _C_ATOM:
		case _C_SHT:
		case _C_USHT:
		case _C_INT:
		case _C_UINT:
		case _C_LNG:
		case _C_BOOL:
		case _C_ULNG:
		case _C_LNG_LNG:
		case _C_ULNG_LNG:
		case _C_FLT:
		case _C_DBL:
		case _C_VOID:
		case _C_UNDEF:
			return ++type;
			break;
			
		case _C_COMPLEX:
			return type + 2;
			break;
			
		case _C_ARY_B:
			/* skip digits, typespec and closing ']' */
			
			while (isdigit ((unsigned char)*++type))
				;
			type = objc_skip_typespec (type);
			if (*type == _C_ARY_E)
				return ++type;
			else
			{
				objc_error (nil, OBJC_ERR_BAD_TYPE, "bad array type %s\n", type);
				return 0;
			}
			
		case _C_BFLD:
			/* The new encoding of bitfields is: b 'position' 'type' 'size' */
			while (isdigit ((unsigned char)*++type))
				;	/* skip position */
			while (isdigit ((unsigned char)*++type))
				;	/* skip type and size */
			return type;
			
		case _C_STRUCT_B:
			/* skip name, and elements until closing '}'  */
			
			while (*type != _C_STRUCT_E && *type++ != '=')
				;
			while (*type != _C_STRUCT_E)
			{
				type = objc_skip_typespec (type);
			}
			return ++type;
			
		case _C_UNION_B:
			/* skip name, and elements until closing ')'  */
			
			while (*type != _C_UNION_E && *type++ != '=')
				;
			while (*type != _C_UNION_E)
			{
				type = objc_skip_typespec (type);
			}
			return ++type;
			
		case _C_PTR:
			/* Just skip the following typespec */
			
			return objc_skip_typespec (++type);
			
		default:
		{
			objc_error (nil, OBJC_ERR_BAD_TYPE, "unknown type %s\n", type);
			return 0;
		}
	}
}


char *
method_get_next_argument (arglist_t argframe, const char **type)
{
	const char *t = objc_skip_argspec (*type);
	
	if (*t == '\0')
		return 0;
	
	*type = t;
	t = objc_skip_typespec (t);
	
	if (*t == '+')
		return argframe->arg_regs + atoi (++t);
	else
		return argframe->arg_ptr + atoi (t);
}

char *
method_get_first_argument (struct objc_method *m,
						   arglist_t argframe,
						   const char **type)
{
	*type = m->method_types;
	return method_get_next_argument (argframe, type);
}

/***********************************************************************
 * SubtypeUntil.
 *
 * Delegation.
 **********************************************************************/
static int	SubtypeUntil	       (const char *	type,
									char		end)
{
    int		level = 0;
    const char *	head = type;
	
    //
    while (*type)
    {
        if (!*type || (!level && (*type == end)))
            return (int)(type - head);
		
        switch (*type)
        {
            case ']': case '}': case ')': level--; break;
            case '[': case '{': case '(': level += 1; break;
        }
		
        type += 1;
    }
	
    _objc_fatal ("Object: SubtypeUntil: end of type encountered prematurely\n");
    return 0;
}


/***********************************************************************
 * SkipFirstType.
 **********************************************************************/
static const char *	SkipFirstType	   (const char *	type)
{
    while (1)
    {
        switch (*type++)
        {
            case 'O':	/* bycopy */
            case 'n':	/* in */
            case 'o':	/* out */
            case 'N':	/* inout */
            case 'r':	/* const */
            case 'V':	/* oneway */
            case '^':	/* pointers */
                break;
				
                /* arrays */
            case '[':
                while ((*type >= '0') && (*type <= '9'))
                    type += 1;
                return type + SubtypeUntil (type, ']') + 1;
				
                /* structures */
            case '{':
                return type + SubtypeUntil (type, '}') + 1;
				
                /* unions */
            case '(':
                return type + SubtypeUntil (type, ')') + 1;
				
                /* basic types */
            default:
                return type;
        }
    }
}


/***********************************************************************
 * encoding_getNumberOfArguments.
 **********************************************************************/
unsigned int 
encoding_getNumberOfArguments(const char *typedesc)
{
    unsigned nargs;
	
    // First, skip the return type
    typedesc = SkipFirstType (typedesc);
	
    // Next, skip stack size
    while ((*typedesc >= '0') && (*typedesc <= '9'))
        typedesc += 1;
	
    // Now, we have the arguments - count how many
    nargs = 0;
    while (*typedesc)
    {
        // Traverse argument type
        typedesc = SkipFirstType (typedesc);
		
        // Skip GNU runtime's register parameter hint
        if (*typedesc == '+') typedesc++;
		
        // Traverse (possibly negative) argument offset
        if (*typedesc == '-')
            typedesc += 1;
        while ((*typedesc >= '0') && (*typedesc <= '9'))
            typedesc += 1;
		
        // Made it past an argument
        nargs += 1;
    }
	
    return nargs;
}

/***********************************************************************
 * encoding_getSizeOfArguments.
 **********************************************************************/
unsigned 
encoding_getSizeOfArguments(const char *typedesc)
{
    unsigned		stack_size;
#if defined(__ppc__) || defined(ppc)
    unsigned		trueBaseOffset;
    unsigned		foundBaseOffset;
#endif
	
    // Get our starting points
    stack_size = 0;
	
    // Skip the return type
#if defined (__ppc__) || defined(ppc)
    // Struct returns cause the parameters to be bumped
    // by a register, so the offset to the receiver is
    // 4 instead of the normal 0.
    trueBaseOffset = (*typedesc == '{') ? (unsigned)sizeof(void *) : 0;
#endif
    typedesc = SkipFirstType (typedesc);
	
    // Convert ASCII number string to integer
    while ((*typedesc >= '0') && (*typedesc <= '9'))
        stack_size = (stack_size * 10) + (*typedesc++ - '0');
#if defined (__ppc__) || defined(ppc)
    // NOTE: This is a temporary measure pending a compiler fix.
    // Work around PowerPC compiler bug wherein the method argument
    // string contains an incorrect value for the "stack size."
    // Generally, the size is reported 4 bytes too small, so we apply
    // that fudge factor.  Unfortunately, there is at least one case
    // where the error is something other than -4: when the last
    // parameter is a double, the reported stack is much too high
    // (about 32 bytes).  We do not attempt to detect that case.
    // The result of returning a too-high value is that objc_msgSendv
    // can bus error if the destination of the marg_list copying
    // butts up against excluded memory.
    // This fix disables itself when it sees a correctly built
    // type string (i.e. the offset for the Id is correct).  This
    // keeps us out of lockstep with the compiler.
	
    // skip the '@' marking the Id field
    typedesc = SkipFirstType (typedesc);
	
    // Skip GNU runtime's register parameter hint
    if (*typedesc == '+') typedesc++;
	
    // pick up the offset for the Id field
    foundBaseOffset = 0;
    while ((*typedesc >= '0') && (*typedesc <= '9'))
        foundBaseOffset = (foundBaseOffset * 10) + (*typedesc++ - '0');
	
    // add fudge factor iff the Id field offset was wrong
    if (foundBaseOffset != trueBaseOffset)
        stack_size += 4;
#endif
	
    return stack_size;
}


/***********************************************************************
 * encoding_getArgumentInfo.
 **********************************************************************/
unsigned int 
encoding_getArgumentInfo(const char *typedesc, int arg,
                         const char **type, int *offset)
{
    unsigned nargs = 0;
    int self_offset = 0;
    BOOL offset_is_negative = NO;
	
    // First, skip the return type
    typedesc = SkipFirstType (typedesc);
	
    // Next, skip stack size
    while ((*typedesc >= '0') && (*typedesc <= '9'))
        typedesc += 1;
	
    // Now, we have the arguments - position typedesc to the appropriate argument
    while (*typedesc && nargs != arg)
    {
		
        // Skip argument type
        typedesc = SkipFirstType (typedesc);
		
        if (nargs == 0)
        {
            // Skip GNU runtime's register parameter hint
            if (*typedesc == '+') typedesc++;
			
            // Skip negative sign in offset
            if (*typedesc == '-')
            {
                offset_is_negative = YES;
                typedesc += 1;
            }
            else
                offset_is_negative = NO;
			
            while ((*typedesc >= '0') && (*typedesc <= '9'))
                self_offset = self_offset * 10 + (*typedesc++ - '0');
            if (offset_is_negative)
                self_offset = -(self_offset);
			
        }
		
        else
        {
            // Skip GNU runtime's register parameter hint
            if (*typedesc == '+') typedesc++;
			
            // Skip (possibly negative) argument offset
            if (*typedesc == '-')
                typedesc += 1;
            while ((*typedesc >= '0') && (*typedesc <= '9'))
                typedesc += 1;
        }
		
        nargs += 1;
    }
	
    if (*typedesc)
    {
        int arg_offset = 0;
		
        *type	 = typedesc;
        typedesc = SkipFirstType (typedesc);
		
        if (arg == 0)
        {
            *offset = 0;
        }
		
        else
        {
            // Skip GNU register parameter hint
            if (*typedesc == '+') typedesc++;
			
            // Pick up (possibly negative) argument offset
            if (*typedesc == '-')
            {
                offset_is_negative = YES;
                typedesc += 1;
            }
            else
                offset_is_negative = NO;
			
            while ((*typedesc >= '0') && (*typedesc <= '9'))
                arg_offset = arg_offset * 10 + (*typedesc++ - '0');
            if (offset_is_negative)
                arg_offset = - arg_offset;
			
            *offset = arg_offset - self_offset;
        }
		
    }
	
    else
    {
        *type	= 0;
        *offset	= 0;
    }
	
    return nargs;
}


void 
encoding_getReturnType(const char *t, char *dst, size_t dst_len)
{
    size_t len;
    const char *end;
	
    if (!dst) return;
    if (!t) {
        strncpy(dst, "", dst_len);
        return;
    }
	
    end = SkipFirstType(t);
    len = end - t;
    strncpy(dst, t, MIN(len, dst_len));
    if (len < dst_len) memset(dst+len, 0, dst_len - len);
}

/***********************************************************************
 * encoding_copyReturnType.  Returns the method's return type string 
 * on the heap. 
 **********************************************************************/
char *
encoding_copyReturnType(const char *t)
{
    size_t len;
    const char *end;
    char *result;
	
    if (!t) return NULL;
	
    end = SkipFirstType(t);
    len = end - t;
    result = malloc(len + 1);
    strncpy(result, t, len);
    result[len] = '\0';
    return result;
}


void 
encoding_getArgumentType(const char *t, unsigned int index, 
                         char *dst, size_t dst_len)
{
    size_t len;
    const char *end;
    int offset;
	
    if (!dst) return;
    if (!t) {
        strncpy(dst, "", dst_len);
        return;
    }
	
    encoding_getArgumentInfo(t, index, &t, &offset);
	
    if (!t) {
        strncpy(dst, "", dst_len);
        return;
    }
	
    end = SkipFirstType(t);
    len = end - t;
    strncpy(dst, t, MIN(len, dst_len));
    if (len < dst_len) memset(dst+len, 0, dst_len - len);
}


/***********************************************************************
 * encoding_copyArgumentType.  Returns a single argument's type string 
 * on the heap. Argument 0 is `self`; argument 1 is `_cmd`. 
 **********************************************************************/
char *
encoding_copyArgumentType(const char *t, unsigned int index)
{
    size_t len;
    const char *end;
    char *result;
    int offset;
	
    if (!t) return NULL;
	
    encoding_getArgumentInfo(t, index, &t, &offset);
	
    if (!t) return NULL;
	
    end = SkipFirstType(t);
    len = end - t;
    result = malloc(len + 1);
    strncpy(result, t, len);
    result[len] = '\0';
    return result;
}
