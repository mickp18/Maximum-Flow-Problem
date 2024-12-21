// Header file that contains class Edge
#include <string>

using namespace std;

class Edge {
    private:
        // start node
        int start_node;

        // end node
        int end_node;

        // flow
        long flow;

        // capacity
        long capacity;

        // residual edge
        Edge *residual;

    public:
        // constructor
        Edge(int start_node, int end_node, long capacity)
        {
            this->start_node = start_node;
            this->end_node = end_node;
            this->capacity = capacity;
            this->flow = 0;
            // residual edge is created only when a path is found
        }

        void setResidual(Edge *edge_residual) {
            this->residual = edge_residual;
            return;
        }

        // check if an edge belongs to the residual network
        bool is_residual(){
            return this->capacity == 0;
        }

        // return remaining capacity in flow network
        long capacity_available()
        {
            return this->capacity - this->flow;
        }

        // add more flow to an edge of an augmented path
        void augment(long bottleneck)
        {
            this->flow += bottleneck;
            residual->flow -= bottleneck;
        }
        
        /// Returns a string representation of the edge in the format "node1 - node2, capacity"
        string to_string()
        {
            return std::to_string(this->start_node) + " - " + std::to_string(this->end_node) + ", " + std::to_string(this->capacity);
        }
};