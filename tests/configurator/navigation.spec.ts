import { expect, Page, test } from "@playwright/test";

const openSimulator = async (page: Page) => {
  await page.goto("/configure");
  await page.getByRole("button", { name: "Continue with simulator" }).click();
};

test("separates global settings from preset editing", async ({ page }) => {
  await openSimulator(page);

  await expect(page.getByText("Choose a starting preset")).toBeVisible();
  await expect(page.getByText("Signal chain", { exact: true })).toBeVisible();
  await expect(page.getByRole("button", { name: "Browse preset library" })).toBeVisible();
  await expect(page.getByRole("button", { name: "Load starter set" })).toBeVisible();

  await page.getByRole("button", { name: /Global/ }).click();
  await expect(page.getByRole("heading", { name: "Settings for every preset" })).toBeVisible();
  await expect(page.getByText("Preset slots")).toBeHidden();
  await expect(page.locator("main details:visible")).toHaveCount(4);
  await expect(page.locator(".routing-panel")).toHaveAttribute("open", "");
  await expect(page.locator(".audition-panel")).toHaveAttribute("open", "");
  await expect(page.locator(".display-panel")).toHaveAttribute("open", "");
  await expect(page.locator(".diagnostics-panel")).not.toHaveAttribute("open", "");

  await page.getByText("Choose the audio source").click();
  await expect(page.locator(".routing-panel")).not.toHaveAttribute("open", "");
  await page.getByText("Choose the audio source").click();
  await page.getByLabel("Audio source mode").selectOption("analog");
  await expect(page.getByText("Unapplied changes")).toBeVisible();
  await page.getByRole("button", { name: "Apply to device" }).click();
  await expect(page.getByText("Unapplied changes")).toBeHidden();
});

test("shows live diagnostics without changing the setup", async ({ page }) => {
  await openSimulator(page);
  await page.getByRole("button", { name: /Global/ }).click();

  await page.getByText("Check USB audio").click();

  await expect(page.locator(".diagnostics-panel")).toHaveAttribute("open", "");
  await expect(page.getByText("Healthy", { exact: true })).toBeVisible();
  await expect(page.getByText("USB packets", { exact: true })).toBeVisible();
  await expect(page.getByText("96 frames", { exact: true })).toBeVisible();
  await expect(page.getByText("Unapplied changes")).toBeHidden();
});

test("applies a processor preset to one block", async ({ page }) => {
  await openSimulator(page);
  const presetSelect = page.getByRole("combobox", { name: "Parametric EQ preset" });

  await presetSelect.selectOption("bass-lift");

  await expect(presetSelect).toHaveValue("bass-lift");
  await expect(page.getByText("Unapplied changes")).toBeVisible();
});

test("keeps primary configurator navigation usable on a phone viewport", async ({ page }) => {
  await page.setViewportSize({ width: 390, height: 844 });
  await openSimulator(page);

  await expect(page.getByRole("button", { name: /Global/ })).toBeVisible();
  await expect(page.getByRole("button", { name: /Presets/ })).toBeVisible();
  await expect(page.getByRole("button", { name: "Browse preset library" })).toBeVisible();
  expect(await page.evaluate(() => document.documentElement.scrollWidth)).toBe(390);

  await page.getByRole("button", { name: /Global/ }).click();
  await expect(page.getByRole("button", { name: "Apply to device" })).toBeVisible();
  expect(await page.evaluate(() => document.documentElement.scrollWidth)).toBe(390);
});
