
# compile parallel version
g++ main_pr.cpp MaxFlowSolverParallel.hpp Edge.hpp -o parallelSolver

# compile sequetial version
g++ main.cpp Edge.hpp MaxFlowSolver.hpp -o sequentialSolver