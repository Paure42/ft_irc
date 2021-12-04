# Ft\_IRC

Ft_IRC is an IRC deamon, written in C++98. It was originally made as part of the 42cursus's ft_irc project.

It was made by three poeple :
- [Salaaad2](https://github.com/salaaad2)
- [Paure](https://github.com/paure42)
- [Ted](https://github.com/tbajrami)

# Building

``` sh
make -j
```

# running

``` sh
./ircserv <port> <password>
```

recommended port is 6667, but it is not set by default

# Features

As it is being worked on, the server only implements part of the official IRC RFC, but it is in a usable state nonetheless.

The client I recommend for usage with this server is [weechat](https://weechat.org)
