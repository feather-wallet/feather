## Maintenance priority

This document is written for developers and users interested in learning how Feather is developed.

### 1. Security

- Fix security issues and privacy leaks affecting Feather
  - Note: If you believe to have found a vulnerability, please refer to [SECURITY.md](SECURITY.md)
- Rebase the Monero submodule on top of the latest `monero-project/monero` tag
  - Monero releases may contain undisclosed security fixes
- Update or patch statically linked dependencies that have known vulnerabilities
  - Run `feather-utils/depends/vulns.py` to check
  - Review the diff of any altered package to mitigate the risk of supply chain attacks
- Update compilers and security flags for better binary security
- Reduce the number of third-party dependencies
- Keep the [website](https://github.com/feather-wallet/feather-site/blob/master/mirrors.txt) VPS up-to-date and secure
- Contact relevant authorities to take [phishing sites](https://gist.github.com/tobtoht/4039fa3cf922d4fe8bca2f8e3ddac63b) offline
- Make improvements to the [release process](RELEASE.md)

Goals:

- Set up a bug bounty program for issues that affect privacy or security
- Set up a status page with information about project health
- Set up a feed for security bulletins
- Sandbox components that handle untrusted input (e.g. QR code scanner)
- Create a package manager for secure distribution of portable binaries
- `-static-pie` release binaries for Linux targets

Security issues that affect Feather always warrant a new release as soon as possible.

### 2. Reproducibility

- Improve and maintain tools to check for non-determinism
- Ensure releases are reproducible and stay that way
- Upload source archives to the fallback mirror

To learn more about Feather's build system, see: [`contrib/guix/README.md`](https://github.com/feather-wallet/feather/blob/master/contrib/guix/README.md)

[Bootstrappable builds](https://bootstrappable.org/) are a requirement for all release builds since version 2.2.2. 
Our Guix time-machine is currently pinned at a commit which implements the 
[Full-Source Bootstrap](https://guix.gnu.org/en/blog/2023/the-full-source-bootstrap-building-from-source-all-the-way-down/).

### 3. Bugs

- Fix bugs and crashes

To report a bug, please see: https://docs.featherwallet.org/guides/report-an-issue

### 4. Tests

- Improve test coverage
- Write more test cases

Feather does not currently have a test suite (apart from the tests in the Monero submodule), this is a WIP.

### 5. Documentation

- Make sure the documentation accurately reflects the latest release
- Add troubleshooting guides for common problems
  - Ideally, most support questions can be answered with a link to the documentation

Goals:
- Reconsider and document default settings
- Write a document on threat modeling

Documentation is available at https://docs.featherwallet.org

### 6. Improvements

- Improve existing features
- Improve UI/UX

Feather should first and foremost be a good __wallet__.
Improving features that are closer to this end should have priority.

### 7. Platform Support

- Add support for more architectures and operating systems
- Drop support for End-of-Life distributions
- Add support for new hardware wallets

See: https://docs.featherwallet.org/guides/supported-operating-systems

### 8. Optimization, cleanup and continuity

Miscellaneous maintenance tasks.

- Remove dead code
- Fix compiler warnings
- Optimize release binary size
- Speed up the [release process](RELEASE.md)
- Automate recurrent maintenance tasks
- Refactor code that is in need of refactoring
- Add comments to the code where necessary
- Reduce complexity in the codebase where possible
- Improve documentation for developers and maintainers
- Keep the build system, toolchain and dependencies modern
- Remove features if their maintenance burden outweighs their usefulness

Goals:

- Make sure Feather is ready for the migration to [Seraphis](https://github.com/seraphis-migration/wallet3)

### 9. Features

- Implement new features
  - Allow Feather to be used or configured for higher, esoteric or new threat models
  - Add experimental features that may later be adopted in the reference wallets
  - Add features that are generally useful and relevant

Every added feature increases the amount of work needed to maintain Feather. Consider the usefulness of a feature 
compared to its expected maintenance and support burden.

For a non-exhaustive list of potentially new features, see: https://featherwallet.org/ideas

### 10. Upstreaming

- Upstream tried and tested features, bugfixes and useful patches
  - Bugfixes should be upstreamed without delay

Goals:

- Upstream polyseed
- Upstream bootstrappable builds using Guix as a replacement for the now deprecated Gitian build system
