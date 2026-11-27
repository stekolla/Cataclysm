#!/usr/bin/env python3
"""Migrate legacy battery charge fields to energy units."""

from run import migrate

ACTION_ALIASES = {
    "need_charges": "need_power",
    "transform_charges": "transform_power",
    "target_charges": "target_power",
    "rand_target_charges": "rand_target_power",
}
ITEM_ALIASES = {
    "magazine_well": "battery_well",
    "charges_per_use": "power_draw",
    "initial_charges": "initial_power",
    "count": "initial_power",
    "capacity": "max_power",
    "max_charges": "max_power",
}


def integer(value):
    return type(value) is int or isinstance(value, float) and value.is_integer()


def kj(value):
    return f"{int(value) if integer(value) else value} kJ"


def migrate_action(action):
    if isinstance(action, str):
        return action
    if not isinstance(action, dict) or action.get("type") != "transform":
        raise ValueError("Invalid battery use action")
    updated = {}
    for key, value in action.items():
        if key == "rand_target_charges":
            if (
                not isinstance(value, list)
                or len(value) != 2
                or not all(map(integer, value))
            ):
                raise ValueError("Invalid random charge range")
            value = list(map(kj, value))
        elif key in ACTION_ALIASES:
            if not integer(value):
                raise ValueError("Invalid action charges")
            value = kj(value)
        updated[ACTION_ALIASES.get(key, key)] = value
    return updated


def migrate_item(entry):
    ammo_fields = [entry[key] for key in ("ammo", "ammo_type") if key in entry]
    if not ammo_fields or any(value != "battery" for value in ammo_fields):
        return entry
    if not isinstance(entry.get("type"), str):
        return entry

    updated = {}
    for key, value in entry.items():
        if key in ("ammo", "ammo_type"):
            continue
        if key == "type" and value == "MAGAZINE":
            value = "BATTERY"
        elif key == "magazine_well" and not isinstance(value, str):
            return entry
        elif key == "power_draw" or key in ITEM_ALIASES and key != "magazine_well":
            if not integer(value):
                return entry
            value = kj(value / 1000 if key == "power_draw" else value)
        elif key == "use_action":
            value = migrate_action(value)
        updated[ITEM_ALIASES.get(key, key)] = value

    if "magazines" not in entry:
        return updated
    magazines = entry["magazines"]
    if not isinstance(magazines, list):
        return entry
    batteries, others = [], []
    for pair in magazines:
        if (
            not isinstance(pair, list)
            or len(pair) != 2
            or not isinstance(pair[0], str)
            or not isinstance(pair[1], list)
            or not all(isinstance(x, str) for x in pair[1])
        ):
            return entry
        if pair[0] == "battery":
            batteries.extend(pair[1])
        else:
            others.append(pair)
    updated.update(magazines=others, batteries=batteries)
    return {
        key: value
        for key, value in updated.items()
        if value is not None and value != []
    }


def migrate_energy(entry):
    if integer(entry.get("ups_charges")):
        return {
            ("power_draw" if key == "ups_charges" else key): (
                kj(value) if key == "ups_charges" else value
            )
            for key, value in entry.items()
        }
    try:
        return migrate_item(entry)
    except ValueError:
        return entry


if __name__ == "__main__":
    migrate(__doc__, migrate_energy, positional=True)
