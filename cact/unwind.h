/* Minimal <unwind.h> for CactOS.
 *
 * libc++abi's cxa_exception.h embeds `_Unwind_Exception` at the *end* of
 * __cxa_exception and static_asserts the negative offsets of its own fields, so
 * the type's size and alignment are part of the Itanium ABI, not a detail the
 * port may choose.  This header provides exactly that type, in the x86_64
 * Itanium layout a GCC/clang unwind.h gives:
 *
 *   exception_class (8) | exception_cleanup (8) | private_1 (8) | private_2 (8)
 *   size 32, alignment 16
 *
 * Nothing here unwinds: the port builds with exceptions off
 * (LIBCXXABI_ENABLE_EXCEPTIONS=OFF → cxa_noexception.cpp), which is why only
 * the type and the class-word helpers are needed and no _Unwind_* function is
 * declared.  A real unwinder (libunwind from compiler-rt) is a later stage, and
 * it will bring its own <unwind.h> — see PORTING-x86_64.md. */

#ifndef _CACT_UNWIND_H
#define _CACT_UNWIND_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long      _Unwind_Word;
typedef long               _Unwind_Sword;
typedef void              *_Unwind_Ptr;
typedef unsigned long long _Unwind_Exception_Class;

typedef int _Unwind_Reason_Code;
#define _URC_NO_REASON 0
#define _URC_FOREIGN_EXCEPTION_CAUGHT 1
#define _URC_FATAL_PHASE1_ERROR 3
#define _URC_FATAL_PHASE2_ERROR 2
#define _URC_END_OF_STACK 5
#define _URC_HANDLER_FOUND 6
#define _URC_INSTALL_CONTEXT 7
#define _URC_CONTINUE_UNWIND 8

typedef int _Unwind_Action;
#define _UA_SEARCH_PHASE 1
#define _UA_CLEANUP_PHASE 2
#define _UA_HANDLER_FRAME 4
#define _UA_FORCE_UNWIND 8
#define _UA_END_OF_STACK 16

struct _Unwind_Context;
typedef struct _Unwind_Context _Unwind_Context;

typedef void (*_Unwind_Exception_Cleanup_Fn)(_Unwind_Reason_Code,
                                             struct _Unwind_Exception *);

struct _Unwind_Exception {
    _Unwind_Exception_Class exception_class;
    _Unwind_Exception_Cleanup_Fn exception_cleanup;
    _Unwind_Word private_1;
    _Unwind_Word private_2;
} __attribute__((__aligned__));

#ifdef __cplusplus
}
#endif

#endif /* _CACT_UNWIND_H */
