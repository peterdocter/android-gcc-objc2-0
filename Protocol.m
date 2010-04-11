/* This file contains the implementation of class Protocol.
   Copyright (C) 1993, 2004 Free Software Foundation, Inc.

This file is part of GCC. 

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to
the Free Software Foundation, 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.  */
 
/* As a special exception, if you link this library with files
   compiled with GCC to produce an executable, this does not cause
   the resulting executable to be covered by the GNU General Public License.
   This exception does not however invalidate any other reasons why
   the executable file might be covered by the GNU General Public License.  */

#include "objc/Protocol.h"
#include "objc/objc-api.h"
#import "objc-private.h"

/* Method description list */
struct objc_method_description_list {
        int count;
        struct objc_method_description list[1];
};


@implementation Protocol

// fixme hack - make Protocol a non-lazy class
+ (void) load { } 

typedef struct {
    uintptr_t count;
    Protocol *list[0];
} protocol_list_t;

- (BOOL) conformsTo: (Protocol *)aProtocolObj
{
    return protocol_conformsToProtocol(self, aProtocolObj);
}

- (struct objc_method_description *) descriptionForInstanceMethod:(SEL)aSel
{
    return method_getDescription(_protocol_getMethod(self, aSel, YES, YES));
}

- (struct objc_method_description *) descriptionForClassMethod:(SEL)aSel
{
    return method_getDescription(_protocol_getMethod(self, aSel, YES, NO));
}

- (const char *)name
{
    return protocol_getName(self);
}

- (BOOL)isEqual:other
{
#if __OBJC2__
    // check isKindOf:
    Class cls;
    Class protoClass = objc_getClass("Protocol");
    for (cls = other->isa; cls; cls = class_getSuperclass(cls)) {
        if (cls == protoClass) break;
    }
    if (!cls) return NO;
    // check equality
    return protocol_isEqual(self, other);
#else
    return [other isKindOf:[Protocol class]] && [self conformsTo: other] && [other conformsTo: self];
#endif
}

- (unsigned int)hash
{
    return 23;
}
@end

