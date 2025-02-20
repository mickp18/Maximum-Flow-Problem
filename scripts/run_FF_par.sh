SCRIPT_DIR=$(dirname $(realpath ${BASH_SOURCE[0]}))
PARENT_DIR=$(dirname $SCRIPT_DIR) # Get parent directory of SCRIPT_DIR


# Get the paths of the program & the exe
SOLVER=$PARENT_DIR/src/MaxFlowSolverParallelPool.hpp
SOLVER_1=$PARENT_DIR/src/MaxFlowSolverParallel.hpp
EDGE=$PARENT_DIR/src/Edge.hpp
NODE=$PARENT_DIR/src/Node.hpp
NODEFIRST=$PARENT_DIR/src/NodeFirst.hpp
MAIN=$PARENT_DIR/src/main_pr.cpp
PROG=$PARENT_DIR/src/MaxFlowSolverPP
PROG_1=$PARENT_DIR/src/MaxFlowSolverP
LOG=$PARENT_DIR/src/ThreadLogger.hpp
MON=$PARENT_DIR/src/ThreadMonitor.hpp

# Compile the program
#---------
### max flow solver parallel THREAD POOL ver
g++  $MAIN $SOLVER $NODE $EDGE $LOG $MON -o  $PROG -g
###
#---------
## max flow solver parallel MANY THREADS
#g++ -fsanitize=thread -pthread $MAIN $SOLVER_1 $NODEFIRST $EDGE $LOG $MON -o  $PROG_1 -g
###
#---------


# g++ -fsanitize=thread -pthread $MAIN $SOLVER $NODE $EDGE $LOG $MON -o  $PROG -g 


# RUN THE program
# $PROG $PARENT_DIR/inputs/first_test.txt $PARENT_DIR/outputs/second_test.txt
#---------
### run max flow solver parallel with THREAD POOL
$PROG $PARENT_DIR/inputs/input3.txt $PARENT_DIR/outputs/output.txt
###
#---------
### run max flow solver parallel with MANY THREADS
# $PROG_1 $PARENT_DIR/inputs/second_test.txt $PARENT_DIR/outputs/second_test.txt
#TSAN_OPTIONS=detect_deadlocks=1:second_deadlock_stack=1 $PROG_1 $PARENT_DIR/inputs/second_test.txt $PARENT_DIR/outputs/second_test.txt
###
#---------
# $PROG $PARENT_DIR/inputs/input_baby.txt $PARENT_DIR/outputs/second_test.txt
# compile sequetial version
# g++ main.cpp Edge.hpp MaxFlowSolver.hpp -o sequentialSolver

echo "Graph saved to $PARENT_DIR/outputs/output.txt"

# Create graph with Graphviz
g++ $PARENT_DIR/viz/graphViz.cpp -o $PARENT_DIR/viz/graphviz  -lgvc -lcgraph -w


# Run Graphviz using result file of the solver
$PARENT_DIR/viz/graphviz $PARENT_DIR/outputs/output.txt
