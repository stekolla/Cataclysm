#include "translations.h"

#include <functional>

#include "cached_options.h"
#include "json.h"
#include "string_utils.h"

const char *detail::_translate_internal( const char *msg ) // *NOPAD*
{
    return msg;
}

const char *vgettext( const char *msgid, const char *msgid_plural, size_t n ) // *NOPAD*
{
    return n == 1 ? msgid : msgid_plural;
}

const char *pgettext( const char * /*context*/, const char *msgid ) // *NOPAD*
{
    return msgid;
}

const char *vpgettext( const char *const /*context*/, const char *const msgid,
                       const char *const msgid_plural, const size_t n ) // *NOPAD*
{
    return n == 1 ? msgid : msgid_plural;
}

translation::translation( const plural_tag ) : raw_pl( cata::make_value<std::string>() ) {}

translation::translation( const std::string &ctxt, const std::string &raw )
    : ctxt( cata::make_value<std::string>( ctxt ) ), raw( raw ), needs_translation( true )
{
}

translation::translation( const std::string &raw )
    : raw( raw ), needs_translation( true )
{
}

translation::translation( const std::string &raw, const std::string &raw_pl,
                          const plural_tag )
    : raw( raw ), raw_pl( cata::make_value<std::string>( raw_pl ) ), needs_translation( true )
{
}

translation::translation( const std::string &ctxt, const std::string &raw,
                          const std::string &raw_pl, const plural_tag )
    : ctxt( cata::make_value<std::string>( ctxt ) ),
      raw( raw ), raw_pl( cata::make_value<std::string>( raw_pl ) ), needs_translation( true )
{
}

translation::translation( const std::string &str, const no_translation_tag ) : raw( str ) {}

translation translation::to_translation( const std::string &raw )
{
    return { raw };
}

translation translation::to_translation( const std::string &ctxt, const std::string &raw )
{
    return { ctxt, raw };
}

translation translation::pl_translation( const std::string &raw, const std::string &raw_pl )
{
    return { raw, raw_pl, plural_tag() };
}

translation translation::pl_translation( const std::string &ctxt, const std::string &raw,
        const std::string &raw_pl )
{
    return { ctxt, raw, raw_pl, plural_tag() };
}

translation translation::no_translation( const std::string &str )
{
    return { str, no_translation_tag() };
}

void translation::make_plural()
{
    if( needs_translation ) {
        // if plural form has not been enabled yet
        if( !raw_pl ) {
            // copy the singular string without appending "s" to preserve the original behavior
            raw_pl = cata::make_value<std::string>( raw );
        }
    } else if( !raw_pl ) {
        // just mark plural form as enabled
        raw_pl = cata::make_value<std::string>();
    }
    // reset the cache
    cached_translation = nullptr;
}

void translation::add_context( const std::string &ctxt )
{
    if( this->ctxt ) {
        // if context already exists, add to it
        std::string &ctxt_this = *this->ctxt;
        ctxt_this += "|";
        ctxt_this += ctxt;
    } else {
        this->ctxt = cata::make_value<std::string>( ctxt );
    }
    // reset the cache
    cached_translation = nullptr;
}

void translation::deserialize( JsonIn &jsin )
{
    // reset the cache
    cached_translation = nullptr;

#ifndef CATA_IN_TOOL
    bool check_style = false;
    std::function<void( const std::string &msg, int offset )> log_error;
    bool auto_plural = false;
    bool is_str_sp = false;
#endif
    if( jsin.test_string() ) {
#ifndef CATA_IN_TOOL
        if( test_mode ) {
            const int origin = jsin.tell();
            check_style = true;
            log_error = [&jsin, origin]( const std::string & msg, const int offset ) {
                const int previous_pos = jsin.tell();
                try {
                    jsin.seek( origin );
                    jsin.string_error( msg, offset );
                } catch( const JsonError &e ) {
                    debugmsg( "(json-error)\n%s", e.what() );
                }
                // seek to previous pos (end of string) so subsequent json input
                // can continue.
                jsin.seek( previous_pos );
            };
        }
#endif
        ctxt = nullptr;
        raw = jsin.get_string();
        // if plural form is enabled
        if( raw_pl ) {
            raw_pl = cata::make_value<std::string>( raw + "s" );
            auto_plural = true;
        }
        needs_translation = true;
    } else {
        JsonObject jsobj = jsin.get_object();
        if( jsobj.has_string( "ctxt" ) ) {
            ctxt = cata::make_value<std::string>( jsobj.get_string( "ctxt" ) );
        } else {
            ctxt = nullptr;
        }
        if( jsobj.has_member( "str_sp" ) ) {
            // same singular and plural forms
            raw = jsobj.get_string( "str_sp" );
            is_str_sp = true;
            // if plural form is enabled
            if( raw_pl ) {
                raw_pl = cata::make_value<std::string>( raw );
            } else {
                try {
                    jsobj.throw_error( "str_sp not supported here", "str_sp" );
                } catch( const JsonError &e ) {
                    debugmsg( "(json-error)\n%s", e.what() );
                }
            }
        } else {
            raw = jsobj.get_string( "str" );
            // if plural form is enabled
            if( raw_pl ) {
                if( jsobj.has_string( "str_pl" ) ) {
                    raw_pl = cata::make_value<std::string>( jsobj.get_string( "str_pl" ) );
                } else {
                    raw_pl = cata::make_value<std::string>( raw + "s" );
                    auto_plural = true;
                }
            } else if( jsobj.has_string( "str_pl" ) ) {
                try {
                    jsobj.throw_error( "str_pl not supported here", "str_pl" );
                } catch( const JsonError &e ) {
                    debugmsg( "(json-error)\n%s", e.what() );
                }
            }
        }
        needs_translation = true;
#ifndef CATA_IN_TOOL
        if( test_mode ) {
            check_style = !jsobj.has_member( "//NOLINT(cata-text-style)" );
            // Copying jsobj to avoid use-after-free
            log_error = [jsobj]( const std::string & msg, const int offset ) {
                try {
                    if( jsobj.has_member( "str" ) ) {
                        jsobj.get_raw( "str" )->string_error( msg, offset );
                    } else {
                        jsobj.get_raw( "str_sp" )->string_error( msg, offset );
                    }
                } catch( const JsonError &e ) {
                    debugmsg( "(json-error)\n%s", e.what() );
                }
            };
        }
#endif
    }
#ifndef CATA_IN_TOOL
    // Check text style in translatable json strings.
    if( test_mode && check_style ) {
        if( raw_pl && !auto_plural && *raw_pl == raw + "s" ) {
            log_error( "\"str_pl\" is not necessary here since the "
                       "plural form can be automatically generated.",
                       0 );
        }
        if( !is_str_sp && raw_pl && !auto_plural && raw == *raw_pl ) {
            log_error( "Please use \"str_sp\" instead of \"str\" and \"str_pl\" "
                       "for text with identical singular and plural forms",
                       0 );
        }
    }
#endif
}

std::string translation::translated( const int num ) const
{
    if( !needs_translation || raw.empty() ) {
        return raw;
    }
    // A count selects the cached plural form when one is present.
    if( ( raw_pl && cached_num != num ) || !cached_translation ) {
        cached_num = num;

        cached_translation = cata::make_value<std::string>( raw_pl && num != 1 ? *raw_pl : raw );
    }
    return *cached_translation;
}

bool translation::empty() const
{
    return raw.empty();
}

bool translation::translated_lt( const translation &that ) const
{
    return localized_compare( translated(), that.translated() );
}

bool translation::translated_eq( const translation &that ) const
{
    return translated() == that.translated();
}

bool translation::translated_ne( const translation &that ) const
{
    return !translated_eq( that );
}

bool translation::operator==( const translation &that ) const
{
    return value_ptr_equals( ctxt, that.ctxt ) && raw == that.raw &&
           value_ptr_equals( raw_pl, that.raw_pl ) &&
           needs_translation == that.needs_translation;
}

bool translation::operator!=( const translation &that ) const
{
    return !operator==( that );
}

std::pair<bool, int> translation::legacy_hash() const
{
    if( needs_translation && !ctxt && !raw_pl ) {
        return {true, djb2_hash( reinterpret_cast<const unsigned char *>( raw.c_str() ) )};
    }
    // Otherwise the translation must have been added after snippets were changed
    // to use string ids only, so the translation doesn't have a legacy hash value.
    return {false, 0};
}

translation to_translation( const std::string &raw )
{
    return translation::to_translation( raw );
}

translation to_translation( const std::string &ctxt, const std::string &raw )
{
    return translation::to_translation( ctxt, raw );
}

translation pl_translation( const std::string &raw, const std::string &raw_pl )
{
    return translation::pl_translation( raw, raw_pl );
}

translation pl_translation( const std::string &ctxt, const std::string &raw,
                            const std::string &raw_pl )
{
    return translation::pl_translation( ctxt, raw, raw_pl );
}

translation no_translation( const std::string &str )
{
    return translation::no_translation( str );
}

std::ostream &operator<<( std::ostream &out, const translation &t )
{
    return out << t.translated();
}

std::string operator+( const translation &lhs, const std::string &rhs )
{
    return lhs.translated() + rhs;
}

std::string operator+( const std::string &lhs, const translation &rhs )
{
    return lhs + rhs.translated();
}

std::string operator+( const translation &lhs, const translation &rhs )
{
    return lhs.translated() + rhs.translated();
}
