#pragma once

#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <tuple>
#include <utility>

#include "value_ptr.h"

#if !defined(translate_marker)
/**
 * Keeps legacy string markers source-compatible. The argument is passed through unchanged.
 */
#define translate_marker(x) x
#endif
#if !defined(translate_marker_context)
/**
 * Keeps legacy contextual string markers source-compatible. The second argument is passed through.
 */
#define translate_marker_context(c, x) x
#endif

#if defined(__GNUC__)
#  define ATTRIBUTE_FORMAT_ARG(a) __attribute__((format_arg(a)))
#else
#  define ATTRIBUTE_FORMAT_ARG(a)
#endif

namespace detail
{
// same as _(), but without local cache
const char *_translate_internal( const char *msg ) ATTRIBUTE_FORMAT_ARG( 1 ); // *NOPAD*

// same as _(), but without local cache
inline std::string _translate_internal( const std::string &msg )
{
    return _translate_internal( msg.c_str() );
}

template<typename T>
class local_translation_cache;

template<>
class local_translation_cache<std::string>
{
    private:
        bool has_cached_arg = false;
        std::string cached_arg;
        std::string cached_translation;
    public:
        const std::string &operator()( const std::string &arg ) {
            if( !has_cached_arg || cached_arg != arg ) {
                cached_arg = arg;
                cached_translation = _translate_internal( arg );
                has_cached_arg = true;
            }
            return cached_translation;
        }
};

template<>
class local_translation_cache<const char *>
{
    private:
        bool has_cached_arg = false;
        std::string cached_arg;
        bool same_as_arg;
        const char *cached_translation;
    public:
        const char *operator()( const char *arg ) { // *NOPAD*
            if( !has_cached_arg || cached_arg != arg ) {
                cached_translation = _translate_internal( arg );
                same_as_arg = cached_translation == arg;
                cached_arg = arg;
                has_cached_arg = true;
            }
            // Return the current argument to preserve pointer identity.
            return same_as_arg ? arg : cached_translation;
        }
};

// these getters are used to work around the MSVC bug that happened with using decltype in lambda
// see build log: https://gist.github.com/Aivean/e76a70edce0a1589c76bcf754ffb016b
static inline local_translation_cache<const char *> get_local_translation_cache( const char * )
{
    return local_translation_cache<const char *>();
}
static inline local_translation_cache<std::string> get_local_translation_cache(
    const std::string & )
{
    return local_translation_cache<std::string>();
}

} // namespace detail

// Note: in case of std::string argument, the result is copied, this is intended (for safety)
#define _( msg ) \
    ( ( []( const auto & arg ) { \
        static auto cache = ::detail::get_local_translation_cache( arg ); \
        return cache( arg ); \
    } )( msg ) )

const char *vgettext( const char *msgid, const char *msgid_plural,
                      size_t n ) ATTRIBUTE_FORMAT_ARG( 1 ); // *NOPAD*

const char *pgettext( const char *context, const char *msgid ) ATTRIBUTE_FORMAT_ARG( 2 ); // *NOPAD*

// same as pgettext, but supports plural forms like vgettext
const char *vpgettext( const char *context, const char *msgid, const char *msgid_plural,
                       size_t n ) ATTRIBUTE_FORMAT_ARG( 2 ); // *NOPAD*


class JsonIn;

/**
 * Class for storing English strings and their optional plural forms.
 **/
class translation
{
    public:
        struct plural_tag {};

        // need to have user-defined constructor to work around clang 3.8 bug
        // translation() = default doesn't work!
        // see: https://stackoverflow.com/a/47368753/1349366
        // NOLINTNEXTLINE default constructor
        translation() {};
        /**
         * Same as `translation()`, but with plural form enabled.
         **/
        translation( plural_tag );

        /**
         * Store a string, an optional plural form, and an optional legacy context
         **/
        static translation to_translation( const std::string &raw );
        static translation to_translation( const std::string &ctxt, const std::string &raw );
        static translation pl_translation( const std::string &raw, const std::string &raw_pl );
        static translation pl_translation( const std::string &ctxt, const std::string &raw,
                                           const std::string &raw_pl );
        /**
         * Store a string without a plural form.
         **/
        static translation no_translation( const std::string &str );

        /**
         * Can be used to ensure a translation object has plural form enabled
         * before loading into it from JSON. If plural form has not been enabled
         * yet, the plural string will be set to the original singular string.
         * `vgettext` will ignore the new plural string and use the English singular form.
         *     Note that a `make_singular()` function is not provided due to the
         * potential loss of information.
         **/
        void make_plural();

        /**
         * Add translation context.
         * If translation already has a context, adds to the existing context.
         **/
        void add_context( const std::string &ctxt );

        /**
         * Deserialize from json. Json format is:
         *     "text"
         * or
         *     { "ctxt": "foo", "str": "bar", "str_pl": "baz" }
         * "ctxt" and "str_pl" are optional. "str_pl" is only valid when an object
         * of this class is constructed with `plural_tag` or `pl_translation()`,
         * or converted using `make_plural()`.
         **/
        void deserialize( JsonIn &jsin );

        /**
         * Returns the stored English string. A number selects the plural form when present.
         **/
        std::string translated( int num = 1 ) const;

        /**
         * Methods exposing the underlying raw strings are not implemented, and
         * probably should not if there's no good reason to do so. Most importantly,
         * the underlying strings should not be re-saved to JSON: doing so risk
         * the original string being changed during development and the saved
         * string will then not be properly translated when loaded back. If you
         * really want to save a translation, translate it early on, store it using
         * `no_translation`, and retrieve it using `translated()` when saving.
         * This ensures consistent behavior before and after saving and loading.
         **/
        std::string untranslated() const = delete;

        /**
         * Whether the underlying string is empty, not matter what the context
         * is or whether translation is needed.
         **/
        bool empty() const;

        /**
         * Compare the stored English strings (singular form).
         **/
        bool translated_lt( const translation &that ) const;
        bool translated_eq( const translation &that ) const;
        bool translated_ne( const translation &that ) const;

        /**
         * Compare translations by their context, raw strings (singular / plural), and no-translation flag
         */
        bool operator==( const translation &that ) const;
        bool operator!=( const translation &that ) const;

        /**
         * Only used for migrating old snippet hashes into snippet ids.
         */
        std::pair<bool, int> legacy_hash() const;

        /**
         * Get raw untranslated string for debug purposes.
         */
        std::string_view debug_get_raw() const {
            return raw;
        }

    private:
        translation( const std::string &ctxt, const std::string &raw );
        translation( const std::string &raw );
        translation( const std::string &raw, const std::string &raw_pl, plural_tag );
        translation( const std::string &ctxt, const std::string &raw, const std::string &raw_pl,
                     plural_tag );
        struct no_translation_tag {};
        translation( const std::string &str, no_translation_tag );

        cata::value_ptr<std::string> ctxt = nullptr;
        std::string raw;
        cata::value_ptr<std::string> raw_pl = nullptr;
        bool needs_translation = false;
        // translation cache. For "plural" translation only latest `num` is optimistically cached
        mutable int cached_num = 0; // `num`, which `cached_translation` corresponds to
        mutable cata::value_ptr<std::string> cached_translation;
};

/**
 * Shorthands for translation::to_translation
 **/
translation to_translation( const std::string &raw );
translation to_translation( const std::string &ctxt, const std::string &raw );
/**
 * Shorthands for translation::pl_translation
 **/
translation pl_translation( const std::string &raw, const std::string &raw_pl );
translation pl_translation( const std::string &ctxt, const std::string &raw,
                            const std::string &raw_pl );
/**
 * Shorthand for translation::no_translation
 **/
translation no_translation( const std::string &str );

/**
 * Stream output and concatenation of translations. Singular forms are used.
 **/
std::ostream &operator<<( std::ostream &out, const translation &t );
std::string operator+( const translation &lhs, const std::string &rhs );
std::string operator+( const std::string &lhs, const translation &rhs );
std::string operator+( const translation &lhs, const translation &rhs );

// Localized comparison operator, intended for sorting strings when they should
// be sorted according to the user's locale.
//
// For convenience, it also sorts pairs recursively, because a common
// requirement is to sort some list of objects by their names, and this can be
// achieved by sorting a list of pairs where the first element of the pair is
// the translated name.
struct localized_comparator {
    template<typename T, typename U>
    bool operator()( const std::pair<T, U> &l, const std::pair<T, U> &r ) const {
        if( ( *this )( l.first, r.first ) ) {
            return true;
        }
        if( ( *this )( r.first, l.first ) ) {
            return false;
        }
        return ( *this )( l.second, r.second );
    }

    template<typename Head, typename... Tail>
    bool operator()( const std::tuple<Head, Tail...> &l,
                     const std::tuple<Head, Tail...> &r ) const {
        if( ( *this )( std::get<0>( l ), std::get<0>( r ) ) ) {
            return true;
        }
        if( ( *this )( std::get<0>( r ), std::get<0>( l ) ) ) {
            return false;
        }
        constexpr std::make_index_sequence<sizeof...( Tail )> Ints{};
        return ( *this )( tie_tail( l, Ints ), tie_tail( r, Ints ) );
    }

    template<typename T>
    bool operator()( const T &l, const T &r ) const {
        return l < r;
    }

    bool operator()( const std::string &, const std::string & ) const;
    bool operator()( const std::wstring &, const std::wstring & ) const;

    template<typename Head, typename... Tail, size_t... Ints>
    auto tie_tail( const std::tuple<Head, Tail...> &t, std::index_sequence<Ints...> ) const {
        return std::tie( std::get < Ints + 1 > ( t )... );
    }
};

constexpr localized_comparator localized_compare{};


