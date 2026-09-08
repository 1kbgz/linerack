import { expect, test } from "@playwright/test";

for (const [path, canonical] of [
  ["/", "https://linerack.dev/"],
  ["/configure", "https://linerack.dev/configure"],
  ["/product", "https://linerack.dev/product"],
  ["/docs", "https://linerack.dev/docs"],
  ["/docs/develop-dsp-plugin", "https://linerack.dev/docs/develop-dsp-plugin"],
] as const) {
  test(`${path} uses the production canonical URL`, async ({ page }) => {
    await page.goto(path);

    await expect(page.locator('link[rel="canonical"]')).toHaveAttribute("href", canonical);
    await expect(page.locator('meta[property="og:url"]')).toHaveAttribute("content", canonical);
  });
}

test("robots.txt permits crawling and declares the sitemap", async ({ request }) => {
  const response = await request.get("/robots.txt");

  expect(response.ok()).toBe(true);
  expect(response.headers()["content-type"]).toContain("text/plain");
  expect(await response.text()).toContain("Sitemap: https://linerack.dev/sitemap.xml");
});

test("sitemap.xml contains every public route", async ({ page, request }) => {
  await page.goto("/docs");
  const documentationLinks = page.locator('a[href^="/docs/"]');
  await documentationLinks.first().waitFor();
  const documentationUrls = await documentationLinks.evaluateAll((links) => [
    ...new Set(
      links.map(
        (link) => new URL((link as HTMLAnchorElement).pathname, "https://linerack.dev").href,
      ),
    ),
  ]);
  const response = await request.get("/sitemap.xml");
  const sitemap = await response.text();
  const expectedUrls = [
    "https://linerack.dev/",
    "https://linerack.dev/product",
    "https://linerack.dev/configure",
    "https://linerack.dev/docs",
    ...documentationUrls,
  ];

  expect(response.ok()).toBe(true);
  expect(response.headers()["content-type"]).toMatch(/^(?:application|text)\/xml/);
  expect([...sitemap.matchAll(/<loc>([^<]+)<\/loc>/g)].map((match) => match[1])).toEqual(
    expectedUrls,
  );
});
