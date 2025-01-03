#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>

using namespace std;

// a struct containing all fields of the label of each node
typedef struct  {
    int pred_id;
    char sign;
    long flow;
} label_t;

class Node {
    private:
        int id;
        label_t *label;
        atomic<bool> labeled;
        
        shared_mutex mx_node;
        mutex mx_cv;       // to read/write label of node
        condition_variable cv;      // used to signal when the label of a node is set

        const long INF = __LONG_LONG_MAX__ / 2;

    public:


        /**
         * Node constructor
         * @param id the id of the node
         *
         * Initialize the node with the given id and set labeled to false
         */
        Node(int id) {
            this->id = id;
            this->label = nullptr;
            this->labeled.store(false);
        }

        /**
         * Retrieves the id of the node.
         * @return the id of the node
         */
        int getId() {
            return this->id;
        }

        /**
         * Sets the label of this node.
         * @param pred_id the id of the predecessor node
         * @param sign the sign of the flow along the edge between pred_id and this
         * @param labelflow the flow to assign to the current node
         *
         * Sets the label of the current node  based on the given parameters. 
         */
        void setLabel(int pred_id, char sign, long labelflow) {
            this->labeled.store(true);
            this->label->pred_id = pred_id;
            this->label->sign = sign;
            this->label->flow = labelflow;
        }

        /**
         * Resets the label of this node.
         * 
         * Sets labeled to false and the label to an empty struct.
         */
        void resetLabel() {
            this->labeled.store(false);
            this->label = {};
        }

        /**
         * Sets the label of this node as source node.
         * The label of the source node is set with pred_id = -1, sign = NULL and flow = INF.
         */
        void setSourceLabel() {
            this->labeled.store(true);
            this->label->pred_id = -1;
            this->label->sign = '\0';
            this->label->flow = INF;
        }

        /**
         * @return true if the predecessor of this node is -1 (i.e., this is the source node), false otherwise
         */
        bool isSource() {
            return this->label->pred_id == -1;
        }

        /**
         * Checks if this node is a sink node (i.e., its id is equal to t).
         * @param t the id of the sink node
         * @return true if this node is a sink node, false otherwise
         */
        bool isSink(int t) {
            return this->id == t;
        }
        
        /**
         * Retrieves the label of this node.
         * 
         * @return A pointer to the label_t structure containing the label's fields:
         *         - pred_id: the predecessor node ID
         *         - sign: the sign of the edge from the predecessor node
         *         - flow: the flow value associated with this node
         */
        label_t *getLabel() {
            return this->label;
        }
        
        /**
         * Checks if the node has already been labeled.
         * 
         * @return true if the node has a label set, false otherwise.
         */
        bool isLabeled() {
            return this->labeled.load();
            /* ver.2 */
            // lock_shared(mx_node);
            // bool val = this->labeled;
            // unlock_shared(mx_node);
            // return val;
        }

        // bool isVisited() {
        //     return this->visited;
        // }

        
        /**
         * Acquires a shared lock on the node's mutex, allowing multiple threads to read the node's data
         * while preventing any thread from modifying the node's data.
         * 
         * This function is used to ensure thread-safety when accessing the node's data, such as when
         * checking if the node has been labeled.
         */
        void lockSharedMutex() {
            this->mx_node.lock();
        }


        /**
         * Releases the shared lock on the node's mutex, allowing other threads to write to the node's data.
         * 
         * This function is used to ensure thread-safety when accessing the node's data. When a thread
         * is finished reading the node's data, it should call this function to release the shared lock,
         * allowing other threads to write to the node's data.
         */
        void unlockSharedMutex() {
            this->mx_node.unlock();
        }

        /**
         * Waits on the node's condition variable until the node is no longer labeled.
         * 
         * This function acquires a unique lock on the node's condition variable mutex
         * and waits until the node's labeled flag is set to false. This can be used
         * to pause execution until the node's label is cleared by another thread.
         */

        void waitOnNodeCV() {
            unique_lock<mutex> lock(this->mx_cv);
            this->cv.wait(lock, [this] {return this->labeled == false;} );
        }

        /**
         * Signals the node's condition variable to all waiting threads, allowing them to proceed.
         * 
         * This function should be called when the node's label is cleared, to notify all threads
         * waiting on the node's condition variable that they can proceed.
         */
        void signalNodeCV() {
            this->cv.notify_all();
        }
};