# Vehicle damage scaling factor

The `Vehicle damage scaling factor` world option (`VEHICLE_DAMAGE`) controls how damaged
pre-spawned vehicles are. It ranges from `0.0` to `4.0` in steps of `0.1`, and defaults to `1.0`.

## When it applies

The option only affects vehicles spawned with a non-mint status, i.e. `status` of `-1` (light
damage, the default) or `1` (disabled). See [Spawning vehicles](../../mod/json/reference/vehicles/vehicles_spawning.md).
Vehicles spawned with `status: 0` are always intact.

Each vehicle part is rolled independently, so one vehicle usually has a mix of intact, damaged and
broken parts.

## How the roll works

For every part, the game computes two thresholds from the option value (`chance`) and rolls `4d8`:

```cpp
int broken = 8 * chance;  // truncated toward zero
int unhurt = 20 * chance; // truncated toward zero
int roll = dice( 4, 8 );  // 4..32, bell-shaped, mean 18
```

| Condition                | Resulting part HP                                      |
| ------------------------ | ------------------------------------------------------ |
| `roll >= unhurt`         | 100% (intact)                                          |
| `broken < roll < unhurt` | `(roll - broken) / (unhurt - broken)` of durability    |
| `roll <= broken`         | 0% (broken); batteries and fuel tanks are also emptied |

Things to note:

- Raising the option moves both thresholds up, so more of the bell curve falls into the damaged
  and broken ranges.
- A damaged part never keeps 100% HP. Its highest HP is `1 - 1 / (unhurt - broken)`, so HP jumps to
  100% at `roll == unhurt`.
- Values below `0.25` leave `unhurt` at 4 or less. Since the lowest possible roll is 4, no damage
  happens at all.
- At `4.0`, `broken` is 32, which is the highest possible roll. Every part of a damaged vehicle is
  broken.

## Reference values

| Factor | `broken` / `unhurt` | Broken | Damaged | Intact | Expected HP |
| ------ | ------------------- | ------ | ------- | ------ | ----------- |
| 0.0    | 0 / 0               | 0%     | 0%      | 100%   | 100%        |
| 0.5    | 4 / 10              | 0.0%   | 3.1%    | 96.9%  | 99.0%       |
| 0.8    | 6 / 16              | 0.4%   | 29.5%   | 70.1%  | 89.9%       |
| 1.0    | 8 / 20              | 1.7%   | 60.7%   | 37.6%  | 75.0%       |
| 1.5    | 12 / 30             | 12.0%  | 87.6%   | 0.4%   | 34.4%       |
| 2.0    | 16 / 40             | 37.6%  | 62.4%   | 0%     | 12.5%       |
| 3.0    | 24 / 60             | 91.9%  | 8.1%    | 0%     | 0.5%        |
| 4.0    | 32 / 80             | 100%   | 0%      | 0%     | 0%          |

## Older worlds

Before the range was reduced to `0.0`-`4.0`, the option accepted values up to `10.0`. Anything above
`4.0` already broke every part of a damaged vehicle. A world saved with a value outside the new
range loads with the default value `1.0` instead.
