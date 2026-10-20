<div align="center">

# ONE MUST FALL 2097

### The 1994 robot fighting classic, running natively in your browser

**English** · [Italiano](README.it.md)

![OpenOMF WebAssembly](https://img.shields.io/badge/OpenOMF-WebAssembly-0b7285?style=for-the-badge)
![Emscripten SDL2](https://img.shields.io/badge/Emscripten-SDL2-1f6feb?style=for-the-badge)
![WebGL2](https://img.shields.io/badge/WebGL2-GLES3_shim-2e7d32?style=for-the-badge)
![Touch controls](https://img.shields.io/badge/touch-pad-PUNCH-orange?style=for-the-badge)
![Released 1994](https://img.shields.io/badge/released-1994-c62828?style=for-the-badge)

[![▶ PLAY ONLINE](https://img.shields.io/badge/%E2%96%B6_PLAY_ONLINE-mich--de.github.io%2Fopenomf2097-ff6d2e?style=for-the-badge)](https://mich-de.github.io/openomf2097/)

**▶ [Play it now in your browser → mich-de.github.io/openomf2097](https://mich-de.github.io/openomf2097/)**

<img src="screenshots/GAMEPLAY_native_fight.png" alt="Crystal Jaguar and Cossette Flail fight in a native WebAssembly build of OpenOMF" width="820">

*Native WebAssembly build (OpenOMF): no emulator, WebGL2, the game's own 320x200 frame.*

</div>

---

## Play online

No install needed: open **https://mich-de.github.io/openomf2097/** and press **PLAY (WASM, NO EMULATOR)**. The page fetches the engine and the game data (~19 MB the first time, then cached by the browser with a new cache entry after every deploy) and boots straight into the game. No DOS, no emulator: this is [OpenOMF](https://github.com/omf2097/openomf), the open reimplementation, compiled to WebAssembly.

- **Keyboard:** player 1 uses the arrows, `Enter` to punch, `Right Shift` to kick. Player 2 plays on `WASD` + `QEZC`, `Left Shift` to punch, `Left Ctrl` to kick. `Esc` opens the menu, `Enter` confirms. `F11` or **FULL SCREEN** for full screen.
- **Phone and tablet:** an on-screen pad appears in the corners — cross pad bottom left, **PUNCH** and **KICK** bottom right, **ESC** top right. The pad sends the same keys as the keyboard, so the game cannot tell the difference.
- **Single player and local two-player** are both supported; online play is not wired up (the menu's NETWORK PLAY needs a server this site doesn't run).
- **Company networks:** web filters such as Zscaler may cut the download (the data pack looks like an archive of DOS programs to them). The page says so and offers to try again; the fix is another network, or asking IT to allow `mich-de.github.io`.

One Must Fall 2097 has been freeware since 1999. This repo is also the site: GitHub Pages serves it straight from the root.

## Gameplay

| | |
|:---:|:---:|
| <img src="screenshots/GAMEPLAY_native_menu.png" width="400"><br>Main menu, native build | <img src="screenshots/GAMEPLAY_pilots.png" width="400"><br>Ten pilots to choose from |
| <img src="screenshots/GAMEPLAY_versus.png" width="400"><br>Trash talk before the fight | <img src="screenshots/GAMEPLAY_native_fight.png" width="400"><br>Live fight with HUD and health bars |
| <img src="screenshots/GAMEPLAY_touch_menu.png" width="400"><br>Phone: menu with the on-screen pad | <img src="screenshots/GAMEPLAY_touch_splash.png" width="400"><br>Phone: splash, controls in the corners |

## Quick start

```bash
git clone https://github.com/mich-de/openomf2097.git
cd openomf2097
python3 -m http.server 8734
```

Open **http://127.0.0.1:8734/** and press **PLAY (WASM, NO EMULATOR)**. Add `?touch=1` to the address to try the mobile controls on a computer (`?touch=0` hides them).

## Highlights

- **Native, no emulator.** The game itself — [OpenOMF](https://github.com/omf2097/openomf) in C with SDL2 — compiled to WebAssembly and rendered on WebGL2. Menus, fights and music run in the page: no DOS, no js-dos, no emulated CPU.
- **One page for desktop and mobile.** Touch screens get an on-screen pad and buttons; keyboards just work.
- **A pad made for fighting.** It synthesises the keyboard's own DOM keys, so the engine sees exactly what a player 1 would press. A diagonal holds two arrows, as on a keyboard, so rolling the thumb from down to down-back only adds back and never passes through neutral mid-move. The down diagonals (crouch block, quarter circles) get wider sectors, and each direction holds until the thumb is 10° past its edge, so a wobbling thumb doesn't flicker between two directions.
- **A launcher made of the game's own art.** Background, lettering and panels come from the 1994 NetArena screens shipped with the game; the touch buttons are drawn pixel by pixel with the same bitmap font.
- **No stale engine, ever.** The page pins `openomf.js`, `openomf.wasm` and `openomf.data` by ETag, so every deploy is a new cache entry and returning players never keep an old engine.

## The native WebAssembly build

`index.html` runs the game as WebAssembly instead of emulating the machine it was written for:

- **The engine.** [OpenOMF](https://github.com/omf2097/openomf) (C, SDL2) pinned at commit `4fd5be9`, compiled with Emscripten. SDL2, SDL2_mixer, libpng and zlib come from the Emscripten ports; enet, libxmp and libconfuse are built as static WebAssembly libraries; libepoxy is replaced by a small shim that serves `epoxy/gl.h` as GLES3, which is what WebGL2 speaks.
- **Shaders.** The GLSL 3.00 shaders are translated for GL ES at build time (in `tools/web/build_openomf.sh`): `highp usampler2D`, no redeclared `gl_FragCoord`, no `int`/`float` mixing, `textureLod` bias as a float.
- **Colour.** WebGL2 cannot render to `RGBA16`, so the palette-index render target is `RGBA16F`; the half-float mantissa keeps every index exact because the decode quantizes with `int(v * 1023.0 + 0.5)`.
- **Timing.** GitHub Pages serves no `COOP`/`COEP` headers, so threads are out and `SDL_Delay` would busy-wait. The build uses ASYNCIFY with a blocking main loop: each frame ends in `emscripten_sleep`, paced to 60 Hz against `emscripten_get_now`.
- **The console.** The in-game console reads stdin with `read(0, ...)`, which never returns in a browser and froze the tab; it is skipped under `__EMSCRIPTEN__`. The console log goes to `window.__omflog` instead (`omf-debug.html` shows it alone).
- **Language files.** `ENGLISH.DAT2`/`GERMAN.DAT2` cannot be generated by the wasm `languagetool` (it runs at build time), so a host-built copy of the tool produces them and they ship in the data pack; without them `lang_init()` aborts.
- **Files.** Game data is packed into `openomf.data`; scores and config live in browser storage. The first load pulls `openomf.js` (239 KB), `openomf.wasm` (~3.1 MB) and `openomf.data` (~15.4 MB), each pinned by ETag so a deploy never leaves a stale copy behind.
- **Patch.** Every source change is a patch in `tools/web/` applied by the build script against the pinned checkout, so the build stays reproducible.

```bash
source ~/emsdk/emsdk_env.sh
tools/web/build_deps.sh      # once: enet, libxmp, libconfuse
tools/web/build_openomf.sh   # clones/pins OpenOMF, patches, builds, copies into wasm/
# OMF_EM_DEBUG=1 tools/web/build_openomf.sh   # assertions and logging build
```

## Touch controls, and how they reach the engine

The engine reads SDL2 keyboard events from the DOM, so the pad does not invent a new protocol: it synthesises the same `KeyboardEvent`s a keyboard would send, with the legacy `keyCode` SDL still reads, and dispatches them on the game canvas. A press is one `keydown`, a release one `keyup`; the held set makes a direction change press the new arrow before releasing the old one, exactly like two keys overlapping on a keyboard.

| Button | Sends |
|---|---|
| Cross pad | Arrow keys (diagonals hold two) |
| **PUNCH** | `Enter` |
| **KICK** | `Right Shift` (`code: "ShiftRight"`) |
| **ESC** | `Esc` |

The controls appear on touch screens automatically; `?touch=1` forces them on and `?touch=0` off. All keys are released on window blur and on `visibilitychange`, so a stray thumb can't leave the game walking.

## Built with

| What | Used for |
|---|---|
| [OpenOMF](https://github.com/omf2097/openomf), pinned at `4fd5be9` | The game engine, compiled to WebAssembly: menus, fights, music, saves |
| Emscripten (SDL2, SDL2_mixer, libpng, zlib ports) | The toolchain and its system libraries |
| enet, libxmp, libconfuse, built as static WebAssembly libraries | Networking sources, module music (`.PSM`), config files |
| WebGL2 | The renderer, via a GLES3 shim for libepoxy |
| GitHub Pages | Hosting, straight from the root of this repo |
| [REA](https://github.com/morluto/rea) + Chrome DevTools Protocol | Headless browser tests: keys, touches, screenshots, network logs |
| [fontTools](https://github.com/fonttools/fonttools) | Turning the game's bitmap fonts into TTF and WOFF2 |

## The game's fonts, as real font files

<img src="assets/fonts/specimen.png" alt="The three OMF 2097 pixel fonts rendered in Chrome" width="820">

The game's bitmap fonts, converted to TTF and WOFF2 in [`assets/fonts/`](assets/fonts/). Each pixel is a square outline, so they stay crisp at their cell size and whole multiples of it.

| Font | From | Cell | Characters |
|---|---|---|---|
| **OMF 2097 NetArena** | `NETFONT1.PCX`, the NetArena lettering of the launcher | 9 px, proportional | ASCII |
| **OMF 2097 Big** | `GRAPHCHR.DAT`, the game's menu and HUD font | 8x8 px | ASCII + code page 437 letters and symbols |
| **OMF 2097 Small** | `CHARSMAL.DAT`, the game's small font | 6x6 px | ASCII (capitals only) + code page 437 letters |

```css
@font-face { font-family: "OMF 2097 Big"; src: url(assets/fonts/omf2097-big.woff2) format("woff2"); }
h1 { font: 32px "OMF 2097 Big"; } /* 4 x 8 px: every pixel lands on whole screen pixels */
```

Rebuild them with `pip install fonttools brotli`, then `python3 tools/build_fonts.py`.

## How it was tested

- **The native build, screenshot by screenshot.** `cdp_shot.js` boots the page in headless Chrome, clicks **PLAY**, holds each key long enough for the polled menu (`SDL_GetKeyboardState` reads the state, so a tapped key is missed) and captures splash → main menu → character select → live fight with HUD, health bars and projectiles. Headless WebGL needs `--use-gl=angle --use-angle=swiftshader --enable-unsafe-swiftshader`.
- **The pad, with real touches.** The Chrome DevTools Protocol sends actual `touchStart`/`touchEnd` points at the pad's arms and buttons: holding the right arm and pressing PUNCH walks the game from splash to main menu to the robot selection screen, and the ESC button quits cleanly (`GAME CLOSED (code 0)`).
- **An opponent that stands still.** Moves and diagonals were checked in a TWO PLAYER GAME, where player 2 never touches a key.
- **Menus that skip.** The game's menus sometimes take one key press twice, so scripts read the highlighted line from the frame (the brightest one) and correct the cursor before confirming.

## What's in the repo

| Path | What it is |
|---|---|
| `index.html` | The game page: PLAY button, launcher, keyboard and touch controls |
| `netfont.js` | The `NETFONT1.PCX` bitmap font, drawn pixel by pixel: launcher lettering and touch buttons |
| `wasm/` | The native build served by the page: `openomf.js`, `openomf.wasm`, `openomf.data` |
| `tools/web/` | Web build script, guarded patches (GL probe, RGBA16F, stdin, frame pacing, opus), GLES3 shim, stage probe |
| `omf-debug.html` | Minimal log-only harness for the native build (`window.__omflog`) |
| `assets/` | Launcher art taken from the game (`NETMEGA.PCX` background, logo) |
| `assets/fonts/` | The game's three bitmap fonts as TTF and WOFF2, plus a specimen |
| `cdp_shot.js` | Drives Chrome over CDP: load, wait, click, screenshot, evaluate |
| `screenshots/` | Gameplay shots and the development screenshots shown above |

## Tools

```bash
# Rebuild the native WebAssembly build (once: enet, libxmp, libconfuse)
source ~/emsdk/emsdk_env.sh
tools/web/build_deps.sh
tools/web/build_openomf.sh        # OMF_EM_DEBUG=1 to build with assertions

# Screenshot the page in real Chrome
google-chrome-stable --headless=new --remote-debugging-port=9333 --remote-allow-origins='*' --window-size=1280,720 about:blank &
node cdp_shot.js "http://127.0.0.1:8734/" 20000 out.png [x,y] [waitAfterClickMs]
```
