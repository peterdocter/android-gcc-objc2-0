// Define NO_GC to disable garbage collection.
// Be sure to edit OBJC_NO_GC in objc-auto.h as well.
#define NO_GC 1

// Define NO_ENVIRON to disable getenv().
#define NO_ENVIRON 1

// Define NO_ZONES to disable malloc zone support in NXHashTable.
#define NO_ZONES 1

// Define NO_MOD to avoid the mod operator in NXHashTable and objc-sel-set.
#if defined(__arm__)
#   define NO_MOD 1
#endif

// Define NO_FIXUP to use non-fixup messaging for OBJC2.
#if defined(__arm__)
#   define NO_FIXUP 1
#endif

// Define NO_VTABLE to disable vtable dispatch for OBJC2.
#if defined(NO_FIXUP)  ||  defined(__ppc64__)
#   define NO_VTABLE 1
#endif

// Define NO_ZEROCOST_EXCEPTIONS to use sjlj exceptions for OBJC2.
// Be sure to edit objc-exception.h as well (objc_add/removeExceptionHandler)
#if defined(__arm__)
#   define NO_ZEROCOST_EXCEPTIONS 1
#endif


// OBJC_INSTRUMENTED controls whether message dispatching is dynamically
// monitored.  Monitoring introduces substantial overhead.
// NOTE: To define this condition, do so in the build command, NOT by
// uncommenting the line here.  This is because objc-class.h heeds this
// condition, but objc-class.h can not #include this file (objc-config.h)
// because objc-class.h is public and objc-config.h is not.
//#define OBJC_INSTRUMENTED

// Get the nice macros for subroutine calling, etc.
// Not available on all architectures.  Not needed
// (by us) on some configurations.
#if defined (__i386__) || defined (i386)
#   include <architecture/i386/asm_help.h>
#elif defined (__ppc__) || defined(ppc)
#   include <architecture/ppc/asm_help.h>
#endif
