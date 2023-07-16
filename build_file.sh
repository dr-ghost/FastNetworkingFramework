#!/bin/bash

g++ -I include -I ~/Documents/boost_1_82_0 -pthread  -w server_node.cpp listener.cpp broadcaster.cpp UDPStream.cpp -o ./bin/server_node
g++ -I include -I ~/Documents/boost_1_82_0 -pthread -w client_node.cpp listener.cpp broadcaster.cpp UDPStream.cpp -o ./bin/client_node