# Security Policy

## Reporting a Vulnerability

Please do not open an issue to report security issues.

To report a vulnerability email to dev@featherwallet.org

The following keys may be used to communicate sensitive information to developers:

| Name    | Fingerprint                                       |
|---------|---------------------------------------------------|
| tobtoht | E87B D921 CDD8 85C9 D78A 38C5 E45B 10DD 027D 2472 |

Public keys can be found in [`utils/pubkeys`](utils/pubkeys).

## Bug Bounty Program

A bounty may be rewarded to a vulnerability report **if and only if** the issue can result in **a loss of funds**.

You must describe a **plausible scenario** in which a loss of funds can occur (or has occurred) that **isn't solely attributable to user error**.

Only **the code** of the **latest tagged release** of **[this repository](https://github.com/feather-wallet/feather/)** is in scope.

**The bounty can only be rewarded in XMR**. The bounty amount for your report is determined by the maintainers and ranges from USD 150 to USD 3000 (in terms of XMR) and depends on the severity of the issue and other factors.

Clarifications on scope:

- The issue **must be present in a [signed](https://docs.featherwallet.org/guides/release-signing-key) [release build](https://github.com/feather-wallet/feather/blob/master/contrib/guix/README.md)**. Custom builds, including distribution packages, are out of scope.
- The developers **must be able to reproduce and fix the issue**. If the issue cannot be fixed **in our code** for any reason, it is out of scope.
- The live [websites](https://github.com/feather-wallet/feather-site/blob/master/mirrors.txt) and their repositories are out of scope.
- Loss of funds due to malware on the user's machine is out of scope.
- Memory imaging, including cold boot attacks, is out of scope.
- Social engineering against users is out of scope.
- Any form of coercion, physical or psychological, is out of scope.
- Vulnerabilities that are attributable to hardware are out of scope.
- If the issue was fixed in the `master` branch before we receive your report, it is invalid and not eligible for a bounty from this program.
- If the vulnerability involves binary exploitation, we may ask you to provide a proof of concept of secret key exfiltration.
- Vulnerabilities that are present in the monero submodule but were not introduced in patches made by the Feather developers must
  be reported [upstream](https://github.com/monero-project/meta/blob/master/VULNERABILITY_RESPONSE_PROCESS.md) and are not eligible for a bounty from this program.
- Vulnerabilities that are present in any of our third-party dependencies must be reported upstream and are not eligible for a bounty from this program.
- Vulnerabilities that are present in [supported hardware wallets](https://docs.featherwallet.org/guides/hardware-wallet-support) must be reported upstream and are not eligible for a bounty from this program.
- A bounty will not be awarded if the reported vulnerability was already known. We may make an exception if you demonstrate that the severity of the issue was underestimated and no immediate fix was planned.
- If, during your research, you disrupt Feather's release infrastructure or services, or attempt to coerce its developers, you will not be awarded a bounty.
