#include "catalua_bindings/item_parts.h"
#include "catalua_bindings_utils.h"
#include "catalua_luna_doc.h"
#include "explosion.h"

namespace cata::detail {
auto reg_item_explosion_data(sol::state& lua) -> void {
#define UT_CLASS explosion_data
    {
        auto ut = luna::new_usertype<UT_CLASS>(lua, luna::no_bases, luna::no_constructor);

        DOC("Damage dealt by the explosion");
        SET_MEMB_RO(damage);

        DOC("Radius of the explosion");
        SET_MEMB_RO(radius);

        DOC("Whether the explosion creates fire");
        SET_MEMB_RO(fire);

        DOC("Returns the safe range of the explosion");
        SET_FX(safe_range);
    }
#undef UT_CLASS
}

} // namespace cata::detail
