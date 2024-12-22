#include "MaxFlowSolver.hpp"
#include <chrono>

using namespace std;
using namespace chrono;

int main(void)
{
    MaxFlowSolver solver = MaxFlowSolver("input2.txt");
    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
    //high_resolution_clock::time_point t1 = high_resolution_clock::now();
    solver.solve();
    chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
    //high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(t2 - t1);

    cout << "Max flow: " << solver.getMaxFlow() << endl;
    cout << "found in: " <<  duration.count() << "ns" << endl;
    solver.printGraphToFile("result.txt");
    
    return 0;
}
