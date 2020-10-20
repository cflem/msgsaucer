# Saucer
Saucer is a decentralized UDP messenger which will run in the background and allow you to send/receive messages from other MSG Saucer users without passing them through a central server. The messenger will run in the background, storing up messages you receive while offline, then deliver them to the first terminal session you activate.

## Nickname Configuration
Saucer pulls your sender nickname from the file .nickname in your home directory. You can edit it with:
```shell
nano ~/.nickname
```

## Sending and Receiving Messages
Once saucer is compiled, it is split into two executables, saucer and sauce. To receive messages, you need only run saucer once per restart. You can also install it as a startup application to avoid this management. To send messages, you must run sauce. The usage is as such:
```shell
./sauce <destination ip[:port]> <message>
```

## Port Forwarding
Saucer can always be used over your local (LAN) network, however, to use it over the broader internet you must port forward port 2523 to your computer. You can also use workarounds such as VPNs, LogMeIn, etc., which may require sending to a different port number.
