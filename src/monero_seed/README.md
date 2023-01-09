## Note to package maintainers:

This library was vendored because it is **deprecated** and **unmaintained**, it is kept for legacy reasons only.
Feather is the only project that will ever use this code, thus it is considered part of Feather source.
Do NOT unvendor this code.

---

## Features

* embedded wallet birthday to optimize restoring from the seed (only blocks after the wallet birthday have to be scanned for transactions)
* 5 bits reserved for future updates
* advanced checksum based on Reed-Solomon linear code, which allows certain types of errors to be detected without false positives and provides limited error correction capability
* built-in way to make seeds incompatible between different coins, e.g. a seed for Aeon cannot be accidentally used to restore a Monero wallet

## Implementation details

The mnemonic phrase contains 154 bits of data, which are used as follows:

* 5 bits reserved for future use
* 10 bits for approximate wallet birthday
* 128 bits for the private key seed
* 11 bits for checksum

### Wordlist

The mnemonic phrase uses the BIP-39 wordlist, which has 2048 words, allowing 11 bits to be stored in each word. It has some additional useful properties,
for example each word can be uniquely identified by its first 4 characters. The wordlist is available for 9 languages (this repository only uses the English list).

### Reserved bits

There are 5 reserved bits for future use. Possible use cases for the reserved bits include:

* a flag to differentiate between normal and "short" address format (with view key equal to the spend key)
* different KDF algorithms for generating the private key
* seed encrypted with a passphrase

Backwards compatibility is achieved under these two conditions:

1. Reserved (unused) bits are required to be 0. The software should return an error otherwise.
2. When defining a new feature bit, 0 should be the previous behavior.

### Wallet birthday

The mnemonic phrase stores the approximate date when the wallet was created. This allows the seed to be generated offline without access to the blockchain. Wallet software can easily convert a date to the corresponding block height when restoring a seed.

The wallet birthday has a resolution of 2629746 seconds (1/12 of the average Gregorian year). All dates between June 2020 and September 2105 can be represented.

### Private key seed

The private key is derived from the 128-bit seed using PBKDF2-HMAC-SHA256 with 4096 iterations.The wallet birthday and the 5 reserved/feature bits are used as a salt. 128-bit seed provides the same level of security as the elliptic curve used by Monero.

Future extensions may define other KDFs.

### Checksum

The mnemonic phrase can be treated as a polynomial over GF(2048), which allows us to use an efficient Reed-Solomon error correction code with one check word. All single-word errors can be detected and all single-word erasures can be corrected without false positives.

To prevent the seed from being accidentally used with a different cryptocurrency, a coin-specific value is subtracted from the first data-word after the checksum is calculated. Checksum validation will fail unless the wallet software adds the same value back to the first data-word when restoring.
