// Header file that contains the Maximum Flow Graph algorithm
// Author: Michele Perseo

#include <iostream>
#include <list>
#include <String

#include <

using namespace std;

class MaxFlowSolver
{

public:
    class Graph
    {
        // TODO;
    };

    class Edge
    {
        // start node
        int start_node;

        // end node
        int end_node;

        // flow
        long flow;

        // capacity
        const long capacity;

        //residual edge
        Edge* residual;
        

        Edge(int start_node, int end_node, long capacity)
        {
            this->start_node = start_node;
            this->end_node = end_node;
            this->capacity = capacity;
            this->flow = 0;
        }

        bool is_residual(){
            return this->capacity == 0;
        }

        long capacity_available(){
            return this->capacity - this->flow;
        }

        void augment(long bottleneck){
            this->flow += bottleneck;
            residual->flow -= bottleneck;

        }

    };

        string to_string(){
            
            //TODO
        }


    private :
        //INPUTS
        // number of nodes
        int n;
        string input_file_path;

        long max_flow;

        //graph (adjacency list)
        List<Edge*> graph;

        vector




    // to avoid overflow
    const long INF = LONG_MAX / 2;
}
