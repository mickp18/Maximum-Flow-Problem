// Header file that contains class Edge
#include <string>
#include <atomic>

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

        // flag to check if there is already a thread generated for the edge
        atomic<bool> hasThread;

    public:
        // constructor
        Edge(int start_node, int end_node, long capacity)
        {
            this->start_node = start_node;
            this->end_node = end_node;
            this->capacity = capacity;
            this->flow = 0;
            // residual edge is created only when a path is found
            this->hasThread.store(false);
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

        Edge *getResidual() {
            return this->residual;
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
            residual->flow = -this->flow;
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

        bool getHasThread() {
            return this->hasThread.load();
        }

        void setHasThread() {
            this->hasThread.store(true);
        }
};