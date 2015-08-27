# etherwall

Ethereum QT5 Wallet

Etherwall is a free software wallet/front-end for Ethereum Geth*.

## Usage

Geth 1.0.2+ is required to be running for Etherwall to work.

Etherwall should auto-detect geth's IPC file/name and work "out of the box" as long as geth is running.

If Etherwall fails to detect the IPC file/name you can specify it in the settings panel.

Do not run Ethwerwall while geth is syncing, it will just lock down processing all the blocks until syncing is done.

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

Geth 1.0.2+ (eth and others might be supported)

Qt5.2+ with qmake

### Building

qmake -config release && make

### Roadmap

- 1.1+ add eth support
- 1.0 add geth account backup and restore
- 0.9 add transaction history support [done]
- 0.8 initial release [done]

### Caveats & bugs

Only supported client at the moment is Geth. Eth and others should work if you go to settings and set the IPC path/name properly.
