# sitara-websocketpp

A basic websocket library that can handle multiple connections to/from each Client or Server.  Architecture derived from the official documentations [utility](https://github.com/zaphoyd/websocketpp/tree/master/tutorials/utility_client)
[classes](https://github.com/zaphoyd/websocketpp/tree/master/tutorials/utility_server).

## Requirements
* websocketpp
* openssl
* asio

All requirements are added as submodules to the project.  To download them, run:

```
git submodule update --init --recursive
```

## Usage

## Examples
* `exampleServer` is a basic server.
* `exampleClient` is a basic client.

## Contributing
This project uses the [Git Flow](http://nvie.com/posts/a-successful-git-branching-model/) paradigm.  Before contributing, please make your own feature branch with your changes.

## More Information
* [Websocket++](https://www.zaphoyd.com/websocketpp/)
