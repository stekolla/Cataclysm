#include "catalua_bindings/item_artifact_descriptions.h"

#include "enum_conversions.h"
#include "translations.h"

#include <ranges>

namespace cata::detail {

auto artifact_effect_names(const std::vector<art_effect_passive>& effects)
    -> std::vector<std::string> {
    return effects | std::views::transform([](const art_effect_passive effect) {
               return io::enum_to_string(effect);
           })
         | std::ranges::to<std::vector>();
}

auto artifact_effect_names(const std::vector<art_effect_active>& effects)
    -> std::vector<std::string> {
    return effects | std::views::transform([](const art_effect_active effect) {
               return io::enum_to_string(effect);
           })
         | std::ranges::to<std::vector>();
}

auto artifact_effect_description(const art_effect_passive effect) -> std::string {
    switch (effect) {
        case AEP_NULL:
            return _("No effect");
        case AEP_STR_UP:
            return _("Strength +4");
        case AEP_DEX_UP:
            return _("Dexterity +4");
        case AEP_PER_UP:
            return _("Perception +4");
        case AEP_INT_UP:
            return _("Intelligence +4");
        case AEP_ALL_UP:
            return _("All stats +2");
        case AEP_SPEED_UP:
            return _("Speed +20");
        case AEP_PBLUE:
            return _("Reduces radiation");
        case AEP_SNAKES:
            return _("Summons friendly snakes when you're hit");
        case AEP_INVISIBLE:
            return _("Makes you invisible");
        case AEP_CLAIRVOYANCE:
            return _("See through walls");
        case AEP_CLAIRVOYANCE_PLUS:
            return _("See through walls farther");
        case AEP_SUPER_CLAIRVOYANCE:
            return _("See through walls at great distance");
        case AEP_STEALTH:
            return _("Your steps are quieted");
        case AEP_EXTINGUISH:
            return _("May extinguish nearby flames");
        case AEP_GLOW:
            return _("Emits light");
        case AEP_PSYSHIELD:
            return _("Protects from fear and paralysis");
        case AEP_RESIST_ELECTRICITY:
            return _("Protection from electricity");
        case AEP_CARRY_MORE:
            return _("Increases carrying capacity");
        case AEP_SAP_LIFE:
            return _("Killing non-zombie monsters may heal you");
        case AEP_FUN:
            return _("Passive morale boost");
        case AEP_SPLIT:
            return _("No effect");
        case AEP_HUNGER:
            return _("Increases hunger");
        case AEP_THIRST:
            return _("Increases thirst");
        case AEP_SMOKE:
            return _("Emits smoke occasionally");
        case AEP_EVIL:
            return _("Adds an evil presence");
        case AEP_SCHIZO:
            return _("Mimics schizophrenia");
        case AEP_RADIOACTIVE:
            return _("Increases your radiation");
        case AEP_MUTAGENIC:
            return _("Mutates you slowly");
        case AEP_ATTENTION:
            return _("Draws netherworld attention");
        case AEP_STR_DOWN:
            return _("Strength -3");
        case AEP_DEX_DOWN:
            return _("Dexterity -3");
        case AEP_PER_DOWN:
            return _("Perception -3");
        case AEP_INT_DOWN:
            return _("Intelligence -3");
        case AEP_ALL_DOWN:
            return _("All stats -2");
        case AEP_SPEED_DOWN:
            return _("Speed -20");
        case AEP_FORCE_TELEPORT:
            return _("Occasionally forces a teleport");
        case AEP_MOVEMENT_NOISE:
            return _("Makes noise when you move");
        case AEP_BAD_WEATHER:
            return _("More likely to experience bad weather");
        case AEP_SICK:
            return _("Decreases health over time");
        case NUM_AEPS:
            break;
    }
    return _("Unknown effect");
}

auto artifact_effect_description(const art_effect_active effect) -> std::string {
    switch (effect) {
        case AEA_NULL:
            return _("No effect");
        case AEA_STORM:
            return _("Emits shock fields");
        case AEA_FIREBALL:
            return _("Launches fireballs");
        case AEA_ADRENALINE:
            return _("Adrenaline rush");
        case AEA_MAP:
            return _("Reveals the area around you");
        case AEA_BLOOD:
            return _("Shoots blood all over");
        case AEA_FATIGUE:
            return _("Creates interdimensional fatigue");
        case AEA_ACIDBALL:
            return _("Launches acid");
        case AEA_PULSE:
            return _("Destroys adjacent terrain");
        case AEA_HEAL:
            return _("Heals minor damage");
        case AEA_CONFUSED:
            return _("Confuses nearby monsters");
        case AEA_ENTRANCE:
            return _("May make nearby monsters friendly");
        case AEA_BUGS:
            return _("May summon friendly insects");
        case AEA_TELEPORT:
            return _("Teleports you");
        case AEA_LIGHT:
            return _("Temporary light source");
        case AEA_GROWTH:
            return _("Rapid plant growth");
        case AEA_HURTALL:
            return _("Hurts all monsters");
        case AEA_FUN:
            return _("Temporary morale bonus");
        case AEA_SPLIT:
            return _("No effect");
        case AEA_RADIATION:
            return _("Spews radioactive gas");
        case AEA_PAIN:
            return _("Increases pain");
        case AEA_MUTATE:
            return _("Chance of mutation");
        case AEA_PARALYZE:
            return _("Paralyzes you");
        case AEA_FIRESTORM:
            return _("Spreads fire around you");
        case AEA_ATTENTION:
            return _("Draws attention from sub-prime denizens");
        case AEA_TELEGLOW:
            return _("Causes teleglow");
        case AEA_NOISE:
            return _("Loud noise");
        case AEA_SCREAM:
            return _("Noise and morale penalty");
        case AEA_DIM:
            return _("Darkens the sky");
        case AEA_FLASH:
            return _("Flashbang");
        case AEA_VOMIT:
            return _("User vomits");
        case AEA_SHADOWS:
            return _("Summons shadow creatures");
        case AEA_STAMINA_EMPTY:
            return _("Empties most of your stamina");
        case NUM_AEAS:
            break;
    }
    return _("Unknown effect");
}

auto artifact_effect_descriptions(const std::vector<art_effect_passive>& effects)
    -> std::vector<std::string> {
    return effects | std::views::transform([](const art_effect_passive effect) {
               return artifact_effect_description(effect);
           })
         | std::ranges::to<std::vector>();
}

auto artifact_effect_descriptions(const std::vector<art_effect_active>& effects)
    -> std::vector<std::string> {
    return effects | std::views::transform([](const art_effect_active effect) {
               return artifact_effect_description(effect);
           })
         | std::ranges::to<std::vector>();
}

auto artifact_charge_description(const art_charge charge) -> std::string {
    switch (charge) {
        case ARTC_NULL:
            return _("No charges");
        case ARTC_TIME:
            return _("Recharges over time");
        case ARTC_SOLAR:
            return _("Recharges in sunlight");
        case ARTC_PAIN:
            return _("Recharges from pain");
        case ARTC_HP:
            return _("Recharges from health loss");
        case ARTC_FATIGUE:
            return _("Recharges from fatigue");
        case ARTC_PORTAL:
            return _("Recharges near portals");
        case NUM_ARTCS:
            break;
    }
    return _("Unknown charge type");
}

auto artifact_charge_req_description(const art_charge_req charge_req) -> std::string {
    switch (charge_req) {
        case ACR_NULL:
            return _("No special requirement");
        case ACR_EQUIP:
            return _("Must be equipped");
        case ACR_SKIN:
            return _("Must be worn against bare skin");
        case ACR_SLEEP:
            return _("Charges while sleeping");
        case ACR_RAD:
            return _("Charges in radiation");
        case ACR_WET:
            return _("Charges while wet");
        case ACR_SKY:
            return _("Charges under open sky");
        case NUM_ACRS:
            break;
    }
    return _("Unknown charge requirement");
}

} // namespace cata::detail
