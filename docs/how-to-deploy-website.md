# How to deploy the website

Deploy the tested static site to the production Cloudflare Pages project using
the repository workflow.

## Prerequisites

- A checkout prepared through [development setup](how-to-set-up-development.md)
- Playwright Chromium installed with `pnpm exec playwright install chromium`
- The `linerack` Pages project and `linerack.dev` domain provisioned by the
  `1kbgz/terraform` repository
- Repository variable `CLOUDFLARE_ACCOUNT_ID`
- Repository secret `CLOUDFLARE_API_TOKEN` with Cloudflare Pages edit access
- A pull request whose web and firmware checks pass

Store credential values only in the GitHub repository settings or the approved
infrastructure secret store. Never put values in documentation, source,
workflow arguments, pull requests, or logs.

## Validate the site artifact

Run the same repository gates required by CI:

```sh
make lint
make check
make test-web
make build-web
```

The static site should exist under `dist`. CI separately tests and builds
firmware before allowing production deployment.

## Deploy from `main`

Merge the reviewed change into `main`. `.github/workflows/build.yaml` runs the
web and firmware jobs, downloads the tested site artifact, and sends `dist` to
the existing `linerack` Pages project through Wrangler Direct Upload.

The deploy job runs only when both build jobs pass, the ref is `main`, and
`CLOUDFLARE_ACCOUNT_ID` is configured. A protected `production` GitHub
environment may also require approval. Pull requests and other branches do not
deploy.

To repeat a deployment without changing source, dispatch **Build Status** on
the `main` branch from GitHub Actions.

## Verify production

After the deploy job passes, check:

1. `https://linerack.dev/`
2. `https://linerack.dev/configure`
3. `https://linerack.dev/docs`
4. One direct documentation deep link

Confirm the landing page loads, the configurator enters simulator mode without
hardware, the developer-doc index contains every section, and a deep-link
refresh returns the requested page instead of a 404.

## Troubleshoot a skipped deploy

Check the workflow ref, the `CLOUDFLARE_ACCOUNT_ID` repository variable, and
whether the `production` environment is waiting for approval. A missing
variable intentionally skips deployment. Do not print the API token while
diagnosing credentials.

See [Web deployment reference](deployment-reference.md) for routes, workflow
ownership, Pages settings, and credential names.
