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

### Requirements

Latest Geth

Qt5.5+ with qmake

### Building

qmake -config release && make

### Roadmap

- 2.0 use parity as back-end with "internal" transaction watching
- 1.4 add contract deployment
- 1.3 added contract support [invoking and watches]
- 0.9 add transaction history support [done]
- 0.8 initial release [done]

### Caveats & bugs

Only supported client at the moment is Geth.
