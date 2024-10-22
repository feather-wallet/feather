release:
	@./contrib/guix/guix-build

codesign:
	@./contrib/shell/guix-codesign.sh

attest:
	@./contrib/shell/guix-attest.sh

verify:
	@./contrib/shell/guix-verify.sh

clean:
	@./contrib/guix/guix-clean

DEFAULT_GOAL := default
default: release

.PHONY: default release attest verify clean
