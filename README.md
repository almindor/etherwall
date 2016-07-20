# etherwall

Ethereum QT5 Wallet

Etherwall is a free software wallet/front-end for Ethereum.

## Usage

Latest geth is required to be running for Etherwall to work. Geth is provided if downloaded from the main
website for windows and mac os x.

Etherwall should auto-detect geth's IPC file/name and work "out of the box" as long as geth is running.

If Etherwall fails to detect the IPC file/name you can specify it in the settings panel.

Ethwerwall will show geth's syncing progress and only process blocks after it's done.

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

Latest Geth (provided if downloaded off site)

Qt5.5+ with qmake

### Building

qmake -config release && make

### Roadmap

- 2.0 added contract support
- 1.1+ add eth support (cancelled)
- 1.0 add geth account backup and restore (skipped for now)
- 0.9 add transaction history support [done]
- 0.8 initial release [done]

### Caveats & bugs

Only supported client at the moment is Geth. Eth and others should work if you go to settings and set the IPC path/name properly.
