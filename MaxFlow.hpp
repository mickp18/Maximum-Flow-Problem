// Header file that contains the Maximum Flow Graph algorithm
// Author: Michele Perseo

#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

class MaxFlowSolver
{

public:

    class Edge
    {
        // start node
        int start_node;

        // end node
        int end_node;

        // flow
        long flow;

        // capacity
        static long capacity;

        // residual edge
        Edge *residual;

        Edge(int start_node, int end_node, long capacity)
        {
            this->start_node = start_node;
            this->end_node = end_node;
            this->capacity = capacity;
            this->flow = 0;
        }

        bool is_residual()
        {
            return this->capacity == 0;
        }

        long capacity_available()
        {
            return this->capacity - this->flow;
        }

        void augment(long bottleneck)
        {
            this->flow += bottleneck;
            residual->flow -= bottleneck;
        }
        string to_string()
        {

            // TODO
        }
    };

private:
    // INPUTS
    //  number of nodes

    int n;

    string input_file_path;

    long max_flow;

    // graph (adjacency list)
    list<Edge *> graph;

    vector<int> visited;

    // Indicates whether the network flow algorithm has ran. We should not need to
    // run the solver multiple times, because it always yields the same result.

    bool solved;

    // to avoid overflow
    const long INF = __LONG_LONG_MAX__ / 2;

    MaxFlowSolver(string input_file_path, int n)
    {
        this->input_file_path = input_file_path;
        this->n = n;
        this->max_flow = 0;
        this->solved = false;
        this->visited = vector<int>(n);
    }

protected:
    list<Edge *> read_graph()
    {
        list<Edge *> graph;
        ifstream file(input_file_path);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << input_file_path << std::endl;
            return graph; // Return an empty list
        }

        string line;

        while (getline(file, line))
        {
            // assuming fomrat node1,node2,capacity
            char* end; // Temporary buffer for parsing numbers

            int start_node = strtol(line.c_str(), &end, 10);
            int end_node = strtol(end + 1, &end, 10);
            long capacity = strtol(end + 1, &end, 10);
            

            Edge *edge = new Edge(start_node, end_node, capacity);

            graph.push_back(edge);
            

        }
    }
};
