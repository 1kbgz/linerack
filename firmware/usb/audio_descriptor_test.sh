#!/bin/sh
set -eu

usb_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
source_root="$usb_dir/../../.deps/libDaisy/Middlewares/ST/STM32_USB_Device_Library/Class"
work_dir=$(mktemp -d "${TMPDIR:-/tmp}/linerack-audio-descriptors.XXXXXX")
trap 'rm -rf "$work_dir"' EXIT HUP INT TERM

cp "$source_root/AUDIO/Src/usbd_audio.c" "$work_dir/usbd_audio.c"
cp "$source_root/CompositeBuilder/Src/usbd_composite_builder.c" "$work_dir/usbd_composite_builder.c"
patch "$work_dir/usbd_audio.c" < "$usb_dir/stereo-input-terminal.patch"
patch "$work_dir/usbd_composite_builder.c" < "$usb_dir/stereo-composite-audio.patch"
patch "$work_dir/usbd_audio.c" < "$usb_dir/uac-volume-control.patch"
patch "$work_dir/usbd_composite_builder.c" < "$usb_dir/uac-volume-composite.patch"

grep -q '#define USB_AUDIO_CONFIG_DESC_SIZ 0x6EU' "$work_dir/usbd_audio.c"
grep -q '0x28,.*wTotalLength' "$work_dir/usbd_audio.c"
grep -q '0x02,.*bNrChannels' "$work_dir/usbd_audio.c"
grep -q '0x03,.*wChannelConfig 0x0003' "$work_dir/usbd_audio.c"
grep -q '0x03,.*bmaControls(0): mute and volume' "$work_dir/usbd_audio.c"
grep -q '0x02,.*bSourceID' "$work_dir/usbd_audio.c"
grep -q '0x09,.*Isochronous, adaptive' "$work_dir/usbd_audio.c"
grep -q 'case AUDIO_REQ_GET_MIN:' "$work_dir/usbd_audio.c"
grep -q 'VolumeCtl(percent)' "$work_dir/usbd_audio.c"

grep -q 'wTotalLength = 0x0028U' "$work_dir/usbd_composite_builder.c"
grep -q 'bNrChannels = 0x02U' "$work_dir/usbd_composite_builder.c"
grep -q 'wChannelConfig = 0x0003U' "$work_dir/usbd_composite_builder.c"
grep -q 'pSpFDesc\[6\] = 0x03U' "$work_dir/usbd_composite_builder.c"
grep -q 'bSourceID = 0x02U' "$work_dir/usbd_composite_builder.c"
grep -q 'bmAttributes = 0x09U' "$work_dir/usbd_composite_builder.c"
