build:
	@./contrib/guix/guix-build

codesign:
	@./contrib/guix/guix-codesign

attest:
	@./contrib/guix/guix-attest

verify:
	@./contrib/guix/guix-attest

clean:
	@./contrib/guix/guix-clean

DEFAULT_GOAL := default
default: build

.PHONY: default build attest verify clean
