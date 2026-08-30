# How to test LineRack on iPhone

This guide verifies class-compliant USB audio playback and bus power from a
USB-C iPhone. Mobile configuration is outside the beta scope; configure the
device from desktop Chrome before starting.

## Prepare a known setup

Connect LineRack to a Mac, Windows, or Linux desktop and open the configurator
in Chrome. Apply a setup with two audibly different presets, then verify:

1. LineRack appears as a stereo audio output.
2. Program audio reaches both output channels.
3. A quick physical-button double tap changes the active preset and its sound.
4. The setup survives one USB power cycle.

Start with low listening volume. Current firmware implements USB Audio master
volume and mute after the effect chain. Confirm the OLED briefly shows the host
volume when the iPhone changes it.

## Verify iPhone playback and power

Disconnect LineRack from the desktop and connect it directly to the iPhone with
a USB-C data cable. Do not place an unpowered hub between them.

If iOS asks whether the connected device is **Headphones** or **Other**, choose
**Headphones**. Start playback in a known audio app and confirm:

1. The iPhone routes audio to LineRack instead of its speakers.
2. LineRack remains powered without another supply.
3. Left and right channels reach the matching headphone channels.
4. Audio pitch and speed remain stable for at least five minutes.
5. A quick physical-button double tap changes presets without interrupting playback.
6. The selected preset's effects remain audible after the change.

The browser configurator is not part of this test. iOS Safari does not provide
the WebHID transport used by the beta configurator.

## Check reconnect behavior

While audio is playing, disconnect the USB-C cable, wait five seconds, and
connect it again. Resume playback if iOS paused it. LineRack should enumerate
without entering DFU or requiring a reset.

Reconnect LineRack to the desktop afterward. Confirm USB Audio and WebHID both
enumerate and that stored presets remain intact.

## Troubleshoot missing audio

If LineRack powers up but produces no audio:

1. Confirm iOS still shows LineRack as the active route.
2. Try another known USB-C data cable.
3. Remove hubs and adapters.
4. Reconnect LineRack to the desktop and verify the same preset and output jack.
5. Reflash the last verified composite firmware only after desktop playback
   also fails.

An iOS Simulator cannot verify USB enumeration, bus power, audio routing, or
timing. Use physical iPhone hardware for every result in this guide.
