# Vendored LLVM — provenance and the Cact additions

This repository is the CactOS port of the C++ runtime: the upstream
`llvm-project` tree is carried in place, but only **libc++** and **libc++abi**
are built, into the loader's reserved bases `0x28000000` and `0x29000000`.

The rest of `llvm-project` (clang, LLVM itself, libc, the runtimes) is present
because the tree is one repository, and because the ports that follow this one —
LLVM for llvmpipe, then Mesa — are built from the same sources. Nothing outside
`libcxx/` and `libcxxabi/` is compiled here.

| | |
|---|---|
| Upstream | <https://github.com/llvm/llvm-project> |
| Ref | tag `llvmorg-23.1.5` — commit `392dfff9` |
| Branch | `cact` (upstream tag + the Cact commits) |
| Sync tool | none yet — this port has no `sync.conf` |

The tag matches the host `clang`/`clang++` (23.1.5): the workspace ports are
compiled by that compiler, and libc++ and the compiler must agree on the ABI.

**Removed from upstream:** nothing. Upstream is CMake-based, and its
`CMakeLists.txt` files are left where they are — the point is that this port
does not *use* them.

## Files CactOS adds

| Path | Role |
|---|---|
| `meson.build` | Builds the two libraries; the source lists are upstream's |
| `meson_options.txt` | `cactlib`, `lr_lib`, `cpkg_py` paths |
| `cact/__config_site` | The configuration upstream's CMake would generate |
| `cact/dso_handle.S` | `__dso_handle` for a shared object (what crtbeginS.o gives) |
| `cact/unwind.h` | The one `_Unwind_*` type libc++abi's ABI needs, exceptions off |
| `cact/library.ld.in` | Fixed-base linker-script template (meson substitutes `@base@`) |
| `cact/program.ld` | The program-side script, for the smoke test's `.init_array` |
| `cact/smoke.cc` | The C++ smoke program (`ninja smoke`) |
| `cross/x86_64-cact-clang.ini` | The workspace's cross file, plus `cpp = clang++` |
| `libcxx/include/__locale_dir/support/cactos.h` | The locale shim the C locale needs |
| `pkg/libcxx/manifest` | `.cpkg` directory-package manifest (both libraries) |
| `PORTING-x86_64.md` | What "building for CactOS" means here |

## Deliberate deviations from upstream

1. **The build is CactOS's own Meson build, not upstream's CMake.** libc++ is
   CMake-based with a source list upstream moves between releases; the port
   carries that list (the files upstream's `libcxx/src/CMakeLists.txt` and
   `libcxxabi/src/CMakeLists.txt` name, minus the groups CactOS does not build).
   Upstream's CMake is not run.
2. **One upstream file is edited** — `libcxx/include/__locale_dir/locale_base_api.h`
   gains an `#elif defined(__CACT__)` that selects the port's locale shim. It is
   the only edit; a sync onto a newer tag conflicts there.
3. **`cact/__config_site` is written by hand.** Upstream has CMake probe a hosted
   system for it; CactOS writes the answers down once. Threads are on (CactLibc
   has pthreads), localization is C-locale-only, wide characters, `<filesystem>`,
   `std::random_device` and the time-zone database are off.
4. **`-nostdlibinc`** (the host libc's and host libstdc++'s headers are not
   searched; clang's builtin `stddef.h`/`stdarg.h` still are), so a header
   CactLibc lacks is a hard error rather than a silent grab of the host's copy.
5. **`-U__linux__`**: clang targets `x86_64-pc-linux-gnu`, and libc++'s Linux
   branches want headers CactOS does not have (`<linux/futex.h>`) and glibc's
   `_l` locale functions. The portable branches are the correct ones.
6. **Exceptions and RTTI are off** (`-fno-exceptions`); `cxa_noexception.cpp`
   stands in for `cxa_exception.cpp`/`cxa_personality.cpp`, exactly as upstream's
   `LIBCXXABI_ENABLE_EXCEPTIONS=OFF` build does.
7. **`-fvisibility=hidden` + `-fvisibility-inlines-hidden`**, as upstream's
   library build flags: only the public API is exported.
8. **Sonames are unversioned** (`libc++.so`, not `libc++.so.1`): ld.so and
   `dlopen()` resolve a `DT_NEEDED` name as a file under `/usr/lib`, and the
   cctkfs image carries no symlinks.
9. **`-w`**: vendored third-party code.

Nothing outside this list is changed, and no other upstream file is edited.
