# etherwall

Ethereum QT5 Wallet

Etherwall is a free software wallet/front-end for Ethereum.

## Donations

#### Flattr
[![Flattr this git repo](http://api.flattr.com/button/flattr-badge-large.png)](https://flattr.com/submit/auto?user_id=Almindor&url=https://github.com/almindor/etherwall&title=Etherwall&language=&tags=github&category=software) [![Join the chat at https://gitter.im/almindor/Lobby](https://badges.gitter.im/almindor/Lobby.svg)](https://gitter.im/almindor/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

#### Bitcoin
`1NcJoao879C1pSKvFqnUKD6wKtFpCMppP6`

#### Litecoin
`LcTfGmqpXCiG7UikBDTa4ZiJMS5cRxSXHm`

#### Ether
`0xC64B50dB57c0362e27A32b65Bd29363f29FDFa59`


## Usage

Latest geth is required to be running for Etherwall to work. Geth is provided if downloaded from the main website for windows and mac os x.

Default geth path on linux points to `/usr/bin/geth`

## License

Etherwall is licensed under the GPLv3 license. See LICENSE for more info.

## Development

### General Requirements

[Latest Geth](https://github.com/ethereum/go-ethereum/releases)

[Qt5.6+ with qmake](https://www.qt.io/developers/)

Qt5 modules:
* qt5-declarative
* qt5-graphicaleffects
* qt5-quickcontrols
* qt5-websockets

[google protobuf](https://github.com/google/protobuf)

[hidapi](https://github.com/signal11/hidapi)

#### Linux Requirements

Udev

#### Windows Requirements

Mingw
The project is set to use static (.a) files on Windows with absolute paths.
You need to update the paths in the `Etherwall.pro` file to point to your compiled libraries.

For Qt5 runtime/deployment see: [Qt5 for windows](http://doc.qt.io/qt-5/windows-deployment.html)

*NOTE:* there is no protobuf generation script on windows atm. You need to run `protoc --cpp_out` into `src/trezor/proto` for all the trezor protocol files manually or use a unix-environment to run the script (e.g. MinGW)

#### Mac OS X Requirements

The project is set to use static (.a) files on Mac OS X with absolute paths.
You need to update the paths in the `Etherwall.pro` file to point to your compiled libraries.
You need protobuf compiled with `CXXFLAGS=-mmacosx-version-min=10.9` to support older Mac versions (for releases)

For Qt5 runtime/deployment see: [Qt5 for macos](http://doc.qt.io/qt-5/osx.html#deploying-applications-on-macos)

### Building

```
git submodule init
git submodule update
./generate_protobuf.sh
qmake -config release && make
```

### Roadmap

#### TODO

- 2.1 improve UX

#### DONE

- 2.0 add "remote IPC" node support
- 1.6 add TREZOR support
- 1.4 add contract deployment
- 1.3 added contract support [invoking and watches]
- 0.9 add transaction history support [done]
- 0.8 initial release [done]

### Caveats & bugs

Only supported client at the moment is Geth.

If etherwall freezes with TREZOR inserted just remove TREZOR and restart Etherwall. Then insert TREZOR in again. This happens from time to time on Linux, probably a bug in hidapi.
