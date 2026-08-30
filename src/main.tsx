import { lazy, StrictMode, Suspense } from "react";
import { createRoot } from "react-dom/client";
import "./styles.css";

const Configurator = lazy(() => import("./App"));
const DocsApp = lazy(() => import("./docs-app"));
const LandingPage = lazy(() => import("./landing-page"));
const ProductPage = lazy(() => import("./product-page"));

const page =
  window.location.pathname === "/product" || window.location.pathname.startsWith("/product/") ? (
    <ProductPage />
  ) : window.location.pathname === "/docs" || window.location.pathname.startsWith("/docs/") ? (
    <DocsApp pathname={window.location.pathname} />
  ) : window.location.pathname === "/configure" ||
    window.location.pathname.startsWith("/configure/") ? (
    <Configurator />
  ) : (
    <LandingPage />
  );

createRoot(document.getElementById("root")!).render(
  <StrictMode>
    <Suspense fallback={<div className="loading">Loading LineRack…</div>}>{page}</Suspense>
  </StrictMode>,
);
