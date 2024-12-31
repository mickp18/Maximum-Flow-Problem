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
        bool labeled;
        //bool visited;

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
            this->labeled = false;
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
            this->labeled = true;
            this->label->pred_id = pred_id;
            this->label->sign = sign;
            this->label->flow = label_flow;
        }


        /**
         * Sets the label of this node as source node.
         * 
         * The label of the source node is set with pred_id = -1 and flow = INF.
         */
        void setSourceLabel() {
            this->labeled = true;
            this->label->pred_id = -1;
           // this->label->sign = '+';
            this->label->flow = INF;
        }
        /**
         * @return true if the predecessor of this node is -1 (i.e., this is the source node), false otherwise
         */

        bool isSource() {
            return this->label->pred_id == -1;
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
            return this->labeled;
        }

        // bool isVisited() {
        //     return this->visited;
        // }
};