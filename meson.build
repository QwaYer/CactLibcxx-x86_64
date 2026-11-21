# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Copyright (C) 2026 CactOS Project

# ==============================================================================
# CactLibcxx-x86_64 — the C++ runtime (libc++ and libc++abi) for CactOS
# ==============================================================================
#
# Upstream sources are carried in place, but the build system is CactOS's own:
# upstream's CMake is not used (see UPSTREAM.md) and only the two libraries are
# built, each into its own loader base
#
#   libc++.so      0x28000000
#   libc++abi.so   0x29000000
#
#   meson setup build-meson --cross-file cross/x86_64-cact-clang.ini
#   ninja -C build-meson            # the two .so files
#   ninja -C build-meson smoke      # a C++ program linked against them
#   ninja -C build-meson stage      # the two .so files into LocalRepo lib/
#   ninja -C build-meson pkg        # repo/libcxx-<ver>.cpkg
#
# libc++ is not in the image yet: nothing in it is C++ (the compositor is
# deferred), so staging it would only add weight.  The two targets above are the
# port's own deployment path — a manual run now, and what the image will call
# when a C++ consumer lands.  The C++ headers are build-time only and stay in
# this tree, where a host consumer (LLVM, Mesa) is pointed by explicit path —
# the same wiring the other ports use instead of pkg-config.
#
# When the headers are used they have to be searched as <prefix>/include/c++/v1
# and not flat in /usr/include: libc++ ships <string.h>, <stdio.h>, <stdlib.h>,
# <math.h> and <errno.h> as wrappers that #include_next the C library's, and
# alongside CactLibc's own headers they would overwrite each other.
#
# Configuration lives in cact/__config_site (a hand-written replacement for the
# file upstream's CMake generates) and the locale shim in
# libcxx/include/__locale_dir/support/cactos.h.  What CactOS does not have —
# wide characters, std::filesystem, exceptions — is off; see PORTING-x86_64.md.
# ==============================================================================

project('cact-libcxx', 'cpp',
  version: '23.1.5',
  meson_version: '>=1.0.0',
  default_options: [
    'cpp_std=none',               # -std=c++26 is passed below, as upstream does
    'warning_level=0',            # vendored third-party code; -w below
    'buildtype=plain',
    'b_staticpic=false',
  ])

fs = import('fs')
bash = find_program('bash')
ld = find_program('ld')
python3 = find_program('python3')

src_root = meson.project_source_root()
build_root = meson.current_build_dir()

_cactlib = get_option('cactlib')
cactlib = _cactlib.startswith('/') ? _cactlib : src_root / _cactlib
_lr_lib = get_option('lr_lib')
lr_lib = _lr_lib.startswith('/') ? _lr_lib : src_root / _lr_lib
_cpkg_py = get_option('cpkg_py')
cpkg_py = _cpkg_py.startswith('/') ? _cpkg_py : src_root / _cpkg_py

libcxx_src   = src_root / 'libcxx'
libcxxabi_src = src_root / 'libcxxabi'
libcxx_inc   = libcxx_src / 'include'

cactlib_inc = cactlib / 'include'
if not fs.is_dir(cactlib_inc)
  error('CactLibc headers not found at ' + cactlib_inc +
        '  —  checkout the sibling repo or pass -Dcactlib=<path>')
endif
foreach h : ['uchar.h', 'locale.h', 'pthread.h']
  if not fs.is_file(cactlib_inc / h)
    error('CactLibc is missing ' + h + ' — libc++ needs it (see PORTING-x86_64.md)')
  endif
endforeach

# ------------------------------------------------------------------------------
# Generated configuration and linker scripts
# ------------------------------------------------------------------------------

# The file upstream's CMake writes from include/__config_site.in; the CactOS
# answers are written down once, in cact/__config_site.
config_site = configure_file(input: 'cact/__config_site',
                             output: '__config_site', copy: true)

# The other header upstream generates: __assertion_handler is copied verbatim
# from the vendor's handler (upstream's default is libcxx/vendor/llvm/).
assertion_handler = configure_file(
  input: 'libcxx/vendor/llvm/default_assertion_handler.in',
  output: '__assertion_handler', copy: true)

# __dso_handle for a shared library: clang emits a hidden reference to it in
# every object that has a namespace-scope object with a destructor, and hidden
# means the definition has to be in the same image.  A program gets one from
# CactLibc's start.o; a .so carries this copy, the way crtbeginS.o provides one
# to a hosted shared object (see cact/dso_handle.S).
dso_handle = custom_target('dso-handle',
  input: 'cact/dso_handle.S',
  output: 'dso_handle.o',
  command: meson.get_compiler('cpp').cmd_array() +
           ['-m64', '-fPIC', '-c', '@INPUT@', '-o', '@OUTPUT@'],
  build_by_default: true)

_ld_cxx = configuration_data()
_ld_cxx.set('base', '0x28000000')
ld_cxx = configure_file(input: 'cact/library.ld.in', output: 'libc++.ld',
                        configuration: _ld_cxx)
_ld_abi = configuration_data()
_ld_abi.set('base', '0x29000000')
ld_abi = configure_file(input: 'cact/library.ld.in', output: 'libc++abi.ld',
                        configuration: _ld_abi)

# ------------------------------------------------------------------------------
# Compile
# ------------------------------------------------------------------------------

# -nostdlibinc: the host libc's *and* host C++ library's headers are not searched
# (clang's builtin stddef.h/stdarg.h still are), so a header CactLibc lacks is a
# hard error instead of a silent grab of the build machine's glibc/libstdc++.
# -nostdinc++ is what upstream passes; it is redundant under -nostdlibinc here,
# but kept because it says the intent.
# -U__linux__: clang targets x86_64-pc-linux-gnu, and libc++'s Linux branches
# want headers CactOS does not have (`<linux/futex.h>` for std::atomic's wait)
# and glibc's `_l` locale functions.  The portable branches are the correct
# ones, exactly as in the libffi port.
# -D_LIBCPP_HAS_CLOCK_GETTIME: libc++ decides this from `_POSIX_TIMERS`, and
# CactOS has no POSIX timers — but it does have clock_gettime(CLOCK_MONOTONIC),
# which is all the macro is used for (std::chrono's steady_clock).
# The macros and visibility flags are upstream's library-build set
# (cxx_add_basic_build_flags); -w is for vendored third-party code.
cxx_args = [
  '-m64',
  '-std=c++26',                 # upstream compiles the library's own sources as C++26
  '-ffreestanding', '-fno-stack-protector', '-nostdlib', '-nostdlibinc', '-nostdinc++',
  '-fno-exceptions', '-U__linux__', '-D_LIBCPP_HAS_CLOCK_GETTIME',
  '-fvisibility=hidden', '-fvisibility-inlines-hidden', '-fsized-deallocation',
  '-D__CACT__',
  '-D_LIBCPP_BUILDING_LIBRARY',
  '-D_LIBCPP_REMOVE_TRANSITIVE_INCLUDES',
  '-I' + build_root,            # the generated __config_site
  '-isystem', libcxx_inc,       # libc++ headers
  '-isystem', libcxx_src / 'src', # libc++abi's "include/..." quoted includes
  '-isystem', cactlib_inc,      # CactLibc headers
  '-isystem', src_root / 'libc', # llvm-libc's shared/ + src/__support: libc++'s
                                 # float <charconv> is built on them
  '-w',
]

abi_args = cxx_args + [
  '-D_LIBCXXABI_BUILDING_LIBRARY',
  '-isystem', libcxxabi_src / 'include',  # cxxabi.h
  '-isystem', src_root / 'cact',          # the port's minimal <unwind.h>
]

# The sources are the lists of upstream's libcxx/src/CMakeLists.txt and
# libcxxabi/src/CMakeLists.txt, with the groups CactOS does not build removed:
# no std::filesystem (LIBCXX_ENABLE_FILESYSTEM=OFF), no std::random_device, no
# exceptions (cxa_noexception.cpp stands in for cxa_exception/cxa_personality),
# and operator new/delete come from libc++abi (upstream's default placement).
cxx_sources = files(
  'libcxx/src/algorithm.cpp',
  'libcxx/src/any.cpp',
  'libcxx/src/atomic.cpp',
  'libcxx/src/barrier.cpp',
  'libcxx/src/bind.cpp',
  'libcxx/src/call_once.cpp',
  'libcxx/src/charconv.cpp',
  'libcxx/src/chrono.cpp',
  'libcxx/src/condition_variable.cpp',
  'libcxx/src/condition_variable_destructor.cpp',
  'libcxx/src/error_category.cpp',
  'libcxx/src/exception.cpp',
  'libcxx/src/expected.cpp',
  'libcxx/src/filesystem/filesystem_clock.cpp',
  'libcxx/src/filesystem/filesystem_error.cpp',
  'libcxx/src/filesystem/path.cpp',
  'libcxx/src/fstream.cpp',
  'libcxx/src/functional.cpp',
  'libcxx/src/future.cpp',
  'libcxx/src/hash.cpp',
  'libcxx/src/ios.cpp',
  'libcxx/src/ios.instantiations.cpp',
  'libcxx/src/iostream.cpp',
  'libcxx/src/locale.cpp',
  'libcxx/src/memory.cpp',
  'libcxx/src/memory_resource.cpp',
  'libcxx/src/mutex.cpp',
  'libcxx/src/mutex_destructor.cpp',
  'libcxx/src/new_handler.cpp',
  'libcxx/src/new_helpers.cpp',
  'libcxx/src/optional.cpp',
  'libcxx/src/ostream.cpp',
  'libcxx/src/print.cpp',
  'libcxx/src/random.cpp',
  'libcxx/src/random_shuffle.cpp',
  'libcxx/src/regex.cpp',
  'libcxx/src/ryu/d2fixed.cpp',
  'libcxx/src/ryu/d2s.cpp',
  'libcxx/src/ryu/f2s.cpp',
  'libcxx/src/shared_mutex.cpp',
  'libcxx/src/stdexcept.cpp',
  'libcxx/src/string.cpp',
  'libcxx/src/strstream.cpp',
  'libcxx/src/system_error.cpp',
  'libcxx/src/text_encoding.cpp',
  'libcxx/src/thread.cpp',
  'libcxx/src/typeinfo.cpp',
  'libcxx/src/valarray.cpp',
  'libcxx/src/variant.cpp',
  'libcxx/src/vector.cpp',
  'libcxx/src/verbose_abort.cpp',
)

abi_sources = files(
  'libcxxabi/src/abort_message.cpp',
  'libcxxabi/src/cxa_aux_runtime.cpp',
  'libcxxabi/src/cxa_default_handlers.cpp',
  'libcxxabi/src/cxa_demangle.cpp',
  'libcxxabi/src/cxa_exception_storage.cpp',
  'libcxxabi/src/cxa_guard.cpp',
  'libcxxabi/src/cxa_handlers.cpp',
  'libcxxabi/src/cxa_noexception.cpp',
  'libcxxabi/src/cxa_thread_atexit.cpp',
  'libcxxabi/src/cxa_vector.cpp',
  'libcxxabi/src/cxa_virtual.cpp',
  'libcxxabi/src/fallback_malloc.cpp',
  'libcxxabi/src/private_typeinfo.cpp',
  'libcxxabi/src/stdlib_exception.cpp',
  'libcxxabi/src/stdlib_new_delete.cpp',
  'libcxxabi/src/stdlib_stdexcept.cpp',
  'libcxxabi/src/stdlib_typeinfo.cpp',
)

abi_obj = static_library('libcxxabi-obj',
  abi_sources,
  cpp_args: abi_args, pic: true)

cxx_obj = static_library('libcxx-obj',
  cxx_sources,
  cpp_args: cxx_args, pic: true)

# ------------------------------------------------------------------------------
# Link
# ------------------------------------------------------------------------------

_link = [ld, '-m', 'elf_x86_64', '-shared', '-nostdlib', '--hash-style=both']

libcxxabi_so = custom_target('libc++abi.so',
  input: abi_obj, output: 'libc++abi.so',
  command: _link + ['-soname=libc++abi.so', '-T', ld_abi,
                    '-o', '@OUTPUT@', '--whole-archive', abi_obj,
                    '--no-whole-archive', dso_handle],
  depend_files: files('cact/library.ld.in'),
  build_by_default: true)

# libc++.so calls into libc++abi.so (__cxa_guard_*, operator new/delete, ...),
# so its link records a DT_NEEDED on it; the loader maps it from /usr/lib.
libcxx_so = custom_target('libc++.so',
  input: cxx_obj, output: 'libc++.so',
  command: _link + ['-soname=libc++.so', '-T', ld_cxx,
                    '-o', '@OUTPUT@', '--whole-archive', cxx_obj,
                    '--no-whole-archive', dso_handle, libcxxabi_so],
  depend_files: files('cact/library.ld.in'),
  build_by_default: true)

# ------------------------------------------------------------------------------
# Smoke test: a C++ program linked the way a CactOS program is
# ------------------------------------------------------------------------------
#
# The C library and the loader are the sibling repos' published artifacts — this
# target only checks that a C++ translation unit that includes <string>,
# <vector> and <iostream> compiles, links against libc++.so/libc++abi.so and
# produces a PIE the kernel can map.  Running it is the in-guest check
# (see PORTING-x86_64.md).

start_o = cactlib / 'build-meson' / 'start.o'
clibc_so = cactlib / 'build-meson' / 'clibc.so'

if fs.is_file(start_o) and fs.is_file(clibc_so)
  # A consumer's flags, not the library's: no _LIBCPP_BUILDING_LIBRARY, no
  # hidden visibility, and C++23 rather than the C++26 the library's own
  # sources use.
  #
  # No -ffreestanding here, unlike a CactOS C program: freestanding turns off
  # clang's special case for `main`, so a C++ program's entry point would be
  # emitted mangled (`_Z4mainv`) and start.o's `call main` would not resolve.
  # A C++ program is hosted against the C library and the C++ runtime anyway;
  # -nostdlibinc is what actually keeps the build machine's headers out.
  consumer_args = [
    '-m64', '-std=c++23',
    '-fno-stack-protector', '-nostdlib', '-nostdlibinc',
    '-fno-exceptions', '-U__linux__',
    '-D__CACT__',
    '-I' + build_root,
    '-isystem', libcxx_inc,
    '-isystem', cactlib_inc,
    '-w',
  ]
  smoke_obj = static_library('smoke-obj', files('cact/smoke.cc'),
    cpp_args: consumer_args, pic: true)

  smoke = custom_target('smoke-cxx',
    input: files('cact/smoke.cc'),
    output: 'smoke-cxx',
    command: [ld, '-m', 'elf_x86_64', '-pie', '--dynamic-linker=/lib/ld.so',
              '--hash-style=both', '-nostdlib', '--gc-sections',
              '-T', src_root / 'cact' / 'program.ld', '-o', '@OUTPUT@',
              start_o, '--whole-archive', smoke_obj, '--no-whole-archive',
              clibc_so, libcxx_so, libcxxabi_so],
    depend_files: files('cact/program.ld'),
    build_by_default: false)

  run_target('smoke', command: [bash, '-c', 'echo linked: $1', 'bash', smoke],
             depends: smoke)
else
  message('smoke target skipped: build CactLibc first (ninja -C ../CactLibc-x86_64/build-meson)')
endif

# ------------------------------------------------------------------------------
# Stage into the image tree and build the package
# ------------------------------------------------------------------------------

# The image does not stage libc++ (see the header above), so these targets are
# the port's own deployment path: a manual run, and what the image will call once
# a C++ consumer exists.  @LIBDIR@ is the image's lib dir for `stage` and the
# package payload's lib/ for `pkg`: cactpkg installs a package under /usr, and
# its default prefix /usr/local is not on ld.so's search path.
_copy_tpl = 'set -e; mkdir -p "@LIBDIR@"; cp -f "$1" "$2" "@LIBDIR@/"'

_pkg_dir = src_root / 'pkg' / 'libcxx'
_pkg_repo = src_root / 'repo'

_stage_copy = _copy_tpl.replace('@LIBDIR@', lr_lib)
_pkg_copy   = _copy_tpl.replace('@LIBDIR@', _pkg_dir / 'payload' / 'lib')

run_target('stage',
  command: [bash, '-c', _stage_copy, 'bash', libcxx_so, libcxxabi_so])

run_target('pkg',
  command: [bash, '-c',
            'set -e; rm -f "' + _pkg_repo + '"/*.cpkg; mkdir -p "' + _pkg_repo +
            '"; rm -rf "' + _pkg_dir + '/payload"; ' +
            _pkg_copy + '; ' +
            python3.full_path() + ' "' + cpkg_py + '" build "' + _pkg_dir +
            '" --repo "' + _pkg_repo + '"; ' +
            python3.full_path() + ' "' + cpkg_py + '" index "' + _pkg_repo + '"',
            'bash', libcxx_so, libcxxabi_so])
