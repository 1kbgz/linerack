import { useEffect } from "react";
import { SiteNav } from "./site-nav";
import { SITE_ORIGIN } from "./site-url";
import "./product.css";

export default function ProductPage() {
  useEffect(() => {
    const productUrl = new URL("/product", SITE_ORIGIN).href;
    document.title = "LineRack · Portable programmable audio DSP";
    document
      .querySelector('meta[name="description"]')
      ?.setAttribute(
        "content",
        "LineRack is a planned USB-powered audio processor with effect chains configured in Chrome and saved on the device.",
      );
    document
      .querySelector('meta[property="og:title"]')
      ?.setAttribute("content", "LineRack · Portable effects for USB and analog audio");
    document
      .querySelector('meta[property="og:description"]')
      ?.setAttribute("content", "A planned source-available audio processor configured in Chrome.");
    document.querySelector('meta[property="og:url"]')?.setAttribute("content", productUrl);
    document.querySelector('link[rel="canonical"]')?.setAttribute("href", productUrl);
  }, []);

  return (
    <div className="product-page">
      <header className="topbar product-topbar">
        <a className="brand-block brand-link" href="/">
          <p className="eyebrow">Portable audio DSP</p>
          <span className="product-brand">LineRack</span>
        </a>
        <SiteNav current="product" />
      </header>

      <main className="product-main">
        <section className="product-hero">
          <div className="product-hero-copy">
            <p className="eyebrow">Source available · Beta planned</p>
            <h1>Portable effects for USB and analog audio.</h1>
            <p>
              LineRack runs effects between your audio source and headphones. Build the chain in
              Chrome. Playback does not need an app.
            </p>
            <div className="product-actions">
              <a className="product-primary" href="/configure">
                Try the configurator
              </a>
              <a className="product-secondary" href="#availability">
                Beta availability
              </a>
            </div>
          </div>

          <div
            aria-label="Concept illustration of a LineRack device"
            className="device-stage"
            role="img"
          >
            <div className="device-concept">
              <div className="device-port usb-port" />
              <div className="device-screen">
                <span>2</span>
                <div>
                  <strong>BASS</strong>
                  <small>4 BLOCKS · LIVE</small>
                </div>
              </div>
              <div className="device-button" />
              <div className="device-port audio-port" />
            </div>
            <p>Concept enclosure. Final hardware will look different.</p>
          </div>
        </section>

        <section className="product-signal" aria-labelledby="signal-title">
          <div>
            <p className="eyebrow">Your signal path</p>
            <h2 id="signal-title">Source → effects → output</h2>
          </div>
          <ol>
            <li>
              <span>01</span>USB or analog audio
            </li>
            <li>
              <span>02</span>Up to eight effects, in any order
            </li>
            <li>
              <span>03</span>Stereo headphone output
            </li>
          </ol>
        </section>

        <section className="product-features" aria-labelledby="features-title">
          <div className="product-section-copy">
            <p className="eyebrow">Designed for daily use</p>
            <h2 id="features-title">Your presets stay on LineRack.</h2>
          </div>
          <div className="product-feature-grid">
            <article>
              <span>Chain</span>
              <h3>Build an effect chain</h3>
              <p>Put EQ, filters, dynamics, reverb, delay, and utility blocks in any order.</p>
            </article>
            <article>
              <span>Configure</span>
              <h3>Configure it in Chrome</h3>
              <p>Use desktop Chrome to edit presets. Saved setups stay on LineRack.</p>
            </article>
            <article>
              <span>Recall</span>
              <h3>Use one button</h3>
              <p>Double-tap to change presets. A single tap wakes the monochrome display.</p>
            </article>
            <article>
              <span>Build</span>
              <h3>Make your own LineRack</h3>
              <p>
                Source files are available for personal projects. Commercial use requires a separate
                license.
              </p>
            </article>
          </div>
        </section>

        <section className="product-beta" id="availability" aria-labelledby="availability-title">
          <div>
            <p className="eyebrow">First beta</p>
            <h2 id="availability-title">We plan to build a small beta run.</h2>
            <p>
              Our Seed3 prototype processes USB audio from Macs and iPhones. Beta orders will open
              after we test the carrier board, enclosure, Windows, and Linux.
            </p>
          </div>
          <dl>
            <div>
              <dt>Target beta price</dt>
              <dd>$99–119</dd>
            </div>
            <div>
              <dt>Configuration</dt>
              <dd>Desktop Chrome</dd>
            </div>
            <div>
              <dt>Playback</dt>
              <dd>Browser-free</dd>
            </div>
            <div>
              <dt>Status</dt>
              <dd>Preorders not open</dd>
            </div>
          </dl>
          <div className="product-beta-actions">
            <a className="product-primary" href="https://github.com/1kbgz/linerack">
              Follow development on GitHub
            </a>
            <small>Target price and specifications may change after testing.</small>
          </div>
        </section>
      </main>

      <footer className="product-footer">
        <span>LineRack source is available for personal projects.</span>
        <a href="/docs">Read the technical documentation</a>
      </footer>
    </div>
  );
}
