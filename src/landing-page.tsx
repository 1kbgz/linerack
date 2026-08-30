import { useEffect } from "react";
import { SITE_ORIGIN } from "./site-url";
import "./landing.css";

const links = [
  {
    description: "Edit presets and load them onto a connected LineRack.",
    href: "/configure",
    label: "Configure",
    marker: "BUILD",
  },
  {
    description: "See the hardware plan, beta status, and expected price.",
    href: "/product",
    label: "Product",
    marker: "DEVICE",
  },
  {
    description: "Find setup guides, protocol references, and design notes.",
    href: "/docs",
    label: "Developer",
    marker: "DOCS",
  },
  {
    description: "Browse source code, hardware files, and BOMs.",
    href: "https://github.com/1kbgz/linerack",
    label: "GitHub",
    marker: "SOURCE",
  },
] as const;

export default function LandingPage() {
  useEffect(() => {
    const landingUrl = new URL("/", SITE_ORIGIN).href;
    document.title = "LineRack · Portable programmable audio DSP";
    document
      .querySelector('meta[name="description"]')
      ?.setAttribute(
        "content",
        "Configure LineRack, see the beta hardware, or read the developer docs.",
      );
    document
      .querySelector('meta[property="og:title"]')
      ?.setAttribute("content", "LineRack · Portable programmable audio DSP");
    document
      .querySelector('meta[property="og:description"]')
      ?.setAttribute(
        "content",
        "A portable audio processor with effect chains configured in Chrome and saved on the device.",
      );
    document.querySelector('meta[property="og:url"]')?.setAttribute("content", landingUrl);
    document.querySelector('link[rel="canonical"]')?.setAttribute("href", landingUrl);
  }, []);

  return (
    <div className="landing-page">
      <main className="landing-main">
        <header className="landing-hero">
          <div className="landing-wordmark" aria-hidden="true">
            LR
          </div>
          <div>
            <p className="eyebrow">Portable audio DSP · Source available</p>
            <h1>LineRack</h1>
            <p className="landing-intro">
              Build an effect chain in Chrome. LineRack stores it and processes audio without the
              browser.
            </p>
          </div>
        </header>

        <nav aria-label="LineRack destinations" className="landing-grid">
          {links.map((link, index) => (
            <a className="landing-tile" href={link.href} key={link.label}>
              <span className="landing-tile-number">{String(index + 1).padStart(2, "0")}</span>
              <span className="landing-tile-marker">{link.marker}</span>
              <strong>{link.label}</strong>
              <span className="landing-tile-description">{link.description}</span>
              <span aria-hidden="true" className="landing-tile-arrow">
                ↗
              </span>
            </a>
          ))}
        </nav>
      </main>

      <footer className="landing-footer">
        <span>USB-powered · configured in Chrome · standalone playback</span>
        <span>Working Seed3 prototype</span>
      </footer>
    </div>
  );
}
