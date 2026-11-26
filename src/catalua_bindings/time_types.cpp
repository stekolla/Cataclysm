#include "calendar.h"
#include "catalua_bindings.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "rng.h"

#include <chrono>
#include <cstdint>
#include <ctime>

auto cata::detail::reg_time_types(sol::state& lua) -> void {
    DOC("Library for dealing with time primitives.");

    {
        DOC("Represent fixed point in time");
        sol::usertype<time_point> ut =
            luna::new_usertype<time_point>(lua, luna::no_bases, luna::constructors<time_point()>());

        // Constructor method
        luna::set_fx(ut, "from_turn", &time_point::from_turn);

        // Methods
        luna::set_fx(ut, "to_turn", [](const time_point& tp) -> int { return to_turn<int>(tp); });

        luna::set_fx(ut, "is_night", &is_night);
        luna::set_fx(ut, "is_day", &is_day);
        luna::set_fx(ut, "is_dusk", &is_dusk);
        luna::set_fx(ut, "is_dawn", &is_dawn);
        luna::set_fx(ut, "sunrise", [](const time_point& tp) { return sunrise(tp); });
        luna::set_fx(ut, "sunset", [](const time_point& tp) { return sunset(tp); });
        luna::set_fx(ut, "moon_phase", &get_moon_phase);
        luna::set_fx(ut, "season", [](const time_point& tp) {
            return calendar::name_season(season_of_year(tp));
        });


        luna::set_fx(ut, "second_of_minute", [](const time_point& tp) -> int {
            return to_turn<int>(tp) % 60;
        });
        luna::set_fx(ut, "minute_of_hour", [](const time_point& tp) -> int {
            return minute_of_hour<int>(tp);
        });
        luna::set_fx(ut, "hour_of_day", [](const time_point& tp) -> int {
            return hour_of_day<int>(tp);
        });

        // (De-)Serialization
        reg_serde_functions(ut);

        luna::set_fx(ut, "to_string_time_of_day", &to_string_time_of_day);

        // To string
        // We're using Lua meta function here to make it work seamlessly with native Lua tostring()
        luna::set_fx(ut, sol::meta_function::to_string,
                     sol::resolve<std::string(const time_point&)>(to_string));

        // Equality operator
        // It's defined as inline friend function inside point class, we can't access it and so have
        // to improvise
        luna::set_fx(ut, sol::meta_function::equal_to, [](const time_point& a, const time_point& b) {
            return a == b;
        });

        // Less-then operator
        // Same deal as with equality operator
        luna::set_fx(ut, sol::meta_function::less_than,
                     [](const time_point& a, const time_point& b) { return a < b; });

        // Arithmetic operators
        luna::set_fx(
            ut, sol::meta_function::addition,
            [](const time_point& a, const time_duration& b) -> time_point { return a + b; });
        luna::set_fx(
            ut, sol::meta_function::subtraction,
            sol::overload(
                [](const time_point& a, const time_point& b) -> time_duration { return a - b; },
                [](const time_point& a, const time_duration& b) -> time_point { return a - b; }));
    }
    {
        DOC("Represent duration between 2 fixed points in time");
        sol::usertype<time_duration> ut = luna::new_usertype<
            time_duration>(lua, luna::no_bases, luna::constructors<time_duration()>());

        // Constructor methods
        luna::set_fx(ut, "from_turns", [](int t) { return time_duration::from_turns(t); });
        luna::set_fx(ut, "from_seconds", [](int t) { return time_duration::from_seconds(t); });
        luna::set_fx(ut, "from_minutes", [](int t) { return time_duration::from_minutes(t); });
        luna::set_fx(ut, "from_hours", [](int t) { return time_duration::from_hours(t); });
        luna::set_fx(ut, "from_days", [](int t) { return time_duration::from_days(t); });
        luna::set_fx(ut, "from_weeks", [](int t) { return time_duration::from_weeks(t); });

        luna::set_fx(ut, "make_random", [](const time_duration& lo, const time_duration& hi) {
            return rng(lo, hi);
        });

        luna::set_fx(ut, "to_turns", [](const time_duration& t) -> int {
            return to_turns<int>(t);
        });
        luna::set_fx(ut, "to_seconds", [](const time_duration& t) -> int {
            return to_seconds<int>(t);
        });
        luna::set_fx(ut, "to_minutes", [](const time_duration& t) -> int {
            return to_minutes<int>(t);
        });
        luna::set_fx(ut, "to_hours", [](const time_duration& t) -> int {
            return to_hours<int>(t);
        });
        luna::set_fx(ut, "to_days", [](const time_duration& t) -> int { return to_days<int>(t); });
        luna::set_fx(ut, "to_weeks", [](const time_duration& t) -> int {
            return to_weeks<int>(t);
        });

        // (De-)Serialization
        reg_serde_functions(ut);

        // To string
        // We're using Lua meta function here to make it work seamlessly with native Lua tostring()
        luna::set_fx(ut, sol::meta_function::to_string,
                     sol::resolve<std::string(const time_duration&)>(to_string));

        luna::set_fx(ut, sol::meta_function::addition,
                     [](const time_duration& a, const time_duration& b) { return a + b; });
        luna::set_fx(ut, sol::meta_function::subtraction,
                     [](const time_duration& a, const time_duration& b) { return a - b; });
        luna::set_fx(ut, sol::meta_function::multiplication, [](const time_duration& a, int b) {
            return a * b;
        });
        luna::set_fx(ut, sol::meta_function::division, [](const time_duration& a, int b) {
            return a / b;
        });
        luna::set_fx(ut, sol::meta_function::unary_minus, [](const time_duration& a) {
            return -a;
        });
    }
}
