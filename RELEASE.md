# Release process

- Before a minor release:
  - Update `src/assets/restore_heights_monero_{mainnet,stagenet}.txt`
    - To obtain values, run `contrib/generate-restore-heights/heights.py`
  - Update default node lists in `src/assets/nodes.json`
  - Bump `openssl`, `qt`, `tor_*` packages in `contrib/depends/packages`
  - Update or patch any statically linked dependencies that have known vulnerabilities
    - Run `feather-utils/depends/vulns.py` to check
  - Rebase on top of latest Monero version
- Update `src/assets/ack.txt`
- Update documentation (`feather-wallet/feather-docs`) and `external/feather-docs` submodule
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
    ./contrib/guix/guix-build
    ```
  - Alternatively, run `feather-utils/guix/run-build.sh`
- Attest the builds (`contrib/guix/guix-attest`) and PR to `feather/feather-sigs`.
  - Wait until at least one other person has independently verified reproducibility
  - If builds are not reproducible: fix any reproducibility defects and bump patch version. Do not sign or release non-reproducible builds.
- Sign release artifacts and hashlists.
  - Transfer files in `guix-build-x.x.x/output` to release signing machine
  - Run `feather-utils/release/make-release.sh`
- Update the site (`feather-wallet/feather-site`)
  - Run the `feather-utils/site/bump-version.py` script to create a template commit.
  - Edit the changelog in `content/changelog`
    - If a release includes security fixes add "(includes security fixes)" to the title.
  - Update the version number, file sizes and paths in `data/release.json`
  - Upload releases, signatures and signed hashlists.
    - Follow the directory structure defined in `MainWindow::onShowUpdateCheck`.
  - Make `depends` source files [available](https://featherwallet.org/files/sources/):
    - `make -C contrib/depends download`
- Announce release on social media (irc/Matrix, Twitter, Lemmy)
