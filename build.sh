#!/bin/bash

g++ -I include -D DEBUG_ARP -D DEBUG_IP -D DEBUG_ETH -w source.cpp arp.cpp skbuff.cpp netdev.cpp ip_input.cpp icmp.cpp utils.cpp ip_output.cpp dst.cpp route.cpp tuntap_if.cpp socket.cpp sock.cpp inet.cpp raw_socket.cpp -o ./bin/source
