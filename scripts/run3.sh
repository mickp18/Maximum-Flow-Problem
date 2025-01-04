SCRIPT_DIR=$(dirname $(realpath ${BASH_SOURCE[0]}))
PARENT_DIR=$(dirname $SCRIPT_DIR) # Get parent directory of SCRIPT_DIR


# Get the paths of the program & the exe
SOLVER=$PARENT_DIR/src/MaxFlowSolverParallel.hpp
EDGE=$PARENT_DIR/src/Edge.hpp
NODE=$PARENT_DIR/src/Node.hpp
MAIN=$PARENT_DIR/src/main_pr.cpp
PROG=$PARENT_DIR/src/MaxFlowSolverP

# Compile the program
g++ $MAIN $SOLVER $NODE $EDGE -o $PROG

# $PROG $PARENT_DIR/inputs/first_test.txt $PARENT_DIR/outputs/second_test.txt
$PROG $PARENT_DIR/inputs/input_baby.txt $PARENT_DIR/outputs/second_test.txt
# compile sequetial version
# g++ main.cpp Edge.hpp MaxFlowSolver.hpp -o sequentialSolver