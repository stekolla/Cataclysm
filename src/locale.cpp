#include "locale.h"

#include <clocale>
#include <exception>
#include <locale>
#include <string>

#include "catacharset.h"
#if defined(MACOSX)
#  include <CoreFoundation/CoreFoundation.h>
#endif
#include "translations.h"

#include "name.h"
#include "path_info.h"

auto init_locale() -> bool
{
#if defined(_WIN32)
    if( setlocale( LC_ALL, ".1252" ) == nullptr ) {
#else
    if( setlocale( LC_ALL, "" ) == nullptr ) {
#endif
        return false;
    }
    try {
        std::locale::global( std::locale( "" ) );
    } catch( const std::exception & ) {
        std::locale::global( std::locale::classic() );
    }
    return true;
}

auto load_english_names() -> void
{
    Name::clear();
    Name::load_from_file( PATH_INFO::names() );
}

bool localized_comparator::operator()( const std::string &l, const std::string &r ) const
{
    // We need different implementations on each platform.  MacOS seems to not
    // support localized comparison of strings via the standard library at all,
    // so resort to MacOS-specific solution.  Windows cannot be expected to be
    // using a UTF-8 locale (whereas our strings are always UTF-8) and so we
    // must convert to wstring for comparison there.  Linux seems to work as
    // expected on regular strings; no workarounds needed.
    // See https://github.com/CleverRaven/Cataclysm-DDA/pull/40041 for further
    // discussion.
#if defined(MACOSX)
    CFStringRef lr = CFStringCreateWithCStringNoCopy( kCFAllocatorDefault, l.c_str(),
                     kCFStringEncodingUTF8, kCFAllocatorNull );
    CFStringRef rr = CFStringCreateWithCStringNoCopy( kCFAllocatorDefault, r.c_str(),
                     kCFStringEncodingUTF8, kCFAllocatorNull );
    bool result = CFStringCompare( lr, rr, kCFCompareLocalized ) < 0;
    CFRelease( lr );
    CFRelease( rr );
    return result;
#elif defined(_WIN32)
    return ( *this )( utf8_to_wstr( l ), utf8_to_wstr( r ) );
#else
    return std::locale()( l, r );
#endif
}

bool localized_comparator::operator()( const std::wstring &l, const std::wstring &r ) const
{
#if defined(MACOSX)
    return ( *this )( wstr_to_utf8( l ), wstr_to_utf8( r ) );
#else
    return std::locale()( l, r );
#endif
}
