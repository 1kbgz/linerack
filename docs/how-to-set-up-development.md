# How to set up LineRack development

Prepare a checkout for web development, native tests, and Seed3 firmware builds.

## Prerequisites

- Node.js 22 or newer
- pnpm 11.9
- GNU Make and Git
- Arm GNU embedded toolchain and `dfu-util` for firmware work

On macOS, install the firmware tools with Homebrew:

```sh
brew install --cask gcc-arm-embedded
brew install dfu-util
```

## Install dependencies

Clone the repository and install web dependencies:

```sh
git clone https://github.com/1kbgz/linerack.git
cd linerack
pnpm install --frozen-lockfile
```

Fetch the pinned libDaisy revision and build it:

```sh
make develop-firmware
```

The dependency is stored in the ignored `.deps/libDaisy` directory. The
Makefile will not change its revision when that checkout contains local changes.

## Verify the checkout

Run the complete repository lifecycle:

```sh
make lint
make check
make test
make build
```

`make test` runs browser, accessibility, native DSP, USB, display, persistence,
and protocol tests. `make build` creates the static site in `dist` and builds
the composite Seed3 firmware in `firmware/build`.

The firmware build enforces a 126,976-byte internal-flash limit. Do not raise
the limit to make a build pass; move the firmware to the supported
libDaisy bootloader/SRAM path first.

## Run the configurator

Start the development server:

```sh
pnpm dev
```

Open the printed origin at `/configure` in desktop Chrome to use WebHID with
attached hardware. Other browsers can use the simulator.

## Flash the composite firmware

Download any setup that must survive a preset-schema change. Enter DFU by
holding **BOOT**, pressing and releasing **RESET**, then releasing **BOOT**.
Flash the current integrated image:

```sh
make -C firmware -f Makefile.usb-composite program-dfu
```

`dfu-util` 0.11 can report `Error during download get_status` after a successful
transfer because Seed3 disconnects while leaving DFU. Confirm success by
checking that USB Audio and WebHID enumerate again.

## Related documentation

- [Build and verify the Seed3 prototype](prototype-bring-up.md)
- [Update from the repository template](how-to-update-template.md)
- [Firmware build targets](firmware-build-targets.md)
- [Verify USB Audio on Seed3](usb-audio-bring-up.md)
