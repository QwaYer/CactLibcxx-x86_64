/*
 * crtbegin's __dso_handle for a CactOS shared library.
 *
 * clang emits a hidden reference to __dso_handle in every object that has a
 * namespace-scope object with a destructor: the address is passed to
 * __cxa_atexit() as the image that owns the destructor.  Hidden means the
 * definition has to be in the same image, and a .so does not link the C
 * library's start.o (that is the program's crt), so each C++ .so carries its
 * own copy — exactly what crtbeginS.o provides a hosted shared object.
 *
 * Self-referential and hidden, as in crtstuff.c: the value is this object's own
 * address, and being local every image's copy is distinct.
 */
    .section .data,"aw",@progbits
    .hidden __dso_handle
    .globl  __dso_handle
    .type   __dso_handle, @object
    .align  8
__dso_handle:
    .quad   __dso_handle
    .size   __dso_handle, 8

    .section .note.GNU-stack,"",@progbits
