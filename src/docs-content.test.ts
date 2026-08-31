import { readdirSync } from "node:fs";
import { describe, expect, it } from "vitest";
import {
  documentationPages,
  documentationSections,
  findDocumentationPage,
  resolveDocumentationHref,
} from "./docs-content";

describe("documentation catalog", () => {
  it("uses unique non-empty routes with source content", () => {
    const slugs = documentationPages.map((page) => page.slug);
    expect(new Set(slugs).size).toBe(slugs.length);
    expect(slugs.every((slug) => slug.length > 0 && !slug.includes("/"))).toBe(true);
    expect(documentationPages.every((page) => page.body.startsWith("# "))).toBe(true);
    for (const page of documentationPages.filter(
      (candidate) => candidate.section === "How-to guides",
    )) {
      const heading = page.body.match(/^# (.+)$/m)?.[1];
      const title = heading?.replace(/^How to /, "");
      expect(page.title).toBe(title && title[0].toUpperCase() + title.slice(1));
    }
  });

  it("keeps every page in a visible section", () => {
    expect(documentationSections).toEqual([
      "Tutorial",
      "How-to guides",
      "Reference",
      "Explanation",
    ]);
    expect(documentationPages.every((page) => documentationSections.includes(page.section))).toBe(
      true,
    );
  });

  it("publishes every developer Markdown file on the website", () => {
    const sourceFiles = documentationPages.map((page) => page.sourceFile).sort();
    const markdownFiles = readdirSync(new URL("../docs/", import.meta.url))
      .filter((file) => file.endsWith(".md"))
      .sort();

    expect(new Set(sourceFiles).size).toBe(sourceFiles.length);
    expect(sourceFiles).toEqual(markdownFiles);
  });

  it("finds pages by public slug", () => {
    expect(findDocumentationPage("hid-control-protocol")?.title).toBe("HID control protocol");
    expect(findDocumentationPage("missing")).toBeUndefined();
  });

  it("resolves repository Markdown links to public documentation routes", () => {
    expect(resolveDocumentationHref("prototype-bring-up.md")).toBe(
      "/docs/bring-up-seed3-prototype",
    );
    expect(resolveDocumentationHref("./hid-protocol.md#commands")).toBe(
      "/docs/hid-control-protocol#commands",
    );
    expect(resolveDocumentationHref("https://example.com/reference")).toBe(
      "https://example.com/reference",
    );
  });
});
