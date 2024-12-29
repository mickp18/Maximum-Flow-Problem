// Header file that contains the Maximum Flow Graph algorithm
// Author(s): Mick Perseo & Gio Silve & M.N.

#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <fstream>
#include <future>
#include <mutex>
#include <thread>

#include "Edge.hpp"

using namespace std;

class MaxFlowSolver
{
private:
    // INPUTS
    //  number of nodes
    int n;

    // source = s , sink = t
    int s, t; 

    // file name
    string input_file_path;

    // maximum flow value to compute
    long max_flow=-1;

    // graph (adjacency list)
    vector<list<Edge *>> graph;

    //
    int visit_flag = 1;
    vector<int> visited;

    /* Indicates whether the network flow algorithm has ran.
    We would not need to run the solver multiple times, because it always yields the same result. */
    bool solved;

    // to avoid overflow
    const long INF = __LONG_LONG_MAX__ / 2;
    std::mutex graph_lock;


public:
    // constructor
    MaxFlowSolver(string input_file_path)
    {
        this->input_file_path = input_file_path;
        this->max_flow = 0;
        this->solved = false;
        this->graph = readGraph();
        this->visited = vector<int>(this->n);
        
        // in our implementation, we assume to have a source node and a sink node:
        // the source node is assumed to have index 0
        // the sink node is assumed to have index n-1 (with n = # nodes)
        // this->s = 0; 
        // this->t = this->n - 1; 
        this->s = 0; 
        this->t = this->n -1; 
       // cout << "Source: " << this->s << ", Sink: " << this->t << endl;
    }

    long getMaxFlow() {
        return this->max_flow;
    }

    // read the graph and save it into an adjacency list
    vector<list<Edge *>> readGraph(){
    
        ifstream file(this->input_file_path);

        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << this->input_file_path << std::endl;
            return graph; // Return an empty list
        }

        char *end;
        string line;
        getline(file, line);
        this->n = strtol(line.c_str(), &end, 10);
        cout << "Number of nodes: " << this->n << endl;

        vector<list<Edge *>> graph(this->n);
        int i = 0;
        while (getline(file, line)) {
            i++;
            // assuming format "node1 node2 capacity"
            int start_node = strtol(line.c_str(), &end, 10);
            int end_node = strtol(end + 1, &end, 10);
            long capacity = strtol(end + 1, &end, 10);

            Edge *edge = new Edge(start_node, end_node, capacity);
            Edge *edge_residual = new Edge(end_node, start_node, 0);
            
            edge->setResidual(edge_residual); // set the residual edge 
            edge_residual->setResidual(edge); // set the foward edge as "residual" of the residual edge
            
            // cout << "adding line " << i <<  ": " << line << endl;
           // cout << "adding to Node " << start_node << endl;
            graph[start_node].push_back(edge);
            graph[end_node].push_back(edge_residual);

            
        }
        cout << "Graph read" << endl;
        return graph;
    }


    // print the graph in format edge - edge, capacity
    void printGraph() {
        for (auto node : this->graph) {
            for (auto edge : node) {
                if (!edge->isResidual()) 
                    cout << edge->toString() << endl;
            }
        }
    }

    // print resulting graph to file
    void printGraphToFile(string fout) {
        //string output = "result.txt";
        ofstream outputFile(fout);
        
        if (!outputFile.is_open()) {
            std::cerr << "Failed to open file: " << fout << std::endl;
            return;
        }
   
        for (auto node : this->graph) {
            for (auto edge : node) {
                if (!edge->isResidual()) 
                    outputFile << edge->toStringFile() << endl;
            }
        }
        
        outputFile.close();
    }


    /// Prints the residual graph in format "edge - edge, residual_capacity"
    void printGraphResidual() {
        for (auto node : this->graph){
            for (auto edge : node){
                cout << edge->toString() << endl;
            }
        }
    }

    void setVisited(int i){
        this->visited[i] = this->visit_flag;
        return;
    }

    // Returns whether or not node 'i' has been visited.
    bool isVisited(int i){
        return this->visited[i] == this->visit_flag;
    }

    // to see if useful?
    void markAllNodesAsUnvisited(){
        visit_flag++;
    }

    void solve(){
        int t_limit = thread::hardware_concurrency();
        // compute max flow
        for (long f = parallelDfs(this->s, INF, t_limit); f != 0; f = parallelDfs(this->s, INF, t_limit)){
            this->visit_flag++;
            this->max_flow += f;
        }
    }

    long parallelDfs(int node, long flow, int thread_limit) {
        if (node == this->t) {
            return flow;
        }

        this->visited[node] = visit_flag;
        std::vector<std::future<long>> futures;

        list<Edge *> node_edges = this->graph[node];
        for (Edge *edge : node_edges) {
            if (edge->getRemainingCapacity() > 0 && this->visited[edge->getEndNode()] != visit_flag) {
                // Launch DFS recursively in parallel
                futures.push_back(async(std::launch::async, [&]() {
                    return parallelDfs(edge->getEndNode(), std::min(flow, edge->getRemainingCapacity()), thread_limit);
                    }));
            }
        }

        for (auto& future : futures){
            long bottleneck = future.get();
            cout << "bottleneck: " << bottleneck << endl;
            cout << "Thread " << std::this_thread::get_id() << " acquiring lock" << endl;
            std::lock_guard<std::mutex> lock(graph_lock); // Ensure thread-safe augmentation
                for (Edge* edge : node_edges) {
                    if (edge->getEndNode() == t) {
                        edge->augment(bottleneck);
                        return bottleneck;
                    }
                }
            if (graph_lock.try_lock()) {
                cout << "lock free" << endl;
            }
        }
        
        return 0;
    }
 
    long bfs(){return 0.00;}

};
