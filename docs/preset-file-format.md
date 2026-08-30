# LineRack preset file format

LineRack setup and shared-preset files are UTF-8 JSON documents. Schema version
1 uses two root formats:

| Content | Format identifier | Conventional suffix |
| --- | --- | --- |
| Complete device setup | `linerack-presets` | `.linerack.json` |
| One shared preset | `linerack-preset` | `.linerack-preset.json` |

## Root object

| Field | Type | Required | Value |
| --- | --- | --- | --- |
| `format` | string | yes | `linerack-presets` |
| `schemaVersion` | integer | yes | `1` |
| `engine` | object | yes | Audio engine metadata |
| `display` | object | accepted as optional | Device-wide display settings |
| `routing` | object | accepted as optional | Device-wide source routing |
| `slots` | array | yes | One or more preset slots |

Missing `display` and `routing` objects are accepted for compatibility with
older version 1 files. Normalization supplies the current defaults. Exported
files always contain both objects.

## Engine object

| Field | Type | Constraint |
| --- | --- | --- |
| `sampleRate` | integer | Positive; current hardware requires `48000` |
| `channels` | integer | Positive; current hardware requires `2` |

## Display object

| Field | Type | Constraint | Default |
| --- | --- | --- | --- |
| `defaultMode` | string | `preset`, `eq-response`, or `visualizer` | `preset` |
| `blankingEnabled` | boolean | `true` or `false` | `true` |

`defaultMode` is the display shown after temporary preset-change feedback has
expired. Current firmware blanks the OLED after 20 seconds when
`blankingEnabled` is `true`.

## Routing object

| Field | Type | Constraint | Default |
| --- | --- | --- | --- |
| `sourceMode` | string | `usb`, `analog`, or `mix` | `usb` |
| `usbTrimDb` | number | -24 through 0 in 0.5 dB steps | `0` |
| `analogTrimDb` | number | -24 through 0 in 0.5 dB steps | `0` |

Mixed routing requires both trims to be -6.5 dB or lower. This preserves
headroom when correlated full-scale USB and analog sources are summed.

## Preset slot object

| Field | Type | Constraint |
| --- | --- | --- |
| `number` | integer | Positive and unique |
| `name` | string | Non-empty |
| `plugins` | array | Ordered plugin instances |

Current hardware requires exactly four slots numbered 1 through 4. Names may
contain ASCII letters, numbers, spaces, period, question mark, and hyphen. The
encoded name length is limited to 12 UTF-8 bytes.

## Plugin instance object

| Field | Type | Constraint |
| --- | --- | --- |
| `id` | string | Non-empty instance identity; normally a UUID |
| `pluginId` | string | ID advertised by the target device |
| `pluginVersion` | integer | Positive and supported by the target device |
| `enabled` | boolean | Processing bypass state |
| `parameters` | object | Parameter IDs mapped to scalar values |

Parameter values may be numbers, booleans, or strings at the file-validation
boundary. Device compatibility requires every advertised numeric parameter,
rejects unknown parameter IDs, and enforces the advertised range.

Current hardware allows ten plugin instances per slot. One `gain` instance is
required at index 0 and one `limiter` instance is required at the final index.
The eight positions between them contain repeatable or reordered effects.

## Shared preset object

A shared-preset document carries one named chain without a slot number or
device-wide display and routing settings.

| Field | Type | Required | Value |
| --- | --- | --- | --- |
| `format` | string | yes | `linerack-preset` |
| `schemaVersion` | integer | yes | `1` |
| `engine` | object | yes | Same shape as a complete setup |
| `preset` | object | yes | `name` and `plugins` from a preset slot |

The configurator assigns an imported shared preset to the selected slot. Device
compatibility checks still apply to its engine, chain, and parameters.

```json
{
  "format": "linerack-preset",
  "schemaVersion": 1,
  "engine": { "sampleRate": 48000, "channels": 2 },
  "preset": {
    "name": "Everyday",
    "plugins": [
      {
        "id": "1e059750-95ee-4d3f-9ef7-e6257ce78cd5",
        "pluginId": "gain",
        "pluginVersion": 1,
        "enabled": true,
        "parameters": { "gainDb": 0 }
      },
      {
        "id": "0d2af531-f075-4d23-9d36-941f95f5ea3a",
        "pluginId": "limiter",
        "pluginVersion": 1,
        "enabled": true,
        "parameters": { "ceilingDb": -1 }
      }
    ]
  }
}
```

## Complete setup example

```json
{
  "format": "linerack-presets",
  "schemaVersion": 1,
  "engine": { "sampleRate": 48000, "channels": 2 },
  "display": { "defaultMode": "eq-response", "blankingEnabled": true },
  "routing": { "sourceMode": "usb", "usbTrimDb": 0, "analogTrimDb": 0 },
  "slots": [
    {
      "number": 1,
      "name": "Everyday",
      "plugins": [
        {
          "id": "1e059750-95ee-4d3f-9ef7-e6257ce78cd5",
          "pluginId": "gain",
          "pluginVersion": 1,
          "enabled": true,
          "parameters": { "gainDb": 0 }
        },
        {
          "id": "0d2af531-f075-4d23-9d36-941f95f5ea3a",
          "pluginId": "limiter",
          "pluginVersion": 1,
          "enabled": true,
          "parameters": { "ceilingDb": -1 }
        }
      ]
    }
  ]
}
```

The abbreviated example is structurally valid but incompatible with current
hardware because current hardware requires all four preset slots.

## Compatibility boundary

JSON validation establishes document shape. Device compatibility additionally
checks slot count, engine format, routing support, display features, plugin
catalog, plugin versions, endpoint positions, parameter ranges, and chain
length. The connected device's advertised capabilities are authoritative.
