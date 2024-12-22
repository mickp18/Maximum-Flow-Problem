#!/bin/bash


# Get the directory where the script is located
SCRIPT_DIR=$(dirname $(realpath ${BASH_SOURCE[0]}))

# Get the paths of the program & the exe
SOLVER=$SCRIPT_DIR/MaxFlowSolver.hpp
EDGE=$SCRIPT_DIR/Edge.hpp
MAIN=$SCRIPT_DIR/main.cpp
PROG=$SCRIPT_DIR/MaxFlowSolver


# Compile the program
g++ $MAIN $SOLVER $EDGE  -o $PROG

# Run the program
$PROG $1


# Create graph with Graphviz

g++ -o graphviz  graphViz.cpp  -lgvc -lcgraph

./graphviz

# xdg-open graph.png



