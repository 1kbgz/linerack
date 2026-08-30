import AxeBuilder from "@axe-core/playwright";
import { expect, test } from "@playwright/test";

test("landing page has no automatically detectable accessibility violations", async ({ page }) => {
  await page.goto("/");
  await expect(page).toHaveTitle("LineRack · Portable programmable audio DSP");
  await expect(
    page.getByRole("navigation", { name: "LineRack destinations" }).getByRole("link"),
  ).toHaveCount(4);

  const results = await new AxeBuilder({ page }).analyze();
  expect(results.violations).toEqual([]);
});

test("landing page fits a phone viewport", async ({ page }) => {
  await page.setViewportSize({ width: 375, height: 667 });
  await page.goto("/");

  for (const label of ["Configure", "Product", "Developer", "GitHub"]) {
    await expect(page.getByRole("link", { name: new RegExp(label) })).toBeVisible();
  }
  expect(await page.evaluate(() => document.documentElement.scrollWidth)).toBe(375);
});

test("configurator has no automatically detectable accessibility violations", async ({ page }) => {
  await page.goto("/configure");
  await expect(page).toHaveTitle("LineRack Configurator");

  const results = await new AxeBuilder({ page }).analyze();
  expect(results.violations).toEqual([]);

  const continueButton = page.getByRole("button", { name: "Continue with simulator" });
  if (await continueButton.isVisible()) await continueButton.click();
  await page.getByRole("button", { name: "Browse preset library" }).click();
  await expect(page.getByRole("heading", { name: "Preset library" })).toBeVisible();
  const libraryResults = await new AxeBuilder({ page }).analyze();
  expect(libraryResults.violations).toEqual([]);

  await page.getByRole("button", { name: "Close" }).click();
  await page.getByRole("button", { name: /Global/ }).click();
  const globalResults = await new AxeBuilder({ page }).analyze();
  expect(globalResults.violations).toEqual([]);
});

test("documentation has no automatically detectable accessibility violations", async ({ page }) => {
  await page.goto("/docs");
  await expect(page).toHaveTitle("Documentation · LineRack");

  const results = await new AxeBuilder({ page }).analyze();
  expect(results.violations).toEqual([]);

  await page.goto("/docs/hid-control-protocol");
  await expect(page).toHaveTitle("HID control protocol · LineRack");
  await expect(page.getByRole("heading", { level: 1 })).toHaveText("LineRack HID control protocol");

  const articleResults = await new AxeBuilder({ page }).analyze();
  expect(articleResults.violations).toEqual([]);
});

test("product page has no automatically detectable accessibility violations", async ({ page }) => {
  await page.goto("/product");
  await expect(page).toHaveTitle("LineRack · Portable programmable audio DSP");
  await expect(page.getByRole("heading", { level: 1 })).toHaveText(
    "Portable effects for USB and analog audio.",
  );

  const results = await new AxeBuilder({ page }).analyze();
  expect(results.violations).toEqual([]);
});
