#!/usr/bin/env python3
"""Migrate legacy SPEAR flags to STAB."""

from run import migrate


def spear_stab(entry):
    flags = entry.get("flags")
    if not isinstance(flags, list) or not all(isinstance(flag, str) for flag in flags):
        return entry
    if "SPEAR" not in flags or "STAB" in flags:
        return entry
    upgraded = (
        flags + ["STAB"]
        if "REACH_ATTACK" in flags
        else ["STAB" if flag == "SPEAR" else flag for flag in flags]
    )
    return {**entry, "flags": upgraded}


if __name__ == "__main__":
    migrate(__doc__, spear_stab, excluded_types={"mapgen", "palette", "mod_tileset"})
