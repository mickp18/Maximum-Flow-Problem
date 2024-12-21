// Header file that contains the Maximum Flow Graph algorithm
// Author(s): Mick Perseo & Gio Silve & M.N.

#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <fstream>

#include "Edge.hpp"

using namespace std;

class MaxFlowSolver
{
private:
    // INPUTS
    //  number of nodes
    int n;

    // file name
    string input_file_path;

    // maximum flow value to compute
    long max_flow;

    // graph (adjacency list)
    vector<list<Edge *>> graph;

    //
    int visitFlag = 1;
    vector<int> visited;

    /* Indicates whether the network flow algorithm has ran.
    We would not need to run the solver multiple times, because it always yields the same result. */
    bool solved;

    // to avoid overflow
    const long INF = __LONG_LONG_MAX__ / 2;

public:
    // constructor
    // MaxFlowSolver(string input_file_path, int n) {
    MaxFlowSolver(string input_file_path)
    {
        this->input_file_path = input_file_path;
        // this->n = n;
        this->max_flow = 0;
        this->solved = false;
        this->graph = read_graph();
        this->visited = vector<int>(this->n);
    }

    // read the graph and save it into an adjacency list
    vector<list<Edge *>> read_graph(){
    
        ifstream file(this->input_file_path);

        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << this->input_file_path << std::endl;
            return graph; // Return an empty list
        }

        char *end;
        string line;
        getline(file, line);
        this->n = strtol(line.c_str(), &end, 10);
        //cout << "Number of nodes: " << this->n << endl;

        vector<list<Edge *>> graph(this->n);

        while (getline(file, line)) {
            // assuming format "node1 node2 capacity"
            int start_node = strtol(line.c_str(), &end, 10);
            int end_node = strtol(end + 1, &end, 10);
            long capacity = strtol(end + 1, &end, 10);

            Edge *edge = new Edge(start_node, end_node, capacity);
            Edge *edge_residual = new Edge(end_node, start_node, 0);
            
            edge->setResidual(edge_residual); // set the residual edge 
            edge_residual->setResidual(edge); // set the foward edge as "residual" of the residual edge
            //cout << "adding to Node " << start_node << endl;
            graph[start_node].push_back(edge);
            graph[end_node].push_back(edge_residual);
        }
        return graph;
    }


    // print the graph in format edge - edge, capacity
    void print_graph() {
        for (auto node : this->graph) {
            for (auto edge : node) {
                cout << edge->to_string() << endl;
            }
        }
    }


    void setVisited(int i) {
        this->visited[i] = this->visitFlag;
        return;
    }

    // Returns whether or not node 'i' has been visited.
    bool isVisited(int i) {
        return this->visited[i] == this->visitFlag;
    }

};
