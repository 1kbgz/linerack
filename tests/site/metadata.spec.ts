import { expect, test } from "@playwright/test";

for (const [path, canonical] of [
  ["/", "https://linerack.dev/"],
  ["/configure", "https://linerack.dev/configure"],
  ["/product", "https://linerack.dev/product"],
] as const) {
  test(`${path} uses the production canonical URL`, async ({ page }) => {
    await page.goto(path);

    await expect(page.locator('link[rel="canonical"]')).toHaveAttribute("href", canonical);
    await expect(page.locator('meta[property="og:url"]')).toHaveAttribute("content", canonical);
  });
}
