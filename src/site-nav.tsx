type SitePage = "configurator" | "product" | "docs";

type SiteNavProps = {
  current: SitePage;
};

export function SiteNav({ current }: SiteNavProps) {
  return (
    <nav aria-label="Primary" className="site-nav">
      <a aria-current={current === "product" ? "page" : undefined} href="/product">
        Product
      </a>
      <a aria-current={current === "configurator" ? "page" : undefined} href="/configure">
        Configurator
      </a>
      <a aria-current={current === "docs" ? "page" : undefined} href="/docs">
        Docs
      </a>
      <a href="https://github.com/1kbgz/linerack">GitHub</a>
    </nav>
  );
}
