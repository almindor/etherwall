# etherwall

Ethereum QT5 Wallet

Etherwall is a free software wallet/front-end for Ethereum.

## Usage

Latest geth is required to be running for Etherwall to work. Geth is provided if downloaded from the main website for windows and mac os x.

Default geth path on linux points to `/usr/bin/geth`

## License

Etherwall is licensed under the GPLv3 license. See LICENSE for more info.

## Donations

#### Flattr
[![Flattr this git repo](http://api.flattr.com/button/flattr-badge-large.png)](https://flattr.com/submit/auto?user_id=Almindor&url=https://github.com/almindor/etherwall&title=Etherwall&language=&tags=github&category=software)

#### Bitcoin
`1NcJoao879C1pSKvFqnUKD6wKtFpCMppP6`

#### Litecoin
`LcTfGmqpXCiG7UikBDTa4ZiJMS5cRxSXHm`

#### Ether
`0xc64b50db57c0362e27a32b65bd29363f29fdfa59`

## Development

### General Requirements

[Latest Geth](https://github.com/ethereum/go-ethereum/releases)

[Qt5.6+ with qmake](https://www.qt.io/developers/)

[google protobuf](https://github.com/google/protobuf)

[hidapi](https://github.com/signal11/hidapi)

### Building

Udev is required in Linux and mingw needs to be used on Windows to compile the required libraries.
The project is set to use static (.a) files on Windows and Mac OS X with absolute paths.
You need to update the paths in the `Etherwall.pro` file to point to your compiled libraries.

qmake -config release && make

### Roadmap

#### TODO

- 2.0 add "remote IPC" node support (think light client)

#### DONE

- 1.6 add TREZOR support
- 1.4 add contract deployment
- 1.3 added contract support [invoking and watches]
- 0.9 add transaction history support [done]
- 0.8 initial release [done]

### Caveats & bugs

Only supported client at the moment is Geth.

If etherwall freezes with TREZOR inserted just remove TREZOR and restart Etherwall. Then insert TREZOR in again. This happens from time to time on Linux, probably a bug in hidapi.
