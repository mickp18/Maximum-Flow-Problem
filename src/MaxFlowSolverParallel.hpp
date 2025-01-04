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

#include "Edge.hpp"
#include "Node.hpp"

using namespace std;
/*   
     #
    ##
   ###
  ####
 ## ##
    ##
    ##
    ##
    ##    
    ##
 #######

    ####       ################          ####              ##
  ##    ##     ##            ##        ##    ##           ###
 ##      ##    ##            ##       ##      ##         ####
         ##    ##   #     #  ##              ##         ## ##
        ##     ##            ##             ##         ##  ##
       ##      ##  ##    ##  ##           ##          ##   ##
      ##       ##    ####    ##          ##          ###   ##
     ##        ##            ##        ##           ############
    ##         ##            ##      ##                    ##
   ##          ##            ##     ##                     ##
 ###########   ################     ############           ##
       ## 
      ###
     ####
    ## ##
   ##  ##
  ##   ##
 ###########
       ##
       ##
Happy New Year 2025!
da MGM
 */
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
    mutex mx_done, mx_sink_reached, mx_num_genereated, mx_num_blocked;
    condition_variable cv, cv_augment;
    mutex mx_cv, mx_cv_augment;

    // create a vector of threads
    vector<thread> threads;
    atomic<bool> augmenter_thread_exists;

    mutex mx_print;

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

    /*
        label source node
    for every neighbour of source:
        generate a thread and pass the func f to each thread
    wait till all threads finish
    return max flow....
    */
    void solve(){
        // set label of source node
        this->nodes[this->s]->setSourceLabel();

        // save edges of source node
        list<Edge *> source_edges = this->graph[this->s];
        int num_source_edges = source_edges.size();
        
        //for every neighbour of source:
        //   generate a thread and pass the func thread_function to each thread
        for (auto edge : source_edges) { 
            int u = edge->getStartNode();
            int v = edge->getEndNode();
            this->num_generated.fetch_add(1);
            this->threads.emplace_back(&MaxFlowSolverParallel::thread_function, this, u, v, edge);
            edge->setHasThread();  
        
        }

        // wait till done is true
        // while (!this->done.load()) {}


        // wait till all threads finish
        mx_print.lock();
        cout << "num threads to join in solve: " << this->threads.size() << endl;
        mx_print.unlock();
// /*         for (int i = 0;  i < this->threads.size(); i++) {
        for (int i = 0; i < num_source_edges; i++) {
            this->threads[i].join();
            // thread.join();
        }
// */

        mx_print.lock();        
        cout << "All threads have finished, wow incredible \\(^o^)/" << endl;
        cout << "SIUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUM" << endl;
        mx_print.unlock();

        for (int i = 0; i < this->n; i++){
            this->nodes[i]->freeLabel();
            delete this->nodes[i];
            this->nodes[i] = nullptr;
            
        }
        this->nodes.clear();
    }

//    (x, y) x->y
    void thread_function(int u, int v, Edge *edge) {
        mx_print.lock();
        cout << "(enter) I am tid: " << this_thread::get_id() << " with edge: (" << u << ", " << v << ") "<< endl;
        mx_print.unlock();
        bool is_augmenter = false;

        
        // bool augmenter_thread_exists = false;
        long augmented_flow = 0;
        vector<thread> threads_local;

        // thread operations...
        while (!this->done.load()){     // ?? need to lock the mutex of done ?? No -> https://chatgpt.com/share/6776c875-9cf0-8004-90ae-f5cdfbce30b4           
            // this->mx_sink_reached.lock();    // to check without mutex?
            if (this->sink_reached.load()){
                // this->mx_num_blocked
                this->mx_cv_augment.lock();
                this->num_blocked.fetch_add(1); // Atomically increments 'num_blocked' by 1
                this->cv_augment.notify_all();
                this->mx_cv_augment.unlock();
                //..
                unique_lock<mutex> lock(this->mx_cv);
                mx_print.lock();
                cout << "1. " << " edge: (" << u << ", " << v << ") " << "waiting on cv"  << endl;
                cout << "num_blocked: " << this->num_blocked.load() << " num_generated: " << this->num_generated.load() << endl;
                mx_print.unlock();
                
                this->cv.wait(lock, [this] {return (this->num_blocked.load() == 0 || this->done.load());} );
                // this->num_blocked.fetch_sub(1);
                
                
                mx_print.lock();                
                cout << "1. " << " edge: (" << u << ", " << v << ") " << "gets out of cv"  << endl;
                mx_print.unlock();
                                
                if (this->done.load()) {
                    // ...        
                    mx_print.lock();
                    cout << " I am thread exiting (" << u << ", " << v << ")"<< endl;
                    mx_print.unlock();

                    break;
                    // return;
                }
                // lock.unlock();
            }        

            bool u_is_labeled = this->nodes[u]->isLabeled();
            if (!u_is_labeled) {
                cout << "Edge: (" << u << ", " << v << ") "<< "waiting on node " << u << endl;
                this->nodes[u]->waitOnNodeCV();
                u_is_labeled = this->nodes[u]->isLabeled();
            }

            bool v_is_labeled = this->nodes[v]->isLabeled();
            long pred_flow = this->nodes[u]->getLabel()->flow;
            // EDGE (U,V)
            // if u is labeled and unscanned, v is unlabeled and f(u, v) < c(u, v) ( equiv. to c(u,v) - f(u,v) > 0)
            if (u_is_labeled && !v_is_labeled) {
                this->nodes[v]->lockSharedMutex();
                long remaining_capacity = edge->getRemainingCapacity();
                if (remaining_capacity > 0){
                    // assign the label (u, +, l(v)) to node v, Where l(v) = min(l(u), c(u, v) − f(u, v)). 
                    long label_flow = std::min(pred_flow, remaining_capacity);

                    mx_print.lock();
                    cout << "Edge: (" << u << ", " << v << ") "<< "setting label of node " << v << endl;
                    mx_print.unlock();


                    this->nodes[v]->setLabel(u, '+', label_flow);
                    // check if the node on which the label was just set is the sink
                    if (this->nodes[v]->isSink(t) && this->augmenter_thread_exists.load() == false) {
                        this->sink_reached.store(true);
                        this->augmenter_thread_exists.store(true);
                        is_augmenter = true;
                    }
                // Let u be labeled and scanned and v is labeled and un-scanned
                }
                this->nodes[v]->signalNodeCV();
                this->nodes[v]->unlockSharedMutex();
            }
            // EDGE (V, U)
            // else if v is labeled and un-scanned, u is unlabeled and f(u, v) > 0.
            else if (v_is_labeled && !u_is_labeled) {
                this->nodes[u]->lockSharedMutex();
                long edge_flow = edge->getFlow();
                if (edge_flow > 0) {
                // assign the label (v, −, l(u)) to node u, where l(u) = min(l(v), f(u, v))
                    long label_flow = std::min(pred_flow, edge_flow);
                    mx_print.lock();
                    cout << " thread: (" << u << ", " << v << ") "<< "setting label of node " << u << endl;
                    mx_print.unlock();

                    this->nodes[u]->setLabel(v, '-', label_flow);
                    // check if the node on which the label was just set is the sink
                    if (this->nodes[v]->isSink(t) && !this->augmenter_thread_exists.load()) {
                        this->sink_reached.store(true);
                        augmenter_thread_exists.store(true);
                        is_augmenter = true;
                    }
                //Let v be labeled and scanned and u is labeled and un-scanned
                }
                this->nodes[u]->signalNodeCV();
                this->nodes[u]->unlockSharedMutex();
            } 
            
            if (augmenter_thread_exists.load() && is_augmenter) {
                // wit until all the other threads are blocked on the conditional variable
                mx_print.lock();
                cout << "I am the augmenter thread (" << u << ", " << v << ")"<< endl;
                mx_print.unlock();

                unique_lock<mutex> l_augment(this->mx_cv_augment);
                mx_print.lock();
                cout << "*. Augmenter" << " edge: (" << u << ", " << v << ") " << "is waiting on cv augment"  << endl;
                cout << "num_blocked: " << this->num_blocked.load() << " num_generated: " << this->num_generated.load() << endl;
                mx_print.unlock();
                this->cv_augment.wait(l_augment, [this] {return (this->num_blocked.load() == (this->num_generated.load() - 1)); } );
                
                mx_print.lock();
                cout << "*. Augmenter" << " - edge: (" << u << ", " << v << ") " <<  "gets out of cv augment"  << endl;
                

                augmented_flow = augment();
                cout << "augmented flow: " << augmented_flow << endl;
                // update max flow
                this->max_flow += augmented_flow;
                if (augmented_flow == 0 || !capacityLeft()) {
                    cout << "done" << endl;
                    this->done.store(true);
                    // this->threads.emplace_back(&MaxFlowSolverParallel::aux, this);
                } else {
                    // reset
                    cout << "reset" << endl;
                    resetLabels();
                    augmenter_thread_exists.store(false);
                    is_augmenter = false;
                    augmented_flow = 0;
                    this->num_blocked=0;
                }
                mx_print.unlock();
        
                this->cv.notify_all();  // wake all threads (to restart)

            } else {
                // SPAWNING OF NEW THREADS

                if (!this->nodes[v]->isSink(t)) {
                    list<Edge *> neighbour_edges = this->graph[v];
                    mx_print.lock();
                    // cout << "tid " << this_thread::get_id() << "spawning" << endl;
                    cout << "thread (" << u << ", " << v << ")"<< " starts spawning" << endl;
                    mx_print.unlock();


                    for (auto next_edge : neighbour_edges) {
                        // get if the end node of a possible nexte edge has already a label
                        bool next_end_node_labeled = this->nodes[next_edge->getEndNode()]->isLabeled();

                        if (!next_end_node_labeled && !next_edge->getHasThread() && !this->augmenter_thread_exists) {
                            
                            mx_print.lock();
                            cout << "thread (" << u << ", " << v << ")"<< " generates new thread (" << next_edge->getStartNode() << ", " << next_edge->getEndNode() << ")"<< endl;
                            mx_print.unlock();

                            threads_local.emplace_back(&MaxFlowSolverParallel::thread_function, this, next_edge->getStartNode(), next_edge->getEndNode(), next_edge);
                            next_edge->setHasThread();   
                            this->mx_cv_augment.lock();
                            this->num_generated.fetch_add(1);
                            this->cv_augment.notify_all();
                            this->mx_cv_augment.unlock();
                        }
                    }
                    mx_print.lock();
                    // cout << "finished spawning" << endl;
                    cout << "thread (" << u << ", " << v << ") finished spawning" << endl;
                    cout << "num_blocked: " << this->num_blocked.load() << " num_generated: " << this->num_generated.load() << endl;
                    mx_print.unlock();
                }
                // block on CV
                this->mx_cv_augment.lock();
                this->num_blocked.fetch_add(1); // Atomically increments 'num_blocked' by 1
                // this->cv_augment.notify_all();
                this->cv_augment.notify_one();
                this->mx_cv_augment.unlock();
                
                unique_lock<mutex> lock(this->mx_cv);
        
                mx_print.lock();
                // cout << "2. tid: " << this_thread::get_id() << " waiting on cv"  << endl;
                cout << "2. thread (" << u << ", " << v << ")"<< " waiting on cv"  << endl;
                cout << "num_blocked: " << this->num_blocked.load() << " num_generated: " << this->num_generated.load() << endl;
                mx_print.unlock();

                this->cv.wait(lock, [this] {return (this->num_blocked.load() == 0 || this->done.load());} );
        
                mx_print.lock();
                // cout << "2. tid: " << this_thread::get_id() << " gets out of cv"  << endl;
                cout << "2. thread (" << u << ", " << v << ")"<< " gets out of cv"  << endl;
                mx_print.unlock();


                // this->num_blocked.fetch_sub(1); 
            }
        }

        // cout << "# generated threads: " << this->num_generated.load() << endl;
        for (thread &t : threads_local) {
            mx_print.lock();
            // cout <<" i'm a thread with id: " << this_thread::get_id() << " and joining thread: " << t.get_id() << endl;
            cout <<" i'm a thread (" << u << ", " << v << ")" << " and joining thread: " << t.get_id() << endl;
            mx_print.unlock();

            t.join();
        } 
        mx_print.lock();
        // cout << "(return) I am a thread with id: " << this_thread::get_id()<< endl;
        cout << "(return) I am thread (" << u << ", " << v << ")"<< endl;
        mx_print.unlock();

        return;
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

    bool capacityLeft() {
        list<Edge*> sink_edges = this->graph[t];        
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
