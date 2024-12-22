#include "MaxFlowSolver.hpp"
#include <chrono>

using namespace std;
using namespace chrono;

int main(int argc, char *argv[])
{
    if (argc < 2) {
        cout << "Please provide the path to the input file." << endl;
        return 1;
    }
    
    MaxFlowSolver solver = MaxFlowSolver(argv[1]);
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
