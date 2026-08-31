# How to update from the repository template

Use this guide to apply changes from `python-project-templates/base` without
losing LineRack-specific web, firmware, and deployment behavior.

## Prerequisites

- A clean working tree on a dedicated branch
- Copier installed
- Access to `python-project-templates/base`
- The repository's web and firmware toolchains

## Apply the template update

Run Copier from the repository root:

```sh
copier update --trust --answers-file .copier-answers.yaml
```

Review every generated change before accepting it. Preserve these LineRack
customizations:

- Firmware build and test jobs
- WebHID dependencies and browser tests
- Cloudflare Pages deployment settings
- Ignored firmware dependencies and build outputs
- The PolyForm Noncommercial license

Do not accept a generated version change unless the update requires one.

## Verify the result

Run the repository checks:

```sh
make lint
make check
make test
make build
```

Inspect the final diff for unrelated generated changes. Commit the template
update only after all checks pass.

## Related documentation

- [Set up LineRack development](how-to-set-up-development.md)
- [Web deployment reference](deployment-reference.md)
