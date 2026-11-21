//===----------------------------------------------------------------------===//
//
// CactOS locale base API for libc++.
//
// libc++ implements its own facets and asks the platform only for the few
// C-library primitives a locale needs (newlocale/freelocale/setlocale,
// strtof/strtod/strtold, strcoll/strxfrm, strftime, MB_CUR_MAX, and
// snprintf).  CactOS has exactly one locale — the C locale — and CactLibc has
// all of those, so this header takes libc++'s "no locale" route, the same one
// __locale_dir/support/llvm_libc.h takes, and binds the primitives to the
// current-locale C functions (there is no *_l family on CactOS).
//
// Selected by __locale_dir/locale_base_api.h on __CACT__; that #elif is the
// port's only edit to an upstream file (see UPSTREAM.md).
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___LOCALE_DIR_SUPPORT_CACTOS_H
#define _LIBCPP___LOCALE_DIR_SUPPORT_CACTOS_H

#include <__config>
#include <clocale>    // std::lconv, LC_*_MASK, locale_t
#include <cstdio>
#include <cstdlib>    // MB_CUR_MAX
#include <ctype.h>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD
namespace __locale {

using __locale_t _LIBCPP_NODEBUG = ::locale_t;

#define _LIBCPP_COLLATE_MASK LC_COLLATE_MASK
#define _LIBCPP_CTYPE_MASK LC_CTYPE_MASK
#define _LIBCPP_MONETARY_MASK LC_MONETARY_MASK
#define _LIBCPP_NUMERIC_MASK LC_NUMERIC_MASK
#define _LIBCPP_TIME_MASK LC_TIME_MASK
#define _LIBCPP_MESSAGES_MASK LC_MESSAGES_MASK
#define _LIBCPP_ALL_MASK LC_ALL_MASK
#define _LIBCPP_LC_ALL LC_ALL

#if defined(_LIBCPP_BUILDING_LIBRARY)
using __lconv_t _LIBCPP_NODEBUG = std::lconv;

inline _LIBCPP_HIDE_FROM_ABI __locale_t __newlocale(int __category_mask, const char* __locale, __locale_t __base) {
  return ::newlocale(__category_mask, __locale, __base);
}

inline _LIBCPP_HIDE_FROM_ABI void __freelocale(__locale_t __loc) { ::freelocale(__loc); }

inline _LIBCPP_HIDE_FROM_ABI char* __setlocale(int __category, char const* __locale) {
  return ::setlocale(__category, __locale);
}

// No per-locale object exists to pass through: every locale_t is the C locale,
// and localeconv() is already answering for it.
inline _LIBCPP_HIDE_FROM_ABI __lconv_t* __localeconv(__locale_t&) { return ::localeconv(); }

// Only used by std::text_encoding::environment() (C++26), where a returned
// null would be a missing answer; the C locale's encoding is ASCII.
inline _LIBCPP_HIDE_FROM_ABI const char* __get_locale_encoding(__locale_t) { return "ASCII"; }
#endif // _LIBCPP_BUILDING_LIBRARY

} // namespace __locale
_LIBCPP_END_NAMESPACE_STD

#include <__locale_dir/support/no_locale/characters.h>
#include <__locale_dir/support/no_locale/conversions.h>
#include <__locale_dir/support/no_locale/formatting.h>
#include <__locale_dir/support/no_locale/strtonum.h>

#endif // _LIBCPP___LOCALE_DIR_SUPPORT_CACTOS_H
