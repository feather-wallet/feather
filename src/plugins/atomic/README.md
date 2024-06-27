### Atomic Plugin
Built in xmr-btc atomic swap

## Installation

## Hacking
sudo apt install libarchive-dev

It may also be helpful to download my fork of the comit network's xmr-btc swap tool as it has everything you need to use btc's testnet4 for swaps. Testnet4 btc is much easier to acquire so it makes testing much more practical. There are scripts to run the electrum server just make sure you have docker-compose installed and build the electrumx testnet4 fork before trying to run the scripts.

[xmr-btc-swap-json-tests](https://github.com/BrandyJSon/xmr-btc-swap-json-tests/tree/master)

Functions used to control swap (AtomicPlugin.cpp)

withdraw(btcaddress) - withdraw btc 
list(rendezvous point) - list sellers at rendezvous
 
## Usage
Navigate to the Atomic tab after opening your wallet. Click the configure button.

// Config Settings
