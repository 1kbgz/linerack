import analogInput from "../docs/analog-input-direction.md?raw";
import deployment from "../docs/deployment-reference.md?raw";
import dspAndDisplay from "../docs/dsp-and-display-direction.md?raw";
import hidProtocol from "../docs/hid-protocol.md?raw";
import enclosurePrinting from "../docs/how-to-print-prototype-enclosure.md?raw";
import developmentSetup from "../docs/how-to-set-up-development.md?raw";
import iphoneTesting from "../docs/how-to-test-on-iphone.md?raw";
import presetFileFormat from "../docs/preset-file-format.md?raw";
import prototypeBringUp from "../docs/prototype-bring-up.md?raw";
import prototypeHardware from "../docs/prototype-hardware-reference.md?raw";
import usbAudioBringUp from "../docs/usb-audio-bring-up.md?raw";

export type DocumentationSection = "How-to guides" | "Reference" | "Architecture";

export type DocumentationPage = {
  body: string;
  description: string;
  section: DocumentationSection;
  slug: string;
  title: string;
};

export const documentationPages: DocumentationPage[] = [
  {
    body: developmentSetup,
    description: "Install dependencies, run checks, and build browser and Seed3 targets.",
    section: "How-to guides",
    slug: "set-up-development",
    title: "Set up LineRack development",
  },
  {
    body: prototypeBringUp,
    description: "Wire, flash, and verify the Seed3 breadboard prototype.",
    section: "How-to guides",
    slug: "bring-up-seed3-prototype",
    title: "Bring up the Seed3 prototype",
  },
  {
    body: enclosurePrinting,
    description: "Render, print, and fit-check the development enclosure.",
    section: "How-to guides",
    slug: "print-prototype-enclosure",
    title: "Print the development enclosure",
  },
  {
    body: usbAudioBringUp,
    description: "Build, flash, and troubleshoot the composite USB Audio image.",
    section: "How-to guides",
    slug: "verify-usb-audio",
    title: "Verify USB Audio on Seed3",
  },
  {
    body: iphoneTesting,
    description: "Verify direct iPhone playback, bus power, and reconnect behavior.",
    section: "How-to guides",
    slug: "test-on-iphone",
    title: "Test LineRack on iPhone",
  },
  {
    body: hidProtocol,
    description: "Reports, framing, commands, data models, and protocol limits.",
    section: "Reference",
    slug: "hid-control-protocol",
    title: "HID control protocol",
  },
  {
    body: presetFileFormat,
    description: "Version 1 JSON fields, constraints, defaults, and compatibility rules.",
    section: "Reference",
    slug: "preset-file-format",
    title: "Preset file format",
  },
  {
    body: prototypeHardware,
    description: "Verified Seed3 pinout, prototype modules, limits, and beta BOM target.",
    section: "Reference",
    slug: "prototype-hardware",
    title: "Prototype hardware",
  },
  {
    body: deployment,
    description: "Static build, public routes, Cloudflare settings, and browser boundaries.",
    section: "Reference",
    slug: "web-deployment",
    title: "Web deployment",
  },
  {
    body: analogInput,
    description: "Why Seed3's codec covers analog input and what the carrier still needs.",
    section: "Architecture",
    slug: "analog-input-architecture",
    title: "Analog input architecture",
  },
  {
    body: dspAndDisplay,
    description: "How signal processing, visualization, and device interaction fit together.",
    section: "Architecture",
    slug: "dsp-and-display-architecture",
    title: "DSP and display architecture",
  },
];

export const documentationSections: DocumentationSection[] = [
  "How-to guides",
  "Reference",
  "Architecture",
];

export const findDocumentationPage = (slug: string) =>
  documentationPages.find((page) => page.slug === slug);
