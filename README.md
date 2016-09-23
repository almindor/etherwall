# etherwall

Ethereum Classic QT5 Wallet

Etherwall is a free software wallet/front-end for Ethereum Classic  .

## Usage

Latest geth is required to be running for Etherwall to work. Geth is provided if downloaded from the main Ehereum Classic
website for windows and mac os x.

Etherwall should auto-detect geth's IPC file/name and work "out of the box" as long as geth is running.

If Etherwall fails to detect the IPC file/name you can specify it in the settings panel.

Ethwerwall will show geth's syncing progress and only process blocks after it's done.

## License

Etherwall is licensed under the GPLv3 license. See LICENSE for more info.

## Donations

#### Bitcoin
`3DPWYmUaymGoTYkvx5gHztPVANJau4unh1`

#### Litecoin
`LaMQ5F8MuxCZFwrhR51mTm3zoc7brDzVeV`

#### Ether Classic
`0xe290b5cb85d66afc1c2ea8800b7caab55a5a0006`

## Development

### Requirements

Latest Geth (provided if downloaded off site)

Qt5.5+ with qmake

### Building

qmake -config release && make
Tested with Ubuntu 14.04 LTS

### Roadmap

-Integrate with other clients out of the box
-Alingn UI to match ETC 
-Translate to multiple languages

### Caveats & bugs

Only supported client at the moment is Geth. Eth and others should work if you go to settings and set the IPC path/name properly.

