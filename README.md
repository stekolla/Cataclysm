# Cataclysm

![Cataclysm screenshots](docs/en/contribute/img/readme-title.png)

Cataclysm is a roguelike with sci-fi elements set in a post-apocalyptic world.

While some have described it as a "zombie game", there is far more to Cataclysm than that. Struggle
to survive in a harsh, persistent, procedurally generated world. Scavenge the remnants of a dead
civilization for food, equipment, or, if you are lucky, a vehicle with a full tank of gas to get you
the hell out of there.

Fight to defeat or escape from a wide variety of powerful monstrosities, from zombies to giant
insects to killer robots and things far stranger and deadlier, and against the others like yourself,
who want what you have.

Find a way to stop the Cataclysm ... or become one of its strongest monsters.

Cataclysm is an independent project derived from Cataclysm: Bright Nights, which in turn descends
from Cataclysm: Dark Days Ahead. It is no longer part of or maintained by the Bright Nights project.

## Browser game

Cataclysm runs in a desktop browser with WebAssembly JavaScript Promise Integration (JSPI).
See the [browser build guide](docs/en/dev/guides/building/cmake.md) to build and run it locally.

## Contributing

> Cataclysm and its inherited content are developed under the Creative Commons Attribution-ShareAlike 3.0 license. The
> code and content of the game is free to use, modify, and redistribute for any purpose whatsoever.
> See http://creativecommons.org/licenses/by-sa/3.0/ for details. Some code distributed with the
> project is not part of the project and is released under different software licenses, the files
> covered by different software licenses have their own license notices.

Please read the [contributor guide](docs/en/contribute/contributing.md) for details.

## Documentation

Gameplay and development documentation is available as Markdown in the
[docs directory](docs/README.md).

## Community

[![Documentation](https://img.shields.io/badge/Docs-Markdown-lightgrey?style=for-the-badge)][docs]
[![Discussions](https://img.shields.io/badge/Discussions-black?style=for-the-badge&logo=github)][discussion]

[discussion]: https://github.com/stekolla/Cataclysm-BN/discussions
[docs]: docs/README.md "Project documentation"

## Frequently Asked Questions

#### Is there a tutorial?

Yes, you can find the tutorial in the **Special** menu at the main menu (be aware that due to many
code changes the tutorial may not function). You can also access documentation in-game via the `?`
key.

#### How can I change the key bindings?

Press the `?` key, followed by the `1` key to see the full list of key commands. Press the `+` key
to add a key binding, select which action with the corresponding letter key `a-w`, and then the key
you wish to assign to that action.

#### How can I start a new world?

**World** on the main menu will generate a fresh world for you. Select **Create World**.

#### Why is there no music or sound?

Sound is disabled in the browser build.

#### I've found a bug. What should I do?

[Bug report](https://github.com/stekolla/Cataclysm-BN/issues/new) can
be submitted via debug menu.

Run `Submit a bug report on github` inside the game to submit an issue.

|   1. open Options (ESC) -> Debug Menu (a)   |              2. open Info (i)               |
| :-----------------------------------------: | :-----------------------------------------: |
| ![](docs/en/contribute/img/readme-bug1.png) | ![](docs/en/contribute/img/readme-bug2.png) |
|    3. Submit a bug report on github (U)     |      4. An link to issue is generated       |
| ![](docs/en/contribute/img/readme-bug3.png) | ![](docs/en/contribute/img/readme-bug4.png) |

It will open a bug report on browser with `Version and configuration` filled in.

#### I would like to make a suggestion. What should I do?

- For simple ideas: please visit
  [the project Discussions page](https://github.com/stekolla/Cataclysm-BN/discussions/categories/ideas).
  It could be a new feature, a port request, a mod idea, or anything else.
- Please submit an issue on
  [the project GitHub page](https://github.com/stekolla/Cataclysm-BN/issues/) using
  [the feature request form](https://github.com/stekolla/Cataclysm-BN/issues/new).
