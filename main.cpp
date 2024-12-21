#include "MaxFlowSolver.hpp"

using namespace std;

int main(void)
{
    MaxFlowSolver solver = MaxFlowSolver("input.txt");
    solver.print_graph();
    
    return 0;
}
