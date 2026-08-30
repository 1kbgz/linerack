# Web deployment reference

LineRack's product, configurator, and documentation pages form one static Vite
application. The deployment contains no server-side application code or
database.

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
| `/product` | Future-device overview and beta status |
| `/` | Hardware configurator and simulator |
| `/docs` | Documentation index |
| `/docs/<slug>` | Individual bundled documentation page |

`public/_redirects` is copied into the build as `dist/_redirects`. Its fallback
rule serves `index.html` for deep links so the client can select the requested
route.

## Cloudflare Pages settings

| Setting | Value |
| --- | --- |
| Production branch | `main` |
| Production URL | `https://linerack.dev` |
| Framework preset | Vite, or equivalent manual settings |
| Build command | `pnpm build` |
| Build output directory | `dist` |
| Root directory | Repository root |
| Node compatibility | Node.js 22 |

Cloudflare Pages consumes the `_redirects` file from the static output. The
repository's rule is `/* /index.html 200`.

## Deployment ownership

The `1kbgz/terraform` repository provisions the `linerack` Pages project. The
LineRack build workflow uploads its already-tested `dist` artifact after both
web and firmware jobs pass on `main`. Cloudflare Git integration is not used;
deployments use Wrangler Direct Upload so infrastructure and application
delivery have one owner each.

Configure these GitHub Actions values on `1kbgz/linerack`:

| Kind | Name | Value |
| --- | --- | --- |
| Repository variable | `CLOUDFLARE_ACCOUNT_ID` | Account containing the `linerack` Pages project |
| Repository secret | `CLOUDFLARE_API_TOKEN` | Scoped token with Account / Cloudflare Pages / Edit |

The deploy job stays skipped until `CLOUDFLARE_ACCOUNT_ID` is set. Terraform
owns the `linerack.dev` DNS record and Pages custom domain. Apply the Terraform
project before relying on the custom URL. Do not create a second Git-integrated
Pages project in the Cloudflare dashboard.

## Repository automation

Template maintenance uses the Python Project Templates Copier Update GitHub
App. The app discovers `.copier-answers.yaml`; no per-repository update workflow
or token is needed. Pull request automerge uses the Python Templates Automerge
GitHub App and remains opt-in through its labels. Neither app is part of the
site deployment path.

## Browser and transport boundary

The static product and documentation pages work in current standards-based
browsers. Hardware configuration additionally requires:

- desktop Chromium with WebHID;
- a secure HTTPS origin, except for browser-recognized local development;
- explicit user authorization for the LineRack HID device.

Playback and stored preset selection do not depend on the hosted site after a
setup is written to the device. iPhone playback is supported by USB Audio;
mobile Safari configuration is not part of the beta.

## External service boundary

The application has no analytics, waitlist, payment, account, email, or remote
preset service. The product page links to GitHub for development updates and
states that preorders are not open.

## Platform references

- [Cloudflare Pages Vite deployment](https://developers.cloudflare.com/pages/framework-guides/deploy-a-vite3-project/)
- [Cloudflare Pages Direct Upload with CI](https://developers.cloudflare.com/pages/how-to/use-direct-upload-with-continuous-integration/)
- [Cloudflare Pages redirects](https://developers.cloudflare.com/pages/configuration/redirects/)
