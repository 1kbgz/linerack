# Web deployment reference

LineRack ships as one static Vite application. It has no server-side
application code or database.

## Build contract

| Property | Value |
| --- | --- |
| Package manager | pnpm 11.9 |
| Node.js | 22 or newer |
| Install command | `pnpm install --frozen-lockfile` |
| Build command | `pnpm build` |
| Output directory | `dist` |
| Required environment variables | None |

## Public routes

| Route | Content |
| --- | --- |
| `/` | Site landing page |
| `/configure` | Hardware configurator and simulator |
| `/product` | Device overview and availability status |
| `/docs` | Developer documentation index |
| `/docs/<slug>` | Developer documentation page |

`public/_redirects` becomes `dist/_redirects`. Its `/* /index.html 200` rule
allows the client to resolve deep links.

## Cloudflare Pages

| Setting | Value |
| --- | --- |
| Production branch | `main` |
| Production URL | `https://linerack.dev` |
| Framework | Vite or equivalent manual settings |
| Build command | `pnpm build` |
| Build output directory | `dist` |
| Root directory | Repository root |
| Node compatibility | Node.js 22 |

The `1kbgz/terraform` repository owns the `linerack` Pages project, custom
domain, and DNS record. The LineRack workflow uploads its tested `dist` artifact
with Wrangler Direct Upload after web and firmware jobs pass on `main`.
Cloudflare Git integration is not used.

Required GitHub Actions values:

| Kind | Name | Value |
| --- | --- | --- |
| Repository variable | `CLOUDFLARE_ACCOUNT_ID` | Account containing the `linerack` Pages project |
| Repository secret | `CLOUDFLARE_API_TOKEN` | Token with Account / Cloudflare Pages / Edit |

The deployment job remains skipped when `CLOUDFLARE_ACCOUNT_ID` is absent.
Credential values are intentionally omitted from this repository and its
documentation. Only their GitHub Actions names form part of the deployment
contract.

## Repository automation

The Python Project Templates Copier Update GitHub App reads
`.copier-answers.yaml`. The Python Templates Automerge GitHub App handles
label-controlled pull-request automerge. Neither app participates in site
deployment.

## Browser boundary

Static landing, product, and documentation pages work in current standards-
based browsers. Hardware configuration additionally requires desktop Chromium,
a secure HTTPS origin or recognized local origin, and explicit WebHID device
authorization.

Playback and stored preset selection do not depend on the site after a setup is
written. iPhone playback uses USB Audio; mobile Safari configuration is not
supported.

## External services

The application has no analytics, accounts, payments, email, waitlist, or
remote preset service.

## Platform references

- [Cloudflare Pages Vite deployment](https://developers.cloudflare.com/pages/framework-guides/deploy-a-vite3-project/)
- [Cloudflare Pages Direct Upload](https://developers.cloudflare.com/pages/how-to/use-direct-upload-with-continuous-integration/)
- [Cloudflare Pages redirects](https://developers.cloudflare.com/pages/configuration/redirects/)

## See also

- [How to deploy the website](how-to-deploy-website.md)
