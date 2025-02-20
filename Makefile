build:
	@./contrib/guix/guix-build

attest:
	@./contrib/guix/guix-attest

verify:
	@./contrib/guix/guix-verify

clean:
	@./contrib/guix/guix-clean

DEFAULT_GOAL := default
default: build

.PHONY: default build attest verify clean
