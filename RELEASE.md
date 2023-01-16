# Release process

- Update `src/assets/restore_heights_monero_{mainnet,stagenet}.txt` (before minor releases)
  - To obtain values, run `contrib/generate-restore-heights/heights.py`
- Update the version number in `CMakeLists.txt`
  - A hardfork-ready release must bump major version
- Create an annotated tag (`git tag x.x.x -a`)
  - Tag must match version in `CMakeLists.txt`
  - Only commits that update the version number in `CMakeLists.txt` may be tagged
- Push the master branch and tags
  - `git push --tags origin master`
- Run `guix` builds in a clean repo:
  - ```bash
    git clone https://github.com/feather-wallet/feather.git
    cd feather
    git checkout <TAG>
    git submodule update --init --recursive
    mkdir contrib/depends/SDKs
    tar -C contrib/depends/SDKs -xaf /path/to/Xcode-<foo>-<bar>-extracted-SDK-with-libcxx-headers.tar.gz
    ./contrib/guix/guix-build
    ```
  - To obtain `Xcode-<foo>-<bar>-extracted-SDK-with-libcxx-headers.tar.gz` follow the instructions in `contrib/macdeploy`.
  - Use at least two machines to verify that the builds are reproducible:
    ```bash
    cd guix-build-x.x.x/output
    find . -type f -not -name "SHA*" -exec sha256sum {} \; | sort -k2
    ```
    - In absence of a system for verified reproduction, at least one machine should be air-gapped.
    - If builds are not reproducible: fix any reproducibility defects and bump patch version. Do not sign or release non-reproducible builds.
     - To quickly identify any non-reproducible `depends` packages:
        ```bash
        cd contrib/depends/built
        find . -name "*.hash" -exec cat {} \; | sort -k2
        ```
- Sign release artifacts and hashlists.
  - Transfer files in `guix-build-x.x.x/output` to release signing machine
  - Run `make-release.sh`
- Update documentation (`feather-wallet/feather-docs`)
- Update the site (`feather-wallet/feather-site`)
  - Add a changelog in `content/changelog`
  - Update the version number, file sizes and paths in `data/release.json`
  - Upload releases, signatures and signed hashlists.
    - Follow the directory structure defined in `MainWindow::onShowUpdateCheck`.
  - Make `depends` source files [available](https://featherwallet.org/files/sources/):
    - `make -C contrib/depends download`
- Announce release on social media (Reddit, Twitter, irc/Matrix)
- Update websocket servers to notify clients of new release
  - Wait up to 7 days to allow for bug reports before major rollout