import { useEffect } from "react";
import ReactMarkdown from "react-markdown";
import remarkGfm from "remark-gfm";
import { documentationPages, documentationSections, findDocumentationPage } from "./docs-content";
import { SiteNav } from "./site-nav";
import "./docs.css";

type DocsAppProps = {
  pathname: string;
};

const slugFromPathname = (pathname: string) =>
  pathname.replace(/^\/docs\/?/, "").replace(/\/$/, "");

export default function DocsApp({ pathname }: DocsAppProps) {
  const slug = slugFromPathname(pathname);
  const selectedPage = slug ? findDocumentationPage(slug) : undefined;

  useEffect(() => {
    document.title = selectedPage
      ? `${selectedPage.title} · LineRack`
      : slug
        ? "Page not found · LineRack"
        : "Documentation · LineRack";
  }, [selectedPage, slug]);

  return (
    <div className="docs-page">
      <header className="topbar docs-topbar">
        <a className="brand-block brand-link" href="/">
          <p className="eyebrow">Portable audio DSP</p>
          <span className="docs-brand">LineRack</span>
        </a>
        <SiteNav current="docs" />
      </header>

      <main className="docs-layout">
        <aside className="docs-sidebar">
          <a className={!slug ? "docs-home active" : "docs-home"} href="/docs">
            Documentation home
          </a>
          {documentationSections.map((section) => (
            <section key={section}>
              <h2>{section}</h2>
              <ul>
                {documentationPages
                  .filter((page) => page.section === section)
                  .map((page) => (
                    <li key={page.slug}>
                      <a
                        aria-current={page.slug === slug ? "page" : undefined}
                        href={`/docs/${page.slug}`}
                      >
                        {page.title}
                      </a>
                    </li>
                  ))}
              </ul>
            </section>
          ))}
        </aside>

        {selectedPage ? (
          <article className="docs-article">
            <ReactMarkdown remarkPlugins={[remarkGfm]}>{selectedPage.body}</ReactMarkdown>
          </article>
        ) : slug ? (
          <article className="docs-article docs-not-found">
            <p className="eyebrow">404</p>
            <h1>Documentation page not found</h1>
            <p>The requested LineRack documentation page does not exist.</p>
            <a href="/docs">Return to documentation</a>
          </article>
        ) : (
          <article className="docs-index">
            <p className="eyebrow">Documentation</p>
            <h1>LineRack documentation</h1>
            <p className="docs-intro">
              Set up the hardware, build the firmware, or look up protocol and design details.
            </p>
            {documentationSections.map((section) => (
              <section className="docs-group" key={section}>
                <h2>{section}</h2>
                <div className="docs-cards">
                  {documentationPages
                    .filter((page) => page.section === section)
                    .map((page) => (
                      <a href={`/docs/${page.slug}`} key={page.slug}>
                        <strong>{page.title}</strong>
                        <span>{page.description}</span>
                      </a>
                    ))}
                </div>
              </section>
            ))}
          </article>
        )}
      </main>
    </div>
  );
}
