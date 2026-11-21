/*
 * PIE ET_DYN @ 0x08000000 — the CactOS program layout (CactUserBins' link.ld,
 * which is cactsole's).  This copy exists because the port's smoke program is
 * C++ and adds the three sections C++ brings; a C program does not need them.
 *
 * `.init_array` has to be inside the image: the loader collects each object's
 * DT_INIT_ARRAY at load time and the program's own is run before main(), so an
 * orphan section the linker placed on its own would not be in the mapped span.
 */
ENTRY(_start)

SECTIONS {
    . = 0x08000000;

    .dynamic   : { *(.dynamic) }
    .hash      : { *(.hash) }
    .gnu.hash  : { *(.gnu.hash) }
    .dynsym    : { *(.dynsym) }
    .dynstr    : { *(.dynstr) }
    .rela.dyn  : { *(.rela.dyn) }
    .rela.plt  : { *(.rela.plt) }
    .rel.dyn   : { *(.rel.dyn) }
    .rel.plt   : { *(.rel.plt) }

    .init      : { *(.init) }
    .plt       : { *(.plt) }
    .text      : { *(.text .text.*) }
    .fini      : { *(.fini) }

    .rodata    : { *(.rodata .rodata.*) }
    .eh_frame  : { *(.eh_frame) }
    .gcc_except_table : { *(.gcc_except_table .gcc_except_table.*) }

    . = ALIGN(0x1000);
    .data.rel.ro : { *(.data.rel.ro .data.rel.ro.*) }
    /* KEEP: nothing else references an init_array entry, so --gc-sections would
     * discard it and its constructor with it.  SORT_BY_INIT_PRIORITY keeps the
     * constructor_priority order. */
    .init_array : { KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*))) KEEP(*(.init_array)) }
    .fini_array : { KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*))) KEEP(*(.fini_array)) }
    .data      : { *(.data .data.*) }
    .got       : { *(.got) }
    .got.plt   : { *(.got.plt) }
    .bss       : { *(.bss .bss.*) *(COMMON) }
}
