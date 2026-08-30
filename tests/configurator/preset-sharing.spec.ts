import { expect, Page, test } from "@playwright/test";

const openSimulator = async (page: Page) => {
  await page.goto("/configure");
  await page.getByRole("button", { name: "Continue with simulator" }).click();
};

test("loads a library recipe into only the selected draft slot", async ({ page }) => {
  await openSimulator(page);
  await page.getByRole("button", { name: "Browse preset library" }).click();
  const liveRoom = page.locator(".preset-library-card").filter({ hasText: "Live Room" });
  await liveRoom.getByRole("button", { name: "Use in slot 1" }).click();

  await expect(page.getByRole("textbox", { name: "Preset name" })).toHaveValue("Live Room");
  await expect(page.getByText("Unapplied changes")).toBeVisible();
  await expect(page.locator(".slot").nth(1).getByText("Punch")).toBeVisible();
});

test("downloads the selected slot as a single-preset file", async ({ page }) => {
  await openSimulator(page);
  const downloadPromise = page.waitForEvent("download");
  await page.getByRole("button", { name: "Download preset" }).click();
  const download = await downloadPromise;

  expect(download.suggestedFilename()).toBe("linerack-clean.linerack-preset.json");
});

test("imports a shared preset into the selected slot", async ({ page }) => {
  await openSimulator(page);
  const downloadPromise = page.waitForEvent("download");
  await page.getByRole("button", { name: "Download preset" }).click();
  const downloadedPath = await (await downloadPromise).path();
  if (!downloadedPath) throw new Error("Downloaded preset has no local path");

  await page.locator(".slot").nth(1).click();
  await page.locator('.file-actions input[type="file"]').nth(1).setInputFiles(downloadedPath);

  await expect(page.getByRole("textbox", { name: "Preset name" })).toHaveValue("Clean");
  await expect(page.getByText("Imported", { exact: false })).toBeVisible();
  await expect(page.getByText("Unapplied changes")).toBeVisible();
});
