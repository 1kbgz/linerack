# How to set up LineRack development

This guide shows contributors how to prepare a checkout, verify the web app and
native logic, and build Seed3 firmware.

## Prepare the checkout

Install Node.js 22 or newer, pnpm 11.9, GNU Make, Git, and the Arm GNU embedded
toolchain. Clone the repository, then install the web dependencies:

```sh
git clone https://github.com/1kbgz/linerack.git
cd linerack
pnpm install --frozen-lockfile
```

To build or flash firmware, fetch the pinned libDaisy revision and compile it:

```sh
make develop-firmware
```

The target is stored under ignored `.deps/libDaisy`. The Makefile refuses to
change its revision if that checkout contains local changes.

## Verify the checkout

Run the repository lifecycle from the project root:

```sh
make lint
make check
make test
make build
```

`make test` runs browser unit and accessibility tests plus native DSP, USB,
display, persistence, and protocol tests. `make build` produces the static web
app and cross-compiles the composite Seed3 firmware.

The firmware build enforces a 126,976-byte internal-flash limit. Do not raise
that gate to fit another feature. Move the firmware to the supported libDaisy
bootloader/SRAM path before exceeding the budget.

## Run the configurator

Start Vite after installing web dependencies:

```sh
pnpm dev
```

Open the printed URL in desktop Chrome for WebHID access. Other browsers can use
the built-in simulator but cannot configure attached beta hardware.

## Flash a Seed3

Download any setup that must be retained before flashing an incompatible
preset schema. Enter DFU by holding **BOOT** while pressing and releasing
**RESET**, then release **BOOT** and run:

```sh
make -C firmware -f Makefile.usb-composite program-dfu
```

`dfu-util` 0.11 can report `Error during download get_status` after completing
a transfer because the board disconnects while leaving DFU. Confirm success by
checking that LineRack enumerates again and passes audio and WebHID checks.

## Update from the repository template

LineRack records its `python-project-templates/base` revision in
`.copier-answers.yaml`. Pass the non-default answers filename explicitly:

```sh
copier update --trust --answers-file .copier-answers.yaml
```

Review generated changes before accepting them. Preserve LineRack-specific
firmware CI, WebHID dependencies, product tests, and formatting exclusions.
