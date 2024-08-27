# Feather Atomic

Feather Atomic is a plugin for the free, open-source Monero wallet for Linux, Tails, macOS and Windows, Feather. It is written in C++ with the Qt framework.

## Usage
Make sure to click configure button in the atomic tab and either auto install the swap binary or select the already installed binary.
All files relevant to swap will be stored in feathers config directory including the binary if you auto install.

### If after installing the swap tool, refresh fails to show any swaps make sure to check that you have a version of GLIBC greater or equal to 2.32 or this plugin will not be able to run on your system.

### Running a swap
1. Click refresh offers (may take a little if routing through tor)
2. Click the offer you would like to take from the table
3. Enter a BTC change address (make sure you are using a testnet address if feather is running stagenet mode)
4. Enter a XMR receive address
5. Click swap
6. Wait for fund dialog to appear, send at least the minimum BTC covering tx fees to the address and not more then the max BTC of the offer (if there is BTC from a previous uncompleted swap then this dialog may not appear)
7. Once the transaction is detected in the BTC mempool a different dialog will appear showing the status of confirmations for BTC and XMR side of swap
8. If all goes right after the BTC transaction reaches at least 1 confirmation and the XMR transaction reaches at least 10 confirmations the swap complete and both parties should be happy with their new coins

### Recovery
If for some reason the swap isn't able to be completed when ran, one party goes offline, application crashes, etc then a recovery dialog will automatically appear next time you launch feather atomic as long as the swap still can be resumed or canceled. 
#### To cancel a swap
1. Make sure swap has had enough BTC confirmations to be able to be canceled (should take about 12 hours, gui reflect if it can be canceled)
2. Click the swap you wish to cancel then click the cancel button
3. A dialog will spawn and then after a moment the dialog should update to say the swap has been canceled

#### To resume a swap
1. Click the swap you wish to resume then click the resume button
2. If the seller is still online then the normal swap dialog will appear
3. The confirmations may not always be accurate when resuming so rely on the status message in the dialog to follow progress of the swap.
4. As long as both parties stay online the swap should continue as normal and end with both parties having their new coins


### Testnet4 swap demo


https://github.com/user-attachments/assets/b7871a2f-21d0-46e2-b575-b9ba885a810d

#### This video has been speed up in multiple sections time on the top right is accurate to real life time.
## Resources

* Web: [featherwallet.org](https://featherwallet.org)
* Docs: [docs.featherwallet.org](https://docs.featherwallet.org)
* Git: [github.com/feather-wallet/feather](https://github.com/feather-wallet/feather)
* Reddit: [/r/FeatherWallet](https://old.reddit.com/r/FeatherWallet)
* Lemmy: [monero.town/c/featherwallet](https://monero.town/c/featherwallet)
* Mail: dev@featherwallet.org
* IRC: `#feather` on [OFTC](https://www.oftc.net/)
* Matrix: [matrix.to/#/#feather:monero.social](https://matrix.to/#/#feather:monero.social)


## Deterministic builds

See [contrib/guix/README.md](https://github.com/feather-wallet/feather/blob/master/contrib/guix/README.md) for more information.

## Development

If you are looking to set up a development environment for Feather, see [HACKING.md](https://github.com/feather-wallet/feather/blob/master/HACKING.md).

It is highly recommended that you join the `#feather` IRC channel on [OFTC](https://www.oftc.net/) or [`#feather:monero.social`](https://matrix.to/#/#feather:monero.social) on Matrix if you 
are hacking on Feather. Due to the nature of this open source software project, idling in this channel is the best 
way to stay updated on best practices and new developments.
