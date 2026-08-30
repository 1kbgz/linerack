import { describe, expect, it } from "vitest";
import { documentationPages, documentationSections, findDocumentationPage } from "./docs-content";

describe("documentation catalog", () => {
  it("uses unique non-empty routes with source content", () => {
    const slugs = documentationPages.map((page) => page.slug);
    expect(new Set(slugs).size).toBe(slugs.length);
    expect(slugs.every((slug) => slug.length > 0 && !slug.includes("/"))).toBe(true);
    expect(documentationPages.every((page) => page.body.startsWith("# "))).toBe(true);
  });

  it("keeps every page in a visible section", () => {
    expect(documentationPages.every((page) => documentationSections.includes(page.section))).toBe(
      true,
    );
  });

  it("finds pages by public slug", () => {
    expect(findDocumentationPage("hid-control-protocol")?.title).toBe("HID control protocol");
    expect(findDocumentationPage("missing")).toBeUndefined();
  });
});
