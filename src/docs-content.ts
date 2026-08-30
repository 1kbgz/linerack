import analogInput from "../docs/analog-input-direction.md?raw";
import deployment from "../docs/deployment-reference.md?raw";
import dspAndDisplay from "../docs/dsp-and-display-direction.md?raw";
import firmwareBuildTargets from "../docs/firmware-build-targets.md?raw";
import hidProtocol from "../docs/hid-protocol.md?raw";
import enclosurePrinting from "../docs/how-to-print-prototype-enclosure.md?raw";
import developmentSetup from "../docs/how-to-set-up-development.md?raw";
import iphoneTesting from "../docs/how-to-test-on-iphone.md?raw";
import templateUpdates from "../docs/how-to-update-template.md?raw";
import presetFileFormat from "../docs/preset-file-format.md?raw";
import prototypeBringUp from "../docs/prototype-bring-up.md?raw";
import prototypeHardware from "../docs/prototype-hardware-reference.md?raw";
import usbAudioBringUp from "../docs/usb-audio-bring-up.md?raw";

export type DocumentationSection = "Tutorial" | "How-to guides" | "Reference" | "Explanation";

export type DocumentationPage = {
  body: string;
  description: string;
  section: DocumentationSection;
  slug: string;
  sourceFile: string;
  title: string;
};

export const documentationPages: DocumentationPage[] = [
  {
    body: prototypeBringUp,
    description: "Build one complete Seed3 prototype and verify each subsystem in sequence.",
    section: "Tutorial",
    slug: "bring-up-seed3-prototype",
    sourceFile: "prototype-bring-up.md",
    title: "Build and verify the Seed3 prototype",
  },
  {
    body: developmentSetup,
    description: "Install dependencies, run checks, and build browser and Seed3 targets.",
    section: "How-to guides",
    slug: "set-up-development",
    sourceFile: "how-to-set-up-development.md",
    title: "Set up LineRack development",
  },
  {
    body: enclosurePrinting,
    description: "Render, print, and fit-check the development enclosure.",
    section: "How-to guides",
    slug: "print-prototype-enclosure",
    sourceFile: "how-to-print-prototype-enclosure.md",
    title: "Print the development enclosure",
  },
  {
    body: usbAudioBringUp,
    description: "Build, flash, and troubleshoot the composite USB Audio image.",
    section: "How-to guides",
    slug: "verify-usb-audio",
    sourceFile: "usb-audio-bring-up.md",
    title: "Verify USB Audio on Seed3",
  },
  {
    body: iphoneTesting,
    description: "Verify direct iPhone playback, bus power, and reconnect behavior.",
    section: "How-to guides",
    slug: "test-on-iphone",
    sourceFile: "how-to-test-on-iphone.md",
    title: "Test LineRack on iPhone",
  },
  {
    body: templateUpdates,
    description: "Apply upstream Copier changes without losing LineRack-specific behavior.",
    section: "How-to guides",
    slug: "update-repository-template",
    sourceFile: "how-to-update-template.md",
    title: "Update from the repository template",
  },
  {
    body: hidProtocol,
    description: "Reports, framing, commands, data models, and protocol limits.",
    section: "Reference",
    slug: "hid-control-protocol",
    sourceFile: "hid-protocol.md",
    title: "HID control protocol",
  },
  {
    body: presetFileFormat,
    description: "Version 1 JSON fields, constraints, defaults, and compatibility rules.",
    section: "Reference",
    slug: "preset-file-format",
    sourceFile: "preset-file-format.md",
    title: "Preset file format",
  },
  {
    body: prototypeHardware,
    description: "Verified Seed3 modules, wiring, interfaces, and prototype limits.",
    section: "Reference",
    slug: "prototype-hardware",
    sourceFile: "prototype-hardware-reference.md",
    title: "Prototype hardware",
  },
  {
    body: deployment,
    description: "Static build, public routes, Cloudflare settings, and browser boundaries.",
    section: "Reference",
    slug: "web-deployment",
    sourceFile: "deployment-reference.md",
    title: "Web deployment",
  },
  {
    body: firmwareBuildTargets,
    description: "Makefiles, generated images, purposes, and shared build-directory behavior.",
    section: "Reference",
    slug: "firmware-build-targets",
    sourceFile: "firmware-build-targets.md",
    title: "Firmware build targets",
  },
  {
    body: analogInput,
    description: "Why Seed3's codec covers analog input and what the carrier still needs.",
    section: "Explanation",
    slug: "analog-input-architecture",
    sourceFile: "analog-input-direction.md",
    title: "Analog input architecture",
  },
  {
    body: dspAndDisplay,
    description: "How signal processing, visualization, and device interaction fit together.",
    section: "Explanation",
    slug: "dsp-and-display-architecture",
    sourceFile: "dsp-and-display-direction.md",
    title: "DSP and display architecture",
  },
];

export const documentationSections: DocumentationSection[] = [
  "Tutorial",
  "How-to guides",
  "Reference",
  "Explanation",
];

export const documentationSectionDescriptions: Record<DocumentationSection, string> = {
  Tutorial: "Start with a guided build that produces a working hardware prototype.",
  "How-to guides": "Complete a specific development, testing, or maintenance task.",
  Reference: "Look up exact interfaces, formats, hardware limits, and deployment facts.",
  Explanation: "Understand the design constraints and decisions behind LineRack.",
};

export const findDocumentationPage = (slug: string) =>
  documentationPages.find((page) => page.slug === slug);

export const resolveDocumentationHref = (href: string | undefined) => {
  if (!href) return href;
  const [path, fragment] = href.split("#", 2);
  const sourceFile = path.split("/").at(-1);
  const page = documentationPages.find((candidate) => candidate.sourceFile === sourceFile);
  return page ? `/docs/${page.slug}${fragment ? `#${fragment}` : ""}` : href;
};
