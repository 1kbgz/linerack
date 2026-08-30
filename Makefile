.DEFAULT_GOAL := help

LIBDAISY_DIR ?= .deps/libDaisy
LIBDAISY_REV := cc146d5065dd8286078a662e2830bf820c37a612
LIBDAISY_URL := https://github.com/electro-smith/libDaisy.git

.PHONY: help develop develop-web develop-firmware lint fix check build build-web build-firmware test test-web test-firmware

help:
	@awk 'BEGIN {FS = ":.*## "} /^[a-zA-Z_-]+:.*## / {printf "%-20s %s\n", $$1, $$2}' $(MAKEFILE_LIST)

develop: develop-web develop-firmware ## Install web and firmware dependencies

develop-web: ## Install web dependencies
	pnpm install

develop-firmware: ## Fetch pinned libDaisy and build it
	@if [ ! -d "$(LIBDAISY_DIR)/.git" ]; then \
		mkdir -p .deps; \
		git clone --recursive "$(LIBDAISY_URL)" "$(LIBDAISY_DIR)"; \
	fi
	@if [ -n "$$(git -C "$(LIBDAISY_DIR)" status --porcelain)" ]; then \
		echo "$(LIBDAISY_DIR) has local changes; refusing to change revisions"; \
		exit 1; \
	fi
	@if [ "$$(git -C "$(LIBDAISY_DIR)" rev-parse HEAD)" != "$(LIBDAISY_REV)" ]; then \
		git -C "$(LIBDAISY_DIR)" fetch origin "$(LIBDAISY_REV)"; \
		git -C "$(LIBDAISY_DIR)" checkout --detach "$(LIBDAISY_REV)"; \
	fi
	git -C "$(LIBDAISY_DIR)" submodule update --init --recursive
	$(MAKE) -C "$(LIBDAISY_DIR)" -j4

lint: ## Check web lint and formatting
	pnpm lint

fix: ## Fix web lint and formatting
	pnpm fix

check: ## Type-check web source
	pnpm check

build: build-web build-firmware ## Build web app and target firmware

build-web: ## Build production web app
	pnpm build

build-firmware: develop-firmware ## Cross-build composite Seed3 firmware
	$(MAKE) -C firmware -f Makefile.usb-composite

test: test-web test-firmware ## Run web and native firmware tests

test-web: ## Run web unit and browser tests
	pnpm test

test-firmware: ## Run native firmware logic and descriptor tests
	$(MAKE) -C firmware/dsp test
	$(MAKE) -C firmware/usb test
