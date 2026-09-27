![Besprited](docs/branding.png)

**A free and open-source pixel-art editor and sprite animation tool for Windows, macOS and Linux.**

[**Download**](https://besprited.veritaware.com/download.html) ·
[Website](https://besprited.veritaware.com/) ·
[Installation guide](INSTALL.md) ·
[Release notes](https://github.com/veritaware/Besprited/releases) ·
[Contributing](CONTRIBUTING.md)

[![Latest release](https://img.shields.io/github/v/release/veritaware/Besprited)](https://github.com/veritaware/Besprited/releases/latest)
[![License: GPL v2](https://img.shields.io/badge/license-GPLv2-blue)](LICENSE.txt)
![Platforms: Windows, macOS, Linux](https://img.shields.io/badge/platforms-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey)

[![Discord](docs/btn_discord.png)](https://discord.gg/hTp5gPUUJT)
[![Ko-Fi](docs/btn_ko-fi.png)](https://ko-fi.com/veritaware)

![Besprited Screenshot](docs/screenshot.png)

## Introduction
Besprited is a free and open source program for creating and animating 2D sprites. It features:
* Layers and frames, with real-time animation preview, onion skinning, frame tags and per-frame durations.
* Pixel-precise tools: a stabilized pencil for clean lines, shading ink, custom brushes, filled contour,
  polygon and symmetry.
* RotSprite rotation, layer blend modes, and a tiling mode for drawing seamless patterns and textures.
* Ready-to-use palettes, or make your own.
* Multiple sprites can be edited at once.
* Opens and saves Aseprite `.ase`/`.aseprite` files (some newer Aseprite features aren't supported yet),
  plus GIF, PNG sequences, sprite sheets with JSON data, WebP, QOI, BMP, TGA, PCX, ICO and FLI/FLC.
* JavaScript [scripting](SCRIPTING.md) and [themes](#theming) with custom fonts.
* GPG-signed releases for Windows, macOS (Intel and Apple Silicon) and Linux (AppImage, `.deb`, `.rpm`, `.tar.gz`).

`Note:` this project makes use of AI tooling to some degree. You can read more about it in
[AI_USAGE.md](AI_USAGE.md)

Before creating issues or commenting on already open issues/PRs read our
[Code of Conduct](CODE_OF_CONDUCT.md) first.

## Download and install
Get the [latest release](https://github.com/Veritaware/Besprited/releases/latest) for Linux, macOS, or Windows
(or use the [download page](https://besprited.veritaware.com/download.html)).
Then follow the [Installation guide](INSTALL.md).

## How is it different from LibreSprite?
Besprited is a fork of [LibreSprite](https://github.com/LibreSprite/LibreSprite) with a faster, more experimental
development cycle and a quarterly release schedule. It regularly merges LibreSprite's changes, and fixes that fit
LibreSprite are offered back upstream. On top of that it adds its own features and fixes: for example
selection fill/stroke commands, zoom presets, a custom checkerboard size, timeline layer buttons, larger brushes,
extended symmetry options, custom theme fonts and a security review of the file-format code. See the
[release notes](https://github.com/veritaware/Besprited/releases) for the full list, and the
[roadmap](https://besprited.veritaware.com/roadmap.html) for what's planned.

## History
It all started with [Aseprite](https://www.aseprite.org), developed by [David Capello](https://github.com/dacap).
Aseprite used to be distributed under the GNU General Public License version 2,
but was moved to a proprietary license on [August 26th, 2016](https://github.com/aseprite/aseprite/commit/5ecc356a41c8e29977f8608d8826489d24f5fa6c).

As a response to that, [LibreSprite](https://github.com/LibreSprite/LibreSprite) was created as a fork of the last
GPLv2-licensed version of Aseprite, to continue its development as a free and open source project.
The work has been branched-out from the [last Aseprite commit](https://github.com/aseprite/aseprite/commit/03be4aa23db465219962f4c62410f628e7392545) covered by the GPL version 2 license,
and is now developed independently of Aseprite.

In 2026 [Nidrax](https://github.com/Nidrax) forked the project under the [Veritaware](https://github.com/veritaware)
organization to create his own opinionated version of the editor independently of LibreSprite's team design decisions, allowing himself a faster, more experimental development cycle,
and thus Besprited was born.

### And why *Besprited*?
Because this project is BS. What other reason did you expect?

## Contributing
Feel free to contribute to the project! Check out the [contributing guidelines](CONTRIBUTING.md) to get started.

New here? Look for issues labelled
[`good first issue`](https://github.com/veritaware/Besprited/issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22)
or [`help wanted`](https://github.com/veritaware/Besprited/issues?q=is%3Aissue+is%3Aopen+label%3A%22help+wanted%22),
and say hi on [Discord](https://discord.gg/hTp5gPUUJT) if you have questions.
Besprited takes part in [Hacktoberfest](https://hacktoberfest.com/); see the
[Hacktoberfest section](CONTRIBUTING.md#hacktoberfest) of the contributing guidelines.

## Compiling and building
Follow the [Building the source](BUILDING.md) guide.

## Theming
The app is compatible with LibreSprite themes you can find [here](https://libresprite.github.io/#!/resources).

## Scripting
Scripting is powered by [QuickJS-ng](https://github.com/quickjs-ng/quickjs), a small, portable, dependency-free JS engine vendored as a submodule, so it always builds the same way on every platform.

For API guide check the [Scripting guide](SCRIPTING.md).

## License
This program is distributed under the [GNU General Public License Version 2](LICENSE.txt).

## Credits
An ***enormous*** thank you to the original developers of [Aseprite](https://www.aseprite.org),
without them and their original licensing this project wouldn’t exist.
And also huge thanks to all the [LibreSprite contributors](https://github.com/LibreSprite/LibreSprite/blob/master/CONTRIBUTORS.md) who have worked hard to create a base for this project to build upon.
