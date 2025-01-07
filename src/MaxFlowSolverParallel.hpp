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
ofstream tmpfout;
void reorderFile(const std::string &inputFile, const std::string &outputFile);

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
    mutex mx_done, mx_sink_reached, mx_num_genereated, mx_num_blocked;
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

    /*
        label source node
    for every neighbour of source:
        generate a thread and pass the func f to each thread
    wait till all threads finish
    return max flow....
    */

    auto computeTime() {
        auto now = std::chrono::high_resolution_clock::now();    // time instant now
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - this->start);    // duration from start instant till current point in time
        return duration.count();
    }

    void solve(){
        this->nodes[this->s]->setSourceLabel();     // set label of source node

        // save edges of source node
        list<Edge *> source_edges = this->graph[this->s];
        int num_source_edges = source_edges.size();
        
        this->start = std::chrono::high_resolution_clock::now();
        tmpfout.open("../outputs/tmp_out.txt");;
        tmpfout.close();

        //for every neighbour of source: generate a thread and pass it the func thread_function
        for (auto edge : source_edges) { 
            int u = edge->getStartNode();
            int v = edge->getEndNode();

            if (edge->isResidual())
            {
                // threads.emplace_back(&MaxFlowSolverParallel::thread_function, this, edge->getEndNode(), edge->getStartNode(), edge);
                continue;
            }
            else
            {
                threads.emplace_back(&MaxFlowSolverParallel::thread_function, this, edge->getStartNode(), edge->getEndNode(), edge);
            }

            this->mng.lock();
            this->num_generated.fetch_add(1);
            this->mng.unlock();
            
            edge->setHasThread();  
        }

        // wait till all threads finish
        mx_print.lock();
        tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
        cout << computeTime() << ": " << "num threads to join in solve: " << this->threads.size() << endl;
        tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
        mx_print.unlock();

        // for (int i = 0;  i < this->threads.size(); i++) {
        for (int i = 0; i < num_source_edges; i++) {
            this->threads[i].join();
        }


        mx_print.lock(); 
        tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
        cout << computeTime() << ": " << "All threads have finished, wow incredible \\(^o^)/" << endl;
        cout << computeTime() << ": " << "SIUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUM" << endl;
        tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
        mx_print.unlock();

        for (int i = 0; i < this->n; i++){
            this->nodes[i]->freeLabel();
            delete this->nodes[i];
            this->nodes[i] = nullptr;
        }
        this->nodes.clear();
    }

    // lambda func for condition variable of worker threads
    bool f(bool *sem, int u, int v) {
        if (*sem == true ) {
            *sem = false;       
            
            mx_print.lock();
            tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
            tmpfout << computeTime() << ": " << "-> thread (" << u << ", " << v << ") " << "stops, sem: " << *sem << endl;
            tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
            mx_print.unlock();
            
            this->mnb.lock();
            this->num_blocked.fetch_add(1);
            this->mnb.unlock();

            if ( (this->num_blocked.load() + this->num_waiting_label.load()) == (this->num_generated.load() - 1) ) {
            // if ( this->num_running.load() == 1 ) {
                this->cv_augment.notify_one();
            }
            return false;
        } else {            // sem == false
            *sem = true;
            mx_print.lock();
            tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
            tmpfout << computeTime() << ": " << "-> thread: (" << u << ", " << v << ") " << "restarts, sem: " << *sem << endl;
            tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
            mx_print.unlock();
            return true;
        }
    };

    void thread_function(int u, int v, Edge *edge) {
        bool sem = true;
        bool is_augmenter = false;
        vector<thread> threads_local;
        long augmented_flow = 0;

        this->num_running.fetch_add(1);

        mx_print.lock();
        tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
        tmpfout << computeTime() << ": " << "(enter) I am tid: " << this_thread::get_id() << " with edge: (" << u << ", " << v << ") "<< endl;
        tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
        mx_print.unlock();

        // thread operations
        while (!this->done.load()){     // ?? need to lock the mutex of done ?? No -> https://chatgpt.com/share/6776c875-9cf0-8004-90ae-f5cdfbce30b4           
            if (this->sink_reached.load()){
                {   // block
                    mx_print.lock();
                    tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                    tmpfout << computeTime() << ": " << "(1) thread (" << u << ", " << v << ") will block on cv, nb: " << this->num_blocked.load() << " ng: " << this->num_generated.load() << endl;
                    tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                    mx_print.unlock();

                    this->num_running.fetch_sub(1);
                    
                    unique_lock<mutex> lock(this->mx_cv);
                    this->cv.wait(lock, [this, &sem, u, v] {
                        mx_print.lock();
                        tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                        tmpfout << computeTime() << ": " << "(1) thread (" << u << ", " << v << ") checks cv" << endl;
                        tmpfout << computeTime() << ": " << "-> (bef) nb: " << this->num_blocked.load() << ", ng: " << this->num_generated.load() << endl;
                        tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                        mx_print.unlock();

                        bool res = f(&sem, u, v);
                        
                        mx_print.lock();
                        tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                        tmpfout << computeTime() << ": " << "-> (aft) nb: " << this->num_blocked.load() << ", ng: " << this->num_generated.load() << endl;
                        tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                        mx_print.unlock();
                        return res;

                        // return f(&sem, u, v);
                    });

                    this->num_running.fetch_add(1);

                    mx_print.lock();
                    tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);               
                    tmpfout << computeTime() << ": " << "(1) " << " edge: (" << u << ", " << v << ") " << "gets out of cv"  << endl;
                    tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                    mx_print.unlock();
                }
                
                if (this->done.load()) {
                    // this->num_running.fetch_sub(1);
                    mx_print.lock();
                    tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                    tmpfout << computeTime() << ": " << "(*) thread (" << u << ", " << v << ") exits"<< endl;
                    tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                    mx_print.unlock();
                    break;
                }
            }        

            // if (!u_is_labeled) {
            //     cout << "thread (" << u << ", " << v << ") "<< "waits the Node " << u << endl;
            //     // blocked on cv also!!
            //     this->num_blocked.fetch_add(1);
            //     this->nodes[u]->waitOnNodeCV();
            //     this->num_blocked.fetch_sub(1);
            //     u_is_labeled = this->nodes[u]->isLabeled();
            // }
            mx_print.lock();
            tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
            tmpfout << computeTime() << ": " << "Thread (" << u << ", " << v << ") trying to lock mx of u=" << u << endl;
            tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
            mx_print.unlock();

            this->nodes[u]->lockSharedMutex();


            mx_print.lock();
            tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
            tmpfout << computeTime() << ": " << "Thread (" << u << ", " << v << ") locked mx of u=" << u << endl;
            tmpfout << computeTime() << ": " << "Thread (" << u << ", " << v << ") trying to lock mx of v=" << v << endl;
            tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
            mx_print.unlock();

            this->nodes[v]->lockSharedMutex();

            mx_print.lock();
            tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
            tmpfout << computeTime() << ": " << "Thread (" << u << ", " << v << ") locked mx of v=" << v << endl;
            tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
            mx_print.unlock();

            bool u_is_labeled = this->nodes[u]->isLabeled();
            bool v_is_labeled = this->nodes[v]->isLabeled();
            // bool& ref_u = ref(u_is_labeled);
            
            // thread blocked if both nodes have no label, remains blocked until one of the two nodes gets a label or until there is no augmenter left
            if (!(u_is_labeled || v_is_labeled)) {
                bool first_time = true;

                // if (this->num_running.load() == 1 && (this->num_blocked.load() + this->num_waiting_label.load() == (this->num_generated.load() - 1))) {
                if ((this->num_blocked.load() + this->num_waiting_label.load() == (this->num_generated.load() - 1))) {
                    this->done.store(true);
                    this->cv.notify_all();
                    this->cv_nodes.notify_all();
                    this->nodes[u]->unlockSharedMutex();
                    this->nodes[v]->unlockSharedMutex();
                    break;
                }

                this->num_waiting_label.fetch_add(1);
                this->num_running.fetch_sub(1);
                
                unique_lock<mutex> lock(this->mx_node);
                this->cv_nodes.wait(lock, [this, u, v, &u_is_labeled, &v_is_labeled, &first_time] { 
                    // ref_u = this->nodes[u]->isLabeled();
                    if (!first_time) {
                        u_is_labeled = this->nodes[u]->isLabeled();
                        v_is_labeled = this->nodes[v]->isLabeled();
                    } else {
                        first_time = false;
                    }
                    // if (!(this->nodes[v]->isLabeled() || this->nodes[u]->isLabeled())) {
                    if (!u_is_labeled &&  !v_is_labeled) {
                        // this->num_blocked.fetch_add(1);                
                        mx_print.lock();
                        tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                        tmpfout << computeTime() << ": " << "Thread (" << u << ", " << v << ") "<< "blocks since neither " << u << " nor " << v << " are labeled" << endl;
                        this->nodes[u]->unlockSharedMutex();
                        tmpfout << computeTime() << ": " << "(no labels of nodes): Thread (" << u << ", " << v << ") Unlocked mx of u=" << u << endl;
                        this->nodes[v]->unlockSharedMutex();
                        tmpfout << computeTime() << ": " << "(no labels of nodes): Thread (" << u << ", " << v << ") Unlocked mx of v=" << v << endl;
                        tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                        mx_print.unlock();

                        return false;
                    }
                    // this->num_blocked.fetch_sub(1);
                    // this->nodes[u]->lockSharedMutex();
                    // this->nodes[v]->lockSharedMutex();
                    mx_print.lock();
                    tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                    tmpfout << computeTime() << ": " << "Thread (" << u << ", " << v << ") trying to lock mx of u=" << u << endl;
                    tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                    mx_print.unlock();

                    this->nodes[u]->lockSharedMutex();

                    mx_print.lock();
                    tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                    tmpfout << computeTime() << ": " << "Thread (" << u << ", " << v << ") locked mx of u=" << u << endl;
                    tmpfout << computeTime() << ": " << "Thread (" << u << ", " << v << ") trying to lock mx of v=" << v << endl;
                    tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                    mx_print.unlock();
                    this->nodes[v]->lockSharedMutex();

                    mx_print.lock();
                    tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                    tmpfout << computeTime() << ": " << "Thread (" << u << ", " << v << ") locked mx of u=" << u << endl;
                    tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                    mx_print.unlock();
                    return true;
                });
                this->num_waiting_label.fetch_sub(1);
                this->num_running.fetch_add(1);
                
                if (this->done.load()) {
                    break;
                }


                // u_is_labeled = this->nodes[u]->isLabeled();
                // v_is_labeled = this->nodes[v]->isLabeled();
            }

            long pred_flow = this->nodes[u]->getLabel()->flow;
            // EDGE (U,V)
            // if u is labeled and unscanned, v is unlabeled and f(u, v) < c(u, v) ( equiv. to c(u,v) - f(u,v) > 0)
            if (u_is_labeled && !v_is_labeled) {
                mx_print.lock();
                cout << "FORWARD EDGE (" << u << ", " << v << ")" << endl;
                mx_print.unlock();
                long remaining_capacity = edge->getRemainingCapacity();
                if (remaining_capacity > 0){
                    // assign the label (u, +, l(v)) to node v, Where l(v) = min(l(u), c(u, v) − f(u, v)). 
                    long label_flow = std::min(pred_flow, remaining_capacity);

                    mx_print.lock();
                    tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                    tmpfout << computeTime() << ": " << "Thread (" << u << ", " << v << ") "<< "setting label of node " << v << endl;
                    tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                    mx_print.unlock();
                    this->nodes[v]->setLabel(u, '+', label_flow);
                    // check if the node on which the label was just set is the sink
                    if (this->nodes[v]->isSink(t) && this->augmenter_thread_exists.load() == false) {
                        this->sink_reached.store(true);
                        this->augmenter_thread_exists.store(true);
                        is_augmenter = true;
                    }
                // Let u be labeled and scanned and v is labeled and un-scanned
                        // .../
                    this->cv_nodes.notify_all();
                } 
                else {
                    mx_print.lock();
                    tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                    tmpfout << computeTime() << ": " << "Thread (" << u << ", " << v << ") "<< "has remaining capacity <= 0: " << remaining_capacity << endl;
                    tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                    mx_print.unlock();
                }
            }
            // EDGE (V, U)
            // else if v is labeled and un-scanned, u is unlabeled and f(u, v) > 0.
            else if (v_is_labeled && !u_is_labeled) {
                mx_print.lock();
                cout << "BACKWARD EDGE (" << u << ", " << v << ")" << endl;
                mx_print.unlock();
                long edge_flow = edge->getFlow();
                if (edge_flow > 0) {
                // assign the label (v, −, l(u)) to node u, where l(u) = min(l(v), f(u, v))
                    long label_flow = std::min(pred_flow, edge_flow);
                    mx_print.lock();
                    tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                    tmpfout << computeTime() << ": " << "Thread: (" << u << ", " << v << ") "<< "setting label of node " << u << endl;
                    tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                    mx_print.unlock();
                    this->nodes[u]->setLabel(v, '-', label_flow);
                    // check if the node on which the label was just set is the sink
                    if (this->nodes[v]->isSink(t) && !this->augmenter_thread_exists.load()) {
                        this->sink_reached.store(true);
                        this->augmenter_thread_exists.store(true);
                        is_augmenter = true;
                    }
                    this->cv_nodes.notify_all();
                //Let v be labeled and scanned and u is labeled and un-scanned
                }
                else {
                    mx_print.lock();
                    tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                    tmpfout << computeTime() << ": " << "Thread (" << u << ", " << v << ") "<< "has edge flow <= 0: " << edge_flow << endl;
                    tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                    mx_print.unlock();
                }
            } 
            
            // this->nodes[u]->unlockSharedMutex();
            // this->nodes[v]->unlockSharedMutex();
            
            mx_print.lock();
            tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
            this->nodes[u]->unlockSharedMutex();
            tmpfout << computeTime() << ": " << "cv_nodes: Thread (" << u << ", " << v << ") UNlocked mx of u=" << u << endl;
            this->nodes[v]->unlockSharedMutex();
            tmpfout << computeTime() << ": " << "cv_nodes: Thread (" << u << ", " << v << ") UNlocked mx of v=" << v << endl;
            tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
            mx_print.unlock();


            if (this->augmenter_thread_exists.load() && is_augmenter) {
                // AUGMENTER THREAD

                // wait until all the other threads are blocked on the conditional variable
                {
                    mx_print.lock();
                    tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                    tmpfout << computeTime() << ": " << "I am the augmenter thread (" << u << ", " << v << ")"<< endl;
                    tmpfout << computeTime() << ": " << "*. Augmenter" << " edge: (" << u << ", " << v << ") " << "is waiting on cv augment"  << endl;
                    // cout << "num_blocked: " << this->num_blocked.load() << " num_generated: " << this->num_generated.load() << endl;
                    tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                    mx_print.unlock();

                    // if ((this->num_blocked.load() + this->num_waiting_label.load()) < (this->num_generated.load() - 1)) {
                    unique_lock<mutex> l_augment(this->mx_cv);
                    this->cv_augment.wait(l_augment, [this, u, v] {
                        mx_print.lock();
                        tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                        tmpfout << computeTime() << ": " << "thread (" << u << ", " << v << "), " << "nb: " << this->num_blocked.load() << " ng: " << this->num_generated.load() << endl;
                        tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                        mx_print.unlock();
                        return ((this->num_blocked.load() + this->num_waiting_label.load()) == (this->num_generated.load() - 1) ); 
                        // return (this->num_running.load() ==  1); 
                    });
                }


                mx_print.lock();
                tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                tmpfout << computeTime() << ": " << "*. Augmenter" << " - edge: (" << u << ", " << v << ") " <<  "gets out of cv augment"  << endl;
                tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                mx_print.unlock();
                

                augmented_flow = augment();
                mx_print.lock();
                tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                tmpfout << computeTime() << ": " << "augmented flow: " << augmented_flow << endl;
                tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                mx_print.unlock();

                // update max flow
                this->max_flow += augmented_flow;
                
                if (augmented_flow == 0 || !sinkCapacityLeft() || !sourceCapacityLeft()) {
                    mx_print.lock();
                    tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                    tmpfout << computeTime() << ": " << "done" << endl;
                    tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                    mx_print.unlock();

                    this->done.store(true);
                } 
                // reset
                mx_print.lock();
                tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                tmpfout << computeTime() << ": " << "reset" << endl;
                tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                mx_print.unlock();

                resetLabels();
                this->augmenter_thread_exists.store(false);
                is_augmenter = false;
                augmented_flow = 0;
                this->mnb.lock();
                this->num_blocked.store(0);
                this->mnb.unlock();
                this->cv.notify_all();  // wake all blocked threads (to restart)

            } else {
                //  WORKER THREAD

                if (!this->nodes[v]->isSink(t)) {
                // SPAWNING OF NEW THREADS
                    list<Edge *> neighbour_edges = this->graph[v];

                    mx_print.lock();
                    tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                    tmpfout << computeTime() << ": " << "thread (" << u << ", " << v << ")"<< " starts spawning" << endl;
                    tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                    mx_print.unlock();

                    for (auto next_edge : neighbour_edges) {
                        // get if the end node of a possible nexte edge has already a label
                        bool next_end_node_labeled = this->nodes[next_edge->getEndNode()]->isLabeled();

                        if (!next_end_node_labeled && !next_edge->getHasThread() && !this->augmenter_thread_exists) {
                            
                            mx_print.lock();
                            tmpfout << computeTime() << ": " << "thread (" << u << ", " << v << ")"<< " generates new thread (" << next_edge->getStartNode() << ", " << next_edge->getEndNode() << ")"<< endl;
                            tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                            mx_print.unlock();
                            {
                                lock_guard<mutex> lock(this->mx_cv);
                                this->mng.lock();
                                this->num_generated.fetch_add(1);
                                this->mng.unlock();
                                next_edge->setHasThread(); 
                                if (next_edge->isResidual()) {
                                    threads_local.emplace_back(&MaxFlowSolverParallel::thread_function, this, next_edge->getStartNode(), next_edge->getEndNode(), next_edge);
                                } else {
                                    threads_local.emplace_back(&MaxFlowSolverParallel::thread_function, this, next_edge->getStartNode(), next_edge->getEndNode(), next_edge);
                                }
                            }
                        }
                    }
                    mx_print.lock();
                    tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                    tmpfout << computeTime() << ": " << "thread (" << u << ", " << v << ") finished spawning; nb: " << this->num_blocked.load() << " ng: " << this->num_generated.load()<< endl;
                    tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                    mx_print.unlock();
                }

                // blocking of WORKER on CV

                // if this is the last running thread and it is about to get blocked, there is no more augmenting flow available, so the algorithm should end
                // if (this->num_running.load() == 1) {
                if ((this->num_blocked.load() + this->num_waiting_label.load()) == (this->num_generated.load() - 1)) {
                    this->done.store(true);
                    this->cv.notify_all();
                    this->cv_nodes.notify_all();
                    break;
                }

                {
                    mx_print.lock();
                    tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                    tmpfout << computeTime() << ": " << "(2) thread (" << u << ", " << v << ") will block on cv, nb: " << this->num_blocked.load() << " ng: " << this->num_generated.load() << endl;
                    tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                    mx_print.unlock();
                    
                    this->num_running.fetch_sub(1);
                    unique_lock<mutex> lock(this->mx_cv);
                    this->cv.wait(lock, [this, &sem, u, v] {
                        mx_print.lock();
                        tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                        tmpfout << computeTime() << ": " << "(2) thread (" << u << ", " << v << ") checks cv" << endl;
                        tmpfout << computeTime() << ": " << "-> (bef) nb: " << this->num_blocked.load() << ", ng: " << this->num_generated.load() << endl;
                        tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                        mx_print.unlock();

                        bool res = f(&sem, u, v);
                        
                        mx_print.lock();
                        tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                        tmpfout << computeTime() << ": " << "-> (aft) nb: " << this->num_blocked.load() << ", ng: " << this->num_generated.load() << endl;
                        tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                        mx_print.unlock();
                        return res;

                        // cout << "thread (" << u << ", " << v << "), " << "nb: " << this->num_blocked.load() << " ng: " << this->num_generated.load() << endl;
                        // return f(&sem, u, v);
                    });
                    this->num_running.fetch_add(1);
                }
                mx_print.lock();
                tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
                tmpfout << computeTime() << ": " <<"(2) thread (" << u << ", " << v << ")"<< " gets out of cv"  << endl;
                tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
                mx_print.unlock();
            }
        }

        for (thread &t : threads_local) {
            mx_print.lock();
            tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
            tmpfout << computeTime() << ": " << "i'm a thread (" << u << ", " << v << ")" << " and joining thread: " << t.get_id() << endl;
            tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
            mx_print.unlock();

            t.join();
        } 
        mx_print.lock();
        tmpfout.open("../outputs/tmp_out.txt", ios_base::out | ios_base::app);
        tmpfout << computeTime() << ": " << "(return) end of thread (" << u << ", " << v << ")"<< endl;
        tmpfout.close(); reorderFile("../outputs/tmp_out.txt", "../outputs/tmp_out_sorted.txt");
        mx_print.unlock();

        this->num_running.fetch_sub(1);
        this->num_generated.fetch_sub(1);

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




// Define a struct to hold each line with its integer value and the string part
struct Line {
    int number;   // Integer before the first colon
    std::string text;   // The string part after the first colon

    // Constructor to initialize the struct
    Line(int num, const std::string& txt) : number(num), text(txt) {}
};

void reorderFile(const std::string& inputFile, const std::string& outputFile) {
    // Open the input file
    std::ifstream inFile(inputFile);
    if (!inFile.is_open()) {
        std::cerr << "Could not open input file!" << std::endl;
        return;
    }

    // Vector to hold the lines
    std::vector<Line> lines;

    // Read each line from the input file
    std::string line;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        int number;
        char colon; // To read the colon ':'
        std::string text;

        // Try reading the integer number before the first colon
        if (iss >> number >> colon) {
            // Read the rest of the line (the string part after the first colon)
            std::getline(iss, text);
            // Trim any leading spaces from the string
            text = text.substr(text.find_first_not_of(" \t"));

            // Store the line in the vector
            lines.push_back(Line(number, text));
        }
    }

    // Close the input file
    inFile.close();

    // Sort the lines by the integer value (number)
    std::sort(lines.begin(), lines.end(), [](const Line& a, const Line& b) {
        return a.number < b.number;  // Compare based on integer value
    });

    // Open the output file
    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        std::cerr << "Could not open output file!" << std::endl;
        return;
    }

    // Write the sorted lines to the output file
    for (const auto& l : lines) {
        outFile << l.number << ": " << l.text << std::endl;
    }

    // Close the output file
    outFile.close();
    //std::cout << "File has been reordered and written to " << outputFile << std::endl;
}