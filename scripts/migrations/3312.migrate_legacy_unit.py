#!/usr/bin/env python3
"""Convert legacy numeric units in Cataclysm JSON."""

from run import migrate

# Legacy sound propagation losses, in millidecibels.
DISTANCE_LOSS = (
    0,
    1500,
    602,
    352,
    250,
    194,
    158,
    134,
    116,
    102,
    92,
    83,
    76,
    70,
    64,
    60,
    56,
    53,
    50,
    47,
    45,
    42,
    40,
    39,
    37,
    35,
    34,
    33,
    32,
    30,
    29,
    28,
    28,
    27,
    26,
    25,
    24,
    24,
    23,
    23,
    22,
    21,
    21,
    20,
    20,
    20,
    19,
    19,
    18,
    18,
    18,
    17,
    17,
    17,
    16,
    16,
    16,
    15,
    15,
    15,
    15,
    14,
    14,
    14,
    14,
    13,
    13,
    13,
    13,
    13,
    12,
    12,
    12,
    12,
    12,
    12,
    12,
    11,
    11,
    11,
    11,
    11,
    11,
    11,
    10,
    10,
    10,
    10,
    10,
    10,
    10,
    10,
    9,
    9,
    9,
    9,
    9,
    9,
    9,
    9,
    9,
    9,
    9,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    7,
    7,
    7,
    7,
)
VOLUME = [
    "barrel_length",
    "barrel_volume",
    "storage",
    "max_pet_vol",
    "min_pet_vol",
    "contains",
    "volume",
    "integral_volume",
    "magazine_well",
    "max_volume",
    "min_volume",
    "size",
]
WEIGHT = [
    "weight",
    "integral_weight",
    "weight_capacity_bonus",
    "max_weight",
    "trigger_weight",
]
SOUND = {
    "reload_noise_volume",
    "reload_noise_volume_dB",
    "sound_fail_vol",
    "sound_vol",
    "sound_volume",
    "targeting_volume",
    "targeting_volume_dB",
}
ENERGY = ["act_cost", "deact_cost", "react_cost", "trigger_cost", "capacity"]


def from_sound(distance):
    total_distance = max(1, distance + distance // 12)
    last_distance = len(DISTANCE_LOSS) - 1
    loss = sum(DISTANCE_LOSS[2 : min(total_distance, last_distance) + 1])
    loss += max(0, total_distance - last_distance) * DISTANCE_LOSS[last_distance]
    return f"{min(19100, 2000 + loss) // 100} dB"


def from_volume(value):
    volume = value * 250
    return f"{volume // 1000} L" if volume and volume % 1000 == 0 else f"{volume} ml"


def from_weight(value):
    return f"{value // 1000} kg" if value % 1000 == 0 else f"{value} g"


def from_currency(value):
    if value and value % 100_000 == 0:
        return f"{value // 100_000} kUSD"
    if value and value % 100 == 0:
        return f"{value // 100} USD"
    return f"{value} cent"


VOLUME_FIELDS = dict.fromkeys(VOLUME, from_volume)
UNITS = {
    **VOLUME_FIELDS,
    **dict.fromkeys(WEIGHT, from_weight),
    **dict.fromkeys(("price", "price_postapoc"), from_currency),
}


def integer(value):
    return type(value) is int or isinstance(value, float) and value.is_integer()


def convert(value, schema):
    if not isinstance(value, dict):
        raise TypeError("Expected object")
    result = value.copy()
    for key, rule in schema.items():
        if key not in value:
            continue
        field = value[key]
        if isinstance(rule, dict):
            result[key] = convert(field, rule)
        elif integer(field):
            result[key] = rule(int(field))
        elif not isinstance(field, str):
            raise ValueError(f"Invalid legacy unit {key}")
    return result


def apply_schema(entry, schema):
    try:
        return convert(entry, schema)
    except (TypeError, ValueError):
        return entry


def nested_sound(value):
    if isinstance(value, dict):
        return {
            key: from_sound(int(field))
            if key in SOUND and integer(field)
            else nested_sound(field)
            for key, field in value.items()
        }
    if isinstance(value, list):
        return [nested_sound(entry) for entry in value]
    return value


def migrate_units(entry):
    schema = {
        **UNITS,
        "relative": UNITS,
        "armor_data": {"storage": from_volume},
        "workbench": {"volume": from_volume, "mass": from_weight},
    }
    base = apply_schema(entry, schema)
    if isinstance(base.get("use_action"), dict) and base is not entry:
        base["use_action"] = apply_schema(
            base["use_action"],
            {
                "max_volume": from_volume,
                "min_volume": from_volume,
                "max_weight": from_weight,
            },
        )
    if base.get("type") == "vehicle_part":
        base = apply_schema(
            base,
            {
                "folded_volume": from_volume,
                "size": from_volume,
                "relative": VOLUME_FIELDS,
            },
        )
    if base.get("type") == "bionic":
        base = apply_schema(base, dict.fromkeys(ENERGY, lambda value: f"{value} kJ"))
    return nested_sound(base)


if __name__ == "__main__":
    migrate(
        __doc__,
        migrate_units,
        excluded_types={
            "speech",
            "sound_effect",
            "mapgen",
            "palette",
            "faction",
            "mod_tileset",
        },
    )
