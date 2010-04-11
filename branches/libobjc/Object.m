/* The implementation of class Object for Objective-C.
   Copyright (C) 1993, 1994, 1995, 1997, 2002 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

GCC is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to
the Free Software Foundation, 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.  */

/* As a special exception, if you link this library with files compiled
   with GCC to produce an executable, this does not cause the resulting
   executable to be covered by the GNU General Public License.  This
   exception does not however invalidate any other reasons why the
   executable file might be covered by the GNU General Public License. */

#include <stdarg.h>
#include "objc/Object.h"
#include "objc/Protocol.h"
#include "objc/objc-api.h"

extern int errno;

#define MAX_CLASS_NAME_LEN 256

@implementation Object

+ initialize
{
  return self;
}

- init
{
  return self;
}

+ new
{
  return [[self alloc] init];
}

+ alloc
{
  return class_create_instance(self);
}

- free
{
  return object_dispose(self);
}

- copy
{
  return [[self shallowCopy] deepen];
}

- shallowCopy
{
  return object_copy(self);
}

- deepen
{
  return self;
}

- deepCopy
{
  return [self copy];
}

- (Class)class
{
  return object_get_class(self);
}

- (Class)superClass
{
  return object_get_super_class(self);
}

- (Class)metaClass
{
  return object_get_meta_class(self);
}

- (const char *)name
{
  return object_get_class_name(self);
}

- self
{
  return self;
}

- (unsigned int)hash
{
  return (size_t)self;
}

- (BOOL)isEqual:anObject
{
  return self==anObject;
}

- (int)compare:anotherObject;
{
  if ([self isEqual:anotherObject])
    return 0;
  // Ordering objects by their address is pretty useless, 
  // so subclasses should override this is some useful way.
  else if (self > anotherObject)
    return 1;
  else 
    return -1;
}

- (BOOL)isMetaClass
{
  return NO;
}

- (BOOL)isClass
{
  return object_is_class(self);
}

- (BOOL)isInstance
{
  return object_is_instance(self);
}

- (BOOL)isKindOf:(Class)aClassObject
{
  Class class;

  for (class = self->isa; class!=Nil; class = class_get_super_class(class))
    if (class==aClassObject)
      return YES;
  return NO;
}

- (BOOL)isMemberOf:(Class)aClassObject
{
  return self->isa==aClassObject;
}

- (BOOL)isKindOfClassNamed:(const char *)aClassName
{
  Class class;

  if (aClassName!=NULL)
    for (class = self->isa; class!=Nil; class = class_get_super_class(class))
      if (!strcmp(class_get_class_name(class), aClassName))
        return YES;
  return NO;
}

- (BOOL)isMemberOfClassNamed:(const char *)aClassName
{
  return ((aClassName!=NULL)
          &&!strcmp(class_get_class_name(self->isa), aClassName));
}

+ (BOOL)instancesRespondTo:(SEL)aSel
{
  return class_get_instance_method(self, aSel) != NULL;
}

- (BOOL)respondsTo:(SEL)aSel
{
  return ((object_is_instance(self)
           ?class_get_instance_method(self->isa, aSel)
           :class_get_class_method(self->isa, aSel)) != NULL);
}

+ (IMP)instanceMethodFor:(SEL)aSel
{
  return method_get_imp(class_get_instance_method(self, aSel));
}

// Indicates if the receiving class or instance conforms to the given protocol
// not usually overridden by subclasses
//
// Modified 9/5/94 to always search the class object's protocol list, rather
// than the meta class.

+ (BOOL) conformsTo: (Protocol*)aProtocol
{
	return NO;
}

- (BOOL) conformsTo: (Protocol*)aProtocol
{
  return [[self class] conformsTo:aProtocol];
}

- (IMP)methodFor:(SEL)aSel
{
  return (method_get_imp(object_is_instance(self)
                         ?class_get_instance_method(self->isa, aSel)
                         :class_get_class_method(self->isa, aSel)));
}

+ (struct objc_method_description *)descriptionForInstanceMethod:(SEL)aSel
{
  return ((struct objc_method_description *)
           class_get_instance_method(self, aSel));
}

- (struct objc_method_description *)descriptionForMethod:(SEL)aSel
{
  return ((struct objc_method_description *)
           (object_is_instance(self)
            ?class_get_instance_method(self->isa, aSel)
            :class_get_class_method(self->isa, aSel)));
}

- perform:(SEL)aSel
{
  IMP msg = objc_msg_lookup(self, aSel);
  if (!msg)
    return [self error:"invalid selector passed to %s", sel_get_name(_cmd)];
  return (*msg)(self, aSel);
}

- perform:(SEL)aSel with:anObject
{
  IMP msg = objc_msg_lookup(self, aSel);
  if (!msg)
    return [self error:"invalid selector passed to %s", sel_get_name(_cmd)];
  return (*msg)(self, aSel, anObject);
}

- perform:(SEL)aSel with:anObject1 with:anObject2
{
  IMP msg = objc_msg_lookup(self, aSel);
  if (!msg)
    return [self error:"invalid selector passed to %s", sel_get_name(_cmd)];
  return (*msg)(self, aSel, anObject1, anObject2);
}

+ poseAs:(Class)aClassObject
{
  return class_pose_as(self, aClassObject);
}

- (Class)transmuteClassTo:(Class)aClassObject
{
  if (object_is_instance(self))
    if (class_is_class(aClassObject))
      if (class_get_instance_size(aClassObject)==class_get_instance_size(isa))
        if ([self isKindOf:aClassObject])
          {
            Class old_isa = isa;
            isa = aClassObject;
            return old_isa;
          }
  return nil;
}

- subclassResponsibility:(SEL)aSel
{
  return [self error:"subclass should override %s", sel_get_name(aSel)];
}

- notImplemented:(SEL)aSel
{
  return [self error:"method %s not implemented", sel_get_name(aSel)];
}

- shouldNotImplement:(SEL)aSel
{
  return [self error:"%s should not implement %s", 
	             object_get_class_name(self), sel_get_name(aSel)];
}

- doesNotRecognize:(SEL)aSel
{
  return [self error:"%s does not recognize %s",
                     object_get_class_name(self), sel_get_name(aSel)];
}

- error:(const char *)aString, ...
{
#define FMT "error: %s (%s)\n%s\n"
  char fmt[(strlen((char*)FMT)+strlen((char*)object_get_class_name(self))
            +((aString!=NULL)?strlen((char*)aString):0)+8)];
  va_list ap;

  sprintf(fmt, FMT, object_get_class_name(self),
                    object_is_instance(self)?"instance":"class",
                    (aString!=NULL)?aString:"");
  va_start(ap, aString);
  objc_verror(self, 0, fmt, ap);
  va_end(ap);
  return nil;
#undef FMT
}

@end
