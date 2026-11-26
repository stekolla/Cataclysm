#include "catalua_bindings.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "units/sound.h"
#include "units_angle.h"
#include "units_energy.h"
#include "units_mass.h"
#include "units_temperature.h"
#include "units_volume.h"

auto cata::detail::reg_units(sol::state& lua) -> void {
    {
        sol::usertype<units::angle> ut =
            luna::new_usertype<units::angle>(lua, luna::no_bases, luna::no_constructor);

        luna::set_fx(ut, "from_radians", &units::from_radians<double>);
        luna::set_fx(ut, "to_radians", &units::to_radians);
        luna::set_fx(ut, "from_degrees", &units::from_degrees<double>);
        luna::set_fx(ut, "to_degrees", &units::to_degrees);
        luna::set_fx(ut, "from_arcmin", &units::from_arcmin<double>);
        luna::set_fx(ut, "to_arcmin", &units::to_arcmin);

        luna::set_fx(ut, sol::meta_function::equal_to, &units::angle::operator==);
        luna::set_fx(ut, sol::meta_function::less_than, &units::angle::operator<);
        luna::set_fx(ut, sol::meta_function::less_than_or_equal_to, &units::angle::operator<=);
    }
    {
        sol::usertype<units::energy> ut =
            luna::new_usertype<units::energy>(lua, luna::no_bases, luna::no_constructor);

        luna::set_fx(ut, "from_joule", &units::from_joule<int>);
        luna::set_fx(ut, "to_joule", &units::to_joule<int>);
        luna::set_fx(ut, "from_kilojoule", &units::from_kilojoule<int>);
        luna::set_fx(ut, "to_kilojoule", &units::to_kilojoule<int>);

        luna::set_fx(ut, sol::meta_function::equal_to, &units::energy::operator==);
        luna::set_fx(ut, sol::meta_function::less_than, &units::energy::operator<);
        luna::set_fx(ut, sol::meta_function::less_than_or_equal_to, &units::energy::operator<=);
    }
    {
        sol::usertype<units::mass> ut =
            luna::new_usertype<units::mass>(lua, luna::no_bases, luna::no_constructor);

        luna::set_fx(ut, "from_milligram", &units::from_milligram<std::int64_t>);
        luna::set_fx(ut, "to_milligram", &units::to_milligram<std::int64_t>);
        luna::set_fx(ut, "from_gram", &units::from_gram<std::int64_t>);
        luna::set_fx(ut, "to_gram", &units::to_gram<std::int64_t>);
        luna::set_fx(ut, "from_kilogram", &units::from_kilogram<std::int64_t>);
        luna::set_fx(ut, "to_kilogram", &units::to_kilogram<std::int64_t>);
        luna::set_fx(ut, "from_newton", &units::from_newton<std::int64_t>);
        luna::set_fx(ut, "to_newton", &units::to_newton<std::int64_t>);

        luna::set_fx(ut, sol::meta_function::equal_to, &units::mass::operator==);
        luna::set_fx(ut, sol::meta_function::less_than, &units::mass::operator<);
        luna::set_fx(ut, sol::meta_function::less_than_or_equal_to, &units::mass::operator<=);
    }
    {
        sol::usertype<units::volume> ut =
            luna::new_usertype<units::volume>(lua, luna::no_bases, luna::no_constructor);

        luna::set_fx(ut, "from_milliliter", &units::from_milliliter<std::int64_t>);
        luna::set_fx(ut, "from_liter", &units::from_liter<std::int64_t>);
        luna::set_fx(ut, "to_milliliter", &units::to_milliliter<std::int64_t>);
        luna::set_fx(ut, "to_liter", &units::to_liter);

        luna::set_fx(ut, sol::meta_function::equal_to, &units::volume::operator==);
        luna::set_fx(ut, sol::meta_function::less_than, &units::volume::operator<);
        luna::set_fx(ut, sol::meta_function::less_than_or_equal_to, &units::volume::operator<=);
    }
    {
        auto ut = luna::new_usertype<units::sound>(lua, luna::no_bases, luna::no_constructor);

        luna::set_fx(ut, "from_decibel", &units::from_decibel<int>);
        luna::set_fx(ut, "to_decibel", &units::to_decibel<int>);

        luna::set_fx(ut, sol::meta_function::equal_to, &units::sound::operator==);
        luna::set_fx(ut, sol::meta_function::less_than, &units::sound::operator<);
        luna::set_fx(ut, sol::meta_function::less_than_or_equal_to, &units::sound::operator<=);
    }
    {
        auto ut = luna::new_usertype<units::temperature>(lua, luna::no_bases, luna::no_constructor);

        luna::set_fx(ut, "from_celsius", [](const double value) -> units::temperature {
            return units::from_celsius(value);
        });
        luna::set_fx(ut, "to_celsius", [](const units::temperature& value) -> double {
            return units::to_celsius<double>(value);
        });
        luna::set_fx(ut, "from_fahrenheit", [](const double value) -> units::temperature {
            return units::from_fahrenheit(value);
        });
        luna::set_fx(ut, "to_fahrenheit", [](const units::temperature& value) -> double {
            return units::to_fahrenheit<double>(value);
        });
        luna::set_fx(ut, "from_kelvin", [](const double value) -> units::temperature {
            return units::from_millidegree_celsius((value - 273.15) * 1000);
        });
        luna::set_fx(ut, "to_kelvin", [](const units::temperature& value) -> double {
            return units::to_kelvins<double>(value);
        });

        luna::set_fx(ut, sol::meta_function::equal_to, &units::temperature::operator==);
        luna::set_fx(ut, sol::meta_function::less_than, &units::temperature::operator<);
        luna::
            set_fx(ut, sol::meta_function::less_than_or_equal_to, &units::temperature::operator<=);
    }
}
