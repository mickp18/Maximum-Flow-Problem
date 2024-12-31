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

        // getter of start_node
        int getStartNode() {
            return this->start_node;
        }

        // getter of end_node
        int getEndNode() {
            return this->end_node;
        }

        void setResidual(Edge *edge_residual) {
            this->residual = edge_residual;
            return;
        }

        // check if an edge belongs to the residual network
        bool isResidual() {
            return this->capacity == 0;
        }

        // getter of capacity
        long getCapacity() {
            return this->capacity;
        }
        
        // getter of flow
        long getFlow() {
            return this->flow;
        }

        // return remaining capacity of edge
        long getRemainingCapacity() {
            return this->capacity - this->flow;
        }

        // add more flow to an edge of an augmented path
        void augment(long bottleneck) {
            this->flow += bottleneck;
            residual->flow -= bottleneck;
        }
        
        /// Returns a string representation of the edge in the format "node1 - node2, capacity"
        string toString() {
            return std::to_string(this->start_node) + " -> " + std::to_string(this->end_node) + ", Flow: " + 
                    std::to_string(this->flow) + ", Cap: "+ std::to_string(this->capacity);
        }

        string toStringFile() {
            return std::to_string(this->start_node) + " " + std::to_string(this->end_node) + " " + 
                    std::to_string(this->flow);
            
        }
};