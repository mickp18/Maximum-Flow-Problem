// Header file that contains the Maximum Flow Graph algorithm
// Author(s): Mick Perseo & Gio Silve & M.N.

#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <fstream>
#include <future>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <sstream>

#include "Edge.hpp"
#include "Node.hpp"
#include "ThreadPool.hpp"

using namespace std; 
ofstream tmpfout;
//void reorderFile(const std::string &inputFile, const std::string &outputFile);

class MaxFlowSolverParallel
{
private:
    // INPUTS
    
    int n;      //  number of nodes
    int s, t;   // source = s , sink = t

    string input_file_path;     // file name

    // maximum flow value to compute
    long max_flow=0;

    // graph (adjacency list)
    vector<list<Edge *>> graph;

    //
    int visit_flag = 1;
    vector<int> visited;
    // vector<Node> nodes;
    vector<Node *> nodes;

    /* Indicates whether the network flow algorithm has ran.
    We would not need to run the solver multiple times, because it always yields the same result. */
    bool solved;

    // to avoid overflow
    const long INF = __LONG_LONG_MAX__ / 2;
    std::mutex graph_lock;

    atomic<bool> done;  // flag to indicate if there is no more augmenting flow left
    atomic<bool> sink_reached;  // flag to indicate if the sink node is labelled
    atomic<int> num_generated;  // # of generated threads
    atomic<int> num_blocked;    // # of threads blocked on cv next_iteration
    atomic<int> num_waiting_label;
    atomic<int> num_running;
    condition_variable cv, cv_augment;
    mutex mx_cv, mx_cv_augment;

    // create a vector of threads
    vector<thread> threads;
    atomic<bool> augmenter_thread_exists;

    mutex mx_print;
    // atomic<bool> toContinue;

    mutex mng, mnb;
    mutex mx_node;
    condition_variable cv_nodes;

    std::chrono::time_point<std::chrono::high_resolution_clock> start;  // reference time instant

    atomic<int> pending_jobs{0}; // Track total active jobs
public:
    // constructor
    MaxFlowSolverParallel(string input_file_path)
    {
        this->input_file_path = input_file_path;
        this->max_flow = 0;
        this->solved = false;
        this->graph = readGraph();
        this->visited = vector<int> (this->n);
        this->augmenter_thread_exists.store(false);
        // this->toContinue.store(false);
        
        // in our implementation, we assume to have a source node and a sink node:
        // the source node is assumed to have index 0
        // the sink node is assumed to have index n-1 (with n = # nodes)
        this->s = 0; 
        this->t = this->n -1; 
       // cout << "Source: " << this->s << ", Sink: " << this->t << endl;
        this->done.store(false);
        this->sink_reached.store(false);
        this->num_generated.store(0);
        this->num_blocked.store(0);
        this->num_waiting_label.store(0);
        this->num_running.store(0);
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

        this->nodes = vector<Node *>(this->n);
        // cout << "nodes size: " << nodes.size() << endl;

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
            
            // cout << "before start node" << endl;
            // cout << "this->nodes[start_node]: " << this->nodes[start_node] << endl;
            if (this->nodes[start_node] == nullptr) {
                Node *node1 = new Node(start_node);
                this->nodes[start_node] = node1;
            }
            // cout << "after start node" << endl;
            // cout << "before end node" << endl;

            if (this->nodes[end_node] == nullptr) {
                Node *node2 = new Node(end_node);
                this->nodes[end_node] = node2;
            }            
            // cout << "after end node" << endl;
        }
        // cout << "hey" << endl;
        cout << "Graph read with nodes: " << endl;
        for (Node* nd : this->nodes) {
            cout << nd->getId() << " ";
        }
        cout << endl;
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


    auto computeTime() {
        auto now = std::chrono::high_resolution_clock::now();    // time instant now
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - this->start);    // duration from start instant till current point in time
        return duration.count();
    }
    /*
        label source node
    for every neighbour of source:
        generate a thread and pass the func f to each thread
    wait till all threads finish
    return max flow....
    */

    /*
    create pool
    start pool
    neighbours edges of source (remove backward edges) --> TODO IN READ GRAPH MAYBE?

    while true
        pred_max_flow = max_flow
        enqueue jobs for all neighbours
        waits for a job to reach sink and all running tasks to end
        check if flow was augmented
            if not, break
            if yes
                update max flow
                destroy queue
                reset labels
                recreate queue
                wake up threads
                loop until no more augmenting paths are found
    stop pool
    */
    /*
   thread function (u,v,edge)

       - checks if sink reached
           - if yes, return
       - lock u,v
       - if v is source or u and v labeled
           - unlock u,v
           - return
       - label(u, v, edge)
       - if v is sink
           sink reached = true
           augment flow
           wake up main
       - else
           find neighbours
           for next_edge in neighbours
               if (!sink reached && n_v is not labeled && n_v != source)
                   if (next_edge is resiudal )
                       enqueue (n_v, n_u, next_edge)
                   else
                       enqueue (n_, n_v, next_edge)
       - unlock u,v
       - return


   1 --> 2  ---> 6                    t1 (1,2) --> label su 2
   |      ^      \
   |       \      |                 nodes[1] = {2} nodes[2] = = {1,3} --> task(2,3)
   |       \       |
   v        \      V
   4 ------>3 ---> 5
   // void label_function (u,v,edge) {

      1 2
      3 2      2 is labeled  and 3 is NOT labeled    edge(2,3) 2-3 is residual? yes -> thread(3,2,edge(2,3)) u is labeled? no  v is labeled? YES  // assign the label (2, −, l(u)) to node 3, where l(u) = min(l(v), f(u, v))
      3 2      2 is labeled  and 3 is NOT labeled    edge(2,3) -> thread(2,3,edge(2,3)) u is labeled? YES  v is labeled? NO  // assign the label (2, +, l(v)) to node 3, Where l(v) = min(l(u), c(u, v) − f(u, v)).
      graph[1] = {edge(1,2)}, graph[2] = {edge(2,1), edge(2,3)},   graph[3] = {edge(3,2)}
       - if u labeled, v unlabeled -> forward edge -> label on v
       - if v labeled, u unlabeled -> backward edge -> label on u  label (v, −, l(u)) on node u
       - return
   }
   */
    void thread_function(ThreadPool *thread_pool, int u, int v, Edge *edge) {
        thread_pool->getMonitor().updateState("Starting task for nodes " + std::to_string(u) + "," + std::to_string(v));
        bool enqueued_any = false;
        Logger() << "thread " << u << " " << v;
        Node* node_u = this->nodes[u];
        Node* node_v = this->nodes[v];
        
        if (this->sink_reached.load()) {
            return;
        }
        thread_pool->getMonitor().updateState("Waiting for locks on nodes " + std::to_string(u) + "," + std::to_string(v));
        Logger() << "locking " << u << " " << v;
        if (u < v)
        {
            node_u->lockSharedMutex();
            node_v->lockSharedMutex();
        }
        else
        {
            node_v->lockSharedMutex();
            node_u->lockSharedMutex();
        }
        thread_pool->getMonitor().updateState("Got locks for nodes " + std::to_string(u) + "," + std::to_string(v));
        Logger() << "locked " << u << " " << v;

        
        
       

        // if (node_v->isSource() || (this-node_u->isLabeled() && node_v->isLabeled())) {
        //     cout << "thread " << u << " " << v << " sink reached" << endl;
        //     node_v->unlockSharedMutex();
        //     node_u->unlockSharedMutex();
        //     cv_mina.notify_all();
        //     return;
        // }
        
        // treat labelling
        if (!this->assign_label(node_u, node_v, edge)){
            Logger() << "thread " << u << " " << v << " label not assigned";
            thread_pool->getMonitor().updateState("Releasing locks for nodes " + std::to_string(u) + "," + std::to_string(v));
            node_u->unlockSharedMutex();
            node_v->unlockSharedMutex();
            // if (thread_pool->emptyQueue() && n_running.load() == 1)
            //     cv_mina.notify_all();
            return;
        }

        if (v == this->t) {
            this->sink_reached.store(true);
            // this->augment_flow = augment();
            Logger() << "thread " << u << " " << v << " sink found";
            thread_pool->getMonitor().updateState("Releasing locks for nodes " + std::to_string(u) + "," + std::to_string(v));
            node_v->unlockSharedMutex();
            node_u->unlockSharedMutex();
            Logger() << "unlocking " << u << " " << v;
            return;
        }

        list<Edge *> neighbour_edges = this->graph[v];
        Logger() << "thread " << u << " " << v << " neighbours "<< neighbour_edges.size();
        for (auto next_edge : neighbour_edges)
        {
            // Skip if sink is already reached
            if (this->sink_reached.load())
            {
                Logger() << "thread " << u << " " << v << " sink reached";
                break;
            }

            // int next_node = next_edge->isResidual() ? next_edge->getStartNode() : next_edge->getEndNode();
            int next_node = next_edge->getEndNode();
            // Only explore if:
            // 1. Node isn't labeled yet
            // 2. Edge has remaining capacity
            // 3. We're not at the source
            Logger() << "thread " << u << " " << v << " has neighbohour id " << next_node;
            Logger() << "thread " << u << " " << v << " has neighbohour edge " << next_edge->getStartNode() << " " << next_edge->getEndNode() << " with capacity " << next_edge->getRemainingCapacity();
            Logger() << "thread " << u << " " << v << " has neighbohour labeled " << nodes[next_node]->isLabeled();
            if (!this->nodes[next_node]->isLabeled() &&
                next_edge->getRemainingCapacity() > 0 &&
                next_node != this->s && next_node != u )
            {
            
                int next_u = next_edge->isResidual() ? next_edge->getEndNode() : next_edge->getStartNode();
                int next_v = next_edge->isResidual() ? next_edge->getStartNode() : next_edge->getEndNode();
                thread_pool->getMonitor().updateState("Adding neighbor tasks for node " + std::to_string(v));
                Logger() << "thread " << u << " " << v << " has neighbohour " << next_u << " " << next_v;
                this->pending_jobs.fetch_add(1, std::memory_order_relaxed);
                thread_pool->QueueJob([&thread_pool, this, next_edge]
                    {
                        thread_function(thread_pool,
                                next_edge->getStartNode(),
                                next_edge->getEndNode(),
                                next_edge); 
                    });
                pending_jobs.fetch_sub(1, std::memory_order_relaxed); // 🔥 Decrement after execution
               
               
            }
        }

        Logger() << "unlocking " << u << " " << v;
        thread_pool->getMonitor().updateState("Unlocking nodes " + std::to_string(u) + "," + std::to_string(v));
        node_v->unlockSharedMutex();
        node_u->unlockSharedMutex();
        thread_pool->getMonitor().updateState("Unlocked nodes " + std::to_string(u) + "," + std::to_string(v));
        
        Logger() << "thread " << u << " " << v << " done";
        thread_pool->getMonitor().updateState("Thread " + std::to_string(u) + "," + std::to_string(v) + " done");
        return;
    }
    // if queue not empty, but sink reached -> isprocessing remains true, main doesn't wake up
    bool assign_label(Node *n_u, Node *n_v, Edge *edge) {
        // check if we are handling residual edges
        if (edge->isResidual()){
            Node* temp = n_u;
            n_u = n_v;
            n_v = temp;
        }
        
        bool u_is_labeled = n_u->isLabeled();
        bool v_is_labeled = n_v->isLabeled();
        long pred_flow_u = n_u->getLabel()->flow;
        long pred_flow_v = n_v->getLabel()->flow;
        Logger() << "assigning label"  << n_u->getId() << " " << n_v->getId();
        Logger() << "u is labeled " << u_is_labeled << " v is labeled " << v_is_labeled;
        Logger() << "pred flow u " << pred_flow_u << " pred flow v " << pred_flow_v;

        if (u_is_labeled && !v_is_labeled)
        {
          
            long remaining_capacity = edge->getRemainingCapacity();
            if (remaining_capacity > 0)
            {
                // assign the label (u, +, l(v)) to node v, Where l(v) = min(l(u), c(u, v) − f(u, v)).
                long label_flow = min(pred_flow_u, remaining_capacity);

                n_v->setLabel(n_u->getId(), '+', label_flow);
               Logger() << "assigned label on" << n_v->getId();
                return true;
            }
        }
        // EDGE (V, U)
        // else if v is labeled and un-scanned, u is unlabeled and f(u, v) > 0.
        else if (v_is_labeled && !u_is_labeled)
        {
            long edge_flow = edge->getFlow();
            Logger() << "edge flow " << edge_flow;
            if (edge_flow < 0)
            {
                // assign the label (v, −, l(u)) to node u, where l(u) = min(l(v), f(u, v))
                long label_flow = std::min(pred_flow_v, -edge_flow);
                n_u->setLabel(n_v->getId(), '-', label_flow);
               
                Logger() << "assigned label on" << n_u->getId();
                return true;   
            }

        }
        else {
            Logger() << "no label assigned" ;
        }
        return false;
    }

    void solve(){
        // create thread pool

        this->nodes[this->s]->setSourceLabel();     // set label of source node


        ThreadPool thread_pool;
      
        Logger() << "starting";
        

        // save edges of source node
        list<Edge *> source_edges = this->graph[this->s];
        int num_source_edges = source_edges.size();
        //bool start = true;
     
        thread_pool.Start();
        while (true) {
            for (auto edge : source_edges) {
                if (edge->getRemainingCapacity() > 0) {
                    int u = edge->getStartNode();
                    int v = edge->getEndNode();
                    Logger() << "edge u " << u << " v " << v << " with edge remaining capacity " << edge->getRemainingCapacity();
                    {
                    // unique_lock<mutex> lock(mx);
                    pending_jobs.fetch_add(1, std::memory_order_relaxed); 
                    thread_pool.QueueJob([&thread_pool, this, u, v, edge] {
                        thread_function(&thread_pool, u, v, edge); });
                    }
                    pending_jobs.fetch_sub(1, std::memory_order_relaxed); // 🔥 Decrement after execution
                } else {
                    Logger() << "edge u " << edge->getStartNode() << " v " << edge->getEndNode() << " with NEGATIVE or 0 edge remaining capacity: " << edge->getRemainingCapacity();
                }
            }

            // cout << "wait for sink" << endl;
            // Wait until job found sink  and Wait until all threads completed running tasks
                 

            
            thread_pool.getMonitor().dumpState();
            thread_pool.waitForCompletion(&pending_jobs);

            Logger() << "mina woke up" ;
            if (!thread_pool.isProcessing()) {
                Logger() << "thread pool wasn't processing";
            }
            if (!this->sink_reached.load())
            {
                Logger() << "MAIN: sink not found";
                break;
            }
            
            float augment_flow = augment();
            // if no augmentation was done, the algorithm is finished
            if (augment_flow <= 0){
                Logger() << "MAIN: augment flow <= 0";
                break;

            }
            Logger() << "MAIN: augment flow: " << augment_flow;
            this->max_flow+= augment_flow;
            Logger() << "MAIN: max flow: " << this->max_flow;
            Logger() << "MAIN: resetting sink reached";
            // destroy queue
            thread_pool.clearQueue();


            // reset labels
            resetLabels();


            // wake up threads
            Logger() << "wake up threads";
            thread_pool.notify();
        }

        // stop thread pool
        Logger() << "stopping thread pool";
        thread_pool.Stop();




        // free resources 
        for (int i = 0; i < this->n; i++){
            this->nodes[i]->freeLabel();
            delete this->nodes[i];
            this->nodes[i] = nullptr;
        }
        this->nodes.clear();
    }

    long augment() {  
        
        // Step 3. let x = t, then do the following work until x = s.
        // • If the label of x is (y, +, l(x)), then let f(y, x) = f(y, x) + l(t)
        // • If the label of x is (y, −, l(x)), then let f(x, y) = f(x, y) − l(t)
        // • Let x = y
        int x = this->t;
        int y = this->nodes[x]->getLabel()->pred_id;
        Edge *e;
        long sink_flow = this->nodes[x]->getLabel()->flow;

        while (x != s){
            for (Edge *edge : this->graph[y]){
                if (edge->getEndNode() == x) {
                    e = edge;
                    break;
                }
            }

            if (this->nodes[x]->getLabel()->sign == '+'){
                e->augment(sink_flow);
            }
            else{
                e->augment(-sink_flow);
            }
        
            x = y;
            y = this->nodes[x]->getLabel()->pred_id;
        }

        return sink_flow;
    }

    bool sinkCapacityLeft() {
        list<Edge*> sink_edges = this->graph[this->t];        
        // t -> x,  t-> y, t->z   
        //     
        // x->t   --> t->x
        // TODO : create a data str to save all edges from any node X to sink
        for (Edge* edge : sink_edges){
            list<Edge*> opposite_edges = this->graph[edge->getEndNode()];
            for (Edge* e : opposite_edges) {
                if (e->getEndNode() == this->t && e->getRemainingCapacity() > 0) {
                    return true;
                }
            }
        }
        return false;
    }

    bool sourceCapacityLeft() {
        list<Edge*> source_edges = this->graph[this->s];        
        
        for (Edge* e : source_edges) {
            if (e->getRemainingCapacity() > 0) {
                return true;
            }
        }
        return false;
    }


    void resetLabels() {
        // reset all the nodes' labels apart from source
        for (int i = 0; i < this->n; i++){
            if ( i != this->s)
                this->nodes[i]->resetLabel();
        }
        this->sink_reached.store(false);
        
    }
    // long bfs(){return 0.00;}

};



