#!/bin/bash


# Get the directory where the script is located
SCRIPT_DIR=$(dirname $(realpath ${BASH_SOURCE[0]}))
PARENT_DIR=$(dirname $SCRIPT_DIR) # Get parent directory of SCRIPT_DIR


# Get the paths of the program & the exe
SOLVER=$PARENT_DIR/src/MaxFlowSolver.hpp
EDGE=$PARENT_DIR/src/Edge.hpp
MAIN=$PARENT_DIR/src/main.cpp
PROG=$PARENT_DIR/src/MaxFlowSolver

# Compile the program
g++ $MAIN $SOLVER $EDGE -o $PROG


# Run the solver
$PROG $PARENT_DIR/inputs/input3.txt $PARENT_DIR/outputs/output.txt
echo "Graph saved to $PARENT_DIR/outputs/output.txt"


# Create graph with Graphviz
g++ $PARENT_DIR/viz/graphViz.cpp -o $PARENT_DIR/viz/graphviz  -lgvc -lcgraph -w


# Run Graphviz using result file of the solver
$PARENT_DIR/viz/graphviz $PARENT_DIR/outputs/output.txt



#xdg-open graph.png
#...