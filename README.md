# etherwall

Ethereum QT5 Wallet

## License

Etherwall is licensed under the GPLv3 license. See LICENSE for more info.

## Donations

### Flattr
[![Flattr this git repo](http://api.flattr.com/button/flattr-badge-large.png)](https://flattr.com/submit/auto?user_id=Almindor&url=https://github.com/almindor/etherwall&title=Etherwall&language=&tags=github&category=software)

### Bitcoin
1NcJoao879C1pSKvFqnUKD6wKtFpCMppP6

### Litecoin
LcTfGmqpXCiG7UikBDTa4ZiJMS5cRxSXHm

### Ether
0xc64b50db57c0362e27a32b65bd29363f29fdfa59

## Development

### Requirements

Geth 1.0.0+ (eth and others might be supported)
Qt5.2+ with qmake

### Building

qmake && make

### Roadmap

- 1.1+ add eth support
- 1.0 add geth account backup and restore
- 0.9 add transaction history support
- 0.8 initial release [done]

### Caveats & bugs

Only supported client at the moment is Geth. Eth and others should work if you go to settings and set the IPC path/name properly.  
Currently doesn't support getting transaction history. If someone knows how to get it for an account from block #0 I'm all ears.  
Do not run while geth is synchronizing a big number of blocks, might slow down to a crawl.  
