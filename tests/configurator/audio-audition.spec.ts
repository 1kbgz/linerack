import { expect, test } from "@playwright/test";

test("loads a local audition file without uploading it", async ({ page }) => {
  await page.goto("/configure");
  await page.getByRole("button", { name: "Continue with simulator" }).click();
  await page.getByRole("button", { name: /Global/ }).click();
  await page.locator('.audition-panel input[type="file"]').setInputFiles({
    name: "familiar-track.wav",
    mimeType: "audio/wav",
    buffer: Buffer.from("RIFF"),
  });

  await expect(page.getByText("familiar-track.wav")).toBeVisible();
  await expect(page.getByLabel("Audition familiar-track.wav")).toBeVisible();
});

test("selects visualizer as the saved display mode", async ({ page }) => {
  await page.goto("/configure");
  await page.getByRole("button", { name: "Continue with simulator" }).click();
  await page.getByRole("button", { name: /Global/ }).click();
  await page.getByRole("button", { name: "Visualizer" }).click();

  await expect(page.getByRole("button", { name: "Visualizer" })).toHaveClass(/selected/);
  await expect(page.getByLabel("Visualizer OLED preview")).toBeVisible();
  await expect(page.getByText("Unapplied changes")).toBeVisible();
});
