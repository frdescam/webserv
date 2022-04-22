#!/bin/bash

# -std=c++98
g++ main.cpp config_checker.cpp server.cpp request_handler.cpp client_handler.cpp -fsanitize=address -g3 -D_debug_ && ./a.out
