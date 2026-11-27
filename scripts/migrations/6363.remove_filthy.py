#!/usr/bin/env python3
"""Remove legacy FILTHY flags and washing options."""

from run import migrate


def valid_flags(entry):
    flags = entry.get("flags", [])
    return isinstance(flags, list) and all(isinstance(flag, str) for flag in flags)


def remove_flags(entry):
    if "flags" not in entry:
        return entry.copy()
    return {**entry, "flags": [flag for flag in entry["flags"] if flag != "FILTHY"]}


def remove_filthy(entry):
    harvest = entry.get("entries")
    if (
        entry.get("type") == "harvest"
        and isinstance(harvest, list)
        and all(isinstance(item, dict) and valid_flags(item) for item in harvest)
    ):
        return {**entry, "entries": [remove_flags(item) for item in harvest]}
    if not valid_flags(entry):
        return entry
    action = entry.get("use_action")
    if "use_action" in entry and not (
        isinstance(action, (list, dict))
        or action in ("WASH_SOFT_ITEMS", "WASH_HARD_ITEMS")
    ):
        return entry
    updated = remove_flags(entry)
    updated.pop("squeamish_penalty", None)
    if isinstance(action, dict):
        updated["use_action"] = {
            key: value
            for key, value in action.items()
            if key != "filthy_volume_threshold"
        }
    elif isinstance(action, str):
        updated.pop("use_action", None)
    return updated


if __name__ == "__main__":
    migrate(__doc__, remove_filthy, positional=True, multiple=False)
