# How to test LineRack on iPhone

Verify class-compliant USB audio playback and bus power on a USB-C iPhone.
Mobile configuration is outside the current scope; configure LineRack from
desktop Chrome first.

## Prerequisites

- A USB-C iPhone
- A known USB-C data cable
- A LineRack prototype that passes desktop USB Audio verification
- Two stored presets with clearly different sound

Start with low listening volume.

## Verify the stored setup

On a desktop, confirm that:

1. LineRack appears as a stereo audio output.
2. Program audio reaches both channels.
3. A quick button double-press changes the active preset and its sound.
4. The setup survives one USB power cycle.

## Test playback and power

Disconnect LineRack from the desktop and connect it directly to the iPhone. Do
not use an unpowered hub. If iOS asks whether the device is **Headphones** or
**Other**, choose **Headphones**.

Play a familiar stereo track and confirm that:

1. iOS routes audio to LineRack instead of its speakers.
2. LineRack remains powered without another supply.
3. Left and right channels are correctly mapped.
4. Pitch and speed remain stable for at least five minutes.
5. A quick button double-press changes presets without interrupting playback.
6. iPhone volume changes output level and briefly appears on the OLED.

iOS Safari does not provide the WebHID transport used by the configurator, so
browser configuration is not part of this test.

## Verify reconnect behavior

While audio is playing, disconnect USB, wait five seconds, and reconnect.
Resume playback if iOS paused it. LineRack should enumerate without a reset or
DFU cycle.

Reconnect LineRack to the desktop afterward. Confirm USB Audio and WebHID both
enumerate and stored presets remain intact.

## Troubleshoot missing audio

1. Confirm iOS still shows LineRack as the active route.
2. Try another known data cable.
3. Remove hubs and adapters.
4. Verify the same preset and output jack on a desktop.
5. Reflash the last verified composite image only if desktop playback also
   fails.

An iOS Simulator cannot test USB enumeration, power, audio routing, or timing.
Use physical hardware for every result in this guide.
