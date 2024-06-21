### Atomic Plugin
Built in xmr-btc atomic swap

## Installation

## Hacking
sudo apt install libarchive-dev

It may also be helpful to download my fork of the comit network's xmr-btc swap tool as it has tests set to ouput JSON and scripts that allow for easy simulation of running swap (filters to Bob POV & removes wrappers on normal output)

[xmr-btc-swap-json-tests](https://github.com/BrandyJSon/xmr-btc-swap-json-tests/tree/master)

Functions used to control swap (AtomicPlugin.cpp)

withdraw(btcaddress) - withdraw btc 
list(rendezvous point) - list sellers at rendezvous
 
## Usage
Navigate to the Atomic tab after opening your wallet. Click the configure button.

// Config Settings
