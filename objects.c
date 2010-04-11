/* GNU Objective C Runtime class related functions
   Copyright (C) 1993, 1995, 1996 Free Software Foundation, Inc.
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

#include "tconfig.h"         /* include defs of bzero for target */
#include "objc/objc.h"
#include "objc/runtime.h"		/* the kitchen sink */
#include "objc/message.h"
#include "objc-private.h"
#include "objc-runtime-new.h"

#if OBJC_WITH_GC
# include <gc.h>
#endif

#define newcls(cls) ((struct class_t *)cls)
#define newcat(cat) ((struct category_t *)cat)
#define newmethod(meth) ((struct method_t *)meth)
#define newivar(ivar) ((struct ivar_t *)ivar)
#define newcategory(cat) ((struct category_t *)cat)
#define newprotocol(p) ((struct protocol_t *)p)

#ifdef __LP64__
#define WORD_SHIFT 3UL
#define WORD_MASK 7UL
#else
#define WORD_SHIFT 2UL
#define WORD_MASK 3UL
#endif

static const char *getName(struct class_t *cls);
static uint32_t instanceSize(struct class_t *cls);
static BOOL isMetaClass(struct class_t *cls);
static struct class_t *getSuperclass(struct class_t *cls);
static void unload_class(class_t *cls, BOOL isMeta);
static class_t *setSuperclass(class_t *cls, class_t *newSuper);
static class_t *realizeClass(class_t *cls);
static void flushCaches(class_t *cls);
static void flushVtables(class_t *cls);
static method_t *getMethodNoSuper_nolock(struct class_t *cls, SEL sel);
static method_t *getMethod_nolock(class_t *cls, SEL sel);
static void changeInfo(class_t *cls, unsigned int set, unsigned int clear);
static IMP _method_getImplementation(method_t *m);

/***********************************************************************
 * isRealized
 * Returns YES if class cls has been realized.
 * Locking: To prevent concurrent realization, hold runtimeLock.
 **********************************************************************/
static BOOL isRealized(class_t *cls)
{
    return (cls->data->flags & RW_REALIZED) ? YES : NO;
}


/***********************************************************************
 * _class_getInstanceSize
 * fixme
 * Locking: none
 **********************************************************************/
size_t 
_class_getInstanceSize(Class cls)
{
    if (!cls) return 0;
    return instanceSize(newcls(cls));
}

static uint32_t
instanceSize(struct class_t *cls)
{
    assert(cls);
    assert(isRealized(cls));
    // fixme rdar://5244378
    return (uint32_t)((cls->data->ro->instanceSize + WORD_MASK) & ~WORD_MASK);
}


id _internal_class_createInstanceFromZone(Class cls, size_t extraBytes, void *zone);

/***********************************************************************
 * class_createInstanceFromZone
 * fixme
 * Locking: none
 **********************************************************************/
id
class_createInstanceFromZone(Class cls, size_t extraBytes, void *zone)
{
    if (cls) assert(isRealized(newcls(cls)));
    return _internal_class_createInstanceFromZone(cls, extraBytes, zone);
}


/***********************************************************************
 * class_createInstance
 * fixme
 * Locking: none
 **********************************************************************/
id 
class_createInstance(Class cls, size_t extraBytes)
{
    return class_createInstanceFromZone(cls, extraBytes, NULL);
}

/***********************************************************************
 * object_copyFromZone
 * fixme
 * Locking: none
 **********************************************************************/
id 
object_copyFromZone(id oldObj, size_t extraBytes, void *zone)
{
    id obj;
    size_t size;
	
    if (!oldObj) return nil;
	
    size = _class_getInstanceSize(oldObj->isa) + extraBytes;
	
	obj = (id) calloc(1, size);
	
    if (!obj) return nil;
	
    // fixme this doesn't handle C++ ivars correctly (#4619414)
    objc_memmove_collectable(obj, oldObj, size);
	
    return obj;
}

/***********************************************************************
 * object_copy
 * fixme
 * Locking: none
 **********************************************************************/
id 
object_copy(id oldObj, size_t extraBytes)
{
    return object_copyFromZone(oldObj, extraBytes, malloc_default_zone());
}

id _internal_object_dispose(id anObject);

/***********************************************************************
 * object_dispose
 * fixme
 * Locking: none
 **********************************************************************/
id 
object_dispose(id obj)
{
    return _internal_object_dispose(obj);
}