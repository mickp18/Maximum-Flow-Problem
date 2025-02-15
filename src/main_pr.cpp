#include "MaxFlowSolverParallel.hpp"
#include <chrono>

using namespace std;
using namespace chrono;

// prog_name inputFileName outputFileName
int main(int argc, char *argv[])
{
    ofstream fout;
    fout.open("../outputs/par_result.txt", ios_base::out | ios_base::app);

    if (argc < 3) {
        cout << "Please provide the path to the input file and the output file." << endl;
        return 1;
    }
    auto tot_time = 0;
    auto num_iterations = 5;
    for (int i = 0; i < num_iterations; i++) {
        MaxFlowSolverParallel solver = MaxFlowSolverParallel(argv[1]);
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        // chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
        solver.solve();
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        // chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(t2 - t1);

        cout << "Max flow: " << solver.getMaxFlow() << endl;
        cout << "found in: " << duration.count() << " micros" << endl;
        fout << "found in: " << duration.count() << " micros" << endl;
        cout << "\n";
        tot_time += duration.count();
    }

    cout << "\nAverage time: " << tot_time / num_iterations << " micros" << endl;
    

    // solver.printGraphToFile(argv[2]);
    
    return 0;
}
