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
class MaxFlowSolver
{
private:
    // INPUTS
    
    int n;      //  number of nodes
    int s, t;   // source = s , sink = t

    string input_file_path;     // file name

    // maximum flow value to compute
    long max_flow=-1;

    // graph (adjacency list)
    vector<list<Edge *>> graph;

    //
    int visit_flag = 1;
    vector<int> visited;
    vector<Node> nodes;

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
        this->visited = vector<int> (this->n);
        this->nodes = vector<Node> (this->n);
        
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
        // compute max flow:
        // set label of source node
        // for every edge in the graph, generate a thread with a task to set a label to the ending node of the edge to which the thread was assigned
        // once all threads are generated (one thread per edge), run all threads and let them run till when a label on the sink node is set
        // when a label on the sink is set, stop the remaining threads and augment the flow along the discovered path
        // reset all labels of all nodes (but keep the flow assigned so far)
        // repeat all steps till the flow value becomes 0, i.e. till when the augmenting value is 0
     
        this->nodes[s].setSourceLabel();
        for (long f = parallelDfs(this->s, INF, t_limit); f != 0; f = parallelDfs(this->s, INF, t_limit)){
            // this->visit_flag++;
            // augment
            this->max_flow += f;
            // reset source
            // cout << f << endl;
            this->nodes[s].setSourceLabel();
            cout << this->max_flow << endl;
               
            
        }
    }

    long parallelDfs(int node, long flow, int thread_limit) {
        cout << "thread: " << std::this_thread::get_id()<< " " << node << " " << flow << endl;
       
        if (node == this->t) {
            cout << "Thread " <<  std::this_thread::get_id() << " reaches sink" << endl;
            return flow;
        }

        this->visited[node] = visit_flag;
        std::vector<std::future<long>> futures;
        long flow;

        list<Edge *> node_edges = this->graph[node];
        for (Edge *edge : node_edges)
        {
            // create thread for each edge
            //   async(std::launch::async, [=]();

        }
        // wait for all threads to finish

        // return nex flow
        // return flow;

        /*
        //     if (edge->getRemainingCapacity() > 0 && this->visited[edge->getEndNode()] != visit_flag)
        //     {
        //         // Launch DFS recursively in parallel
        //         futures.push_back(async(std::launch::async, [=]()
        //                                 { return parallelDfs(edge->getEndNode(), std::min(flow, edge->getRemainingCapacity()), thread_limit); }));

        //         // Wait for the future to become ready, but don't block
        //         auto status = futures.back().wait_for(std::chrono::milliseconds(0));
        //         while (status != std::future_status::ready)
        //         {
        //             // Future is ready, get the result
        //             long bottleneck = futures.back().get();
        //             if (bottleneck > 0)
        //             {
        //                 edge->augment(bottleneck);
        //                 cout << "edge of " << node << " -> " << edge->getEndNode() << " augmented with flow " << bottleneck << " by thread " << std::this_thread::get_id() << endl;
        //                 return bottleneck;
        //             }
        //         }
        //     }
        // }

        // for (auto& future : futures){  // futures = [thread(1,3)->1, thread(2,3)->1]
        //     long bottleneck = future.get();
        //     cout << "Thread " << std::this_thread::get_id() << " acquiring lock and got bottleneck: " << bottleneck << endl;
        //     std::lock_guard<std::mutex> lock(graph_lock); // Ensure thread-safe augmentation
        //         for (Edge* edge : node_edges) { // 0,1 , 0,2
        //             if (visited[node] == visit_flag) {
        //                 edge->augment(bottleneck);
        //                 cout << "edge of " << node << " -> " << edge->getEndNode() << " augmented with flow " << bottleneck << " by thread " << std::this_thread::get_id() << endl;
        //                 return bottleneck;
        //             }
        //         }
        
        cout << "thread " << std::this_thread::get_id() << " finished with bottleneck 0" << endl;
        return 0;
        */
    }
 
    long thread(int u, int v, Edge *edge){
        // if u is labeled and unscanned, v is unlabeled and f(u, v) < c(u, v)
        if (nodes[u].isLabeled() && !visited[u] && !nodes[v].isLabeled()){
            if (edge->getRemainingCapacity() > 0){
            // assign the label (u, +, l(v)) to node v, Where l(v) = min(l(u), c(u, v) − f(u, v)). 
                long pred_flow = nodes[u].getLabel()->flow;
                long remaining_capacity = edge->getRemainingCapacity();
                long label_flow = std::min(pred_flow, remaining_capacity);
                nodes[v].setLabel(u, '+', label_flow);
                
            // Let u be labeled and scanned and v is labeled and un-scanned
                visited[u] = visit_flag;
            }

        }
        // v is labeled and un-scanned, u is unlabeled and f(u, v) > 0.
        else if (nodes[v].isLabeled() && !visited[v] && !nodes[u].isLabeled() ){
            if (edge->getFlow() > 0) {
            // assign the label (v, −, l(u)) to node u, where l(u) = min(l(v), f(u, v))
                long pred_flow = nodes[v].getLabel()->flow;
                long edge_flow = edge->getFlow();
                long label_flow = std::min(pred_flow, edge_flow);
                nodes[u].setLabel(v, '-', label_flow);

            //Let v be labeled and scanned and u is labeled and un-scanned
                visited[v] = visit_flag;
            }
        }

        // If the sink node t is labeled then go to step 3, else return to step 2.
        if (nodes[t].isLabeled()){
            // augment path
            // Step 3. let x = t, then do the following work until x = s.
            // • If the label of x is (y, +, l(x)), then let f(y, x) = f(y, x) + l(t)
            // • If the label of x is (y, −, l(x)), then let f(x, y) = f(x, y) − l(t)
            // • Let x = y
            int x = t;
            int y = nodes[x].getLabel()->pred_id;
            Edge *e;
            long sink_flow = nodes[x].getLabel()->flow;

            while (x != s){
                for (Edge *edge : this->graph[y]){
                    if (edge->getEndNode() == x)
                        e = edge;
                }
                
                if (nodes[x].getLabel()->sign == '+'){
                    edge->augment(sink_flow);
                }
                else{
                    edge->augment(-sink_flow);
                }
            
                x = y;
                y = nodes[x].getLabel()->pred_id;
            }
            // Then go to step 1.
            return sink_flow;
        } else {
            // get next edges
            list<Edge *> node_edges = this->graph[v];

            for (Edge *edge : node_edges)
            {
                // create thread for each edge
                // ... TODO
                // async(std::launch::async, [=]()
    
            }   
            
        }
    }
    
    
    long bfs(){return 0.00;}

};

/*
thread(u,v)
	
	while (!sink_labeled){
		// put label
        if case 1
		else case 2
		
		if (nodes[t].isLabeled())
			// block all threads
			augment()
			// update flow
			// signal all
			update done
			return
		
		else 
			for (edges of v)
				thread(u1, v1)
			
			wait threads.join
		}
		
    }
       
         > 2
       /
      0       - o labeled, unscanned -> thread (0,1) puts 0 as scanned and puts label on 1
              - 0 labeled, scanned -> thread (2,0') -> ?
        \> 1


*/


/***
generate array of thread ids, with len = 2*num of edges
label source node
for every neighbour of source:
    generate a thread
    add the thread id to the array of thread ids
    pass the func f to each thread
wait for a flow to be returned (>=0)
when a flow val is returned, remove all labels except label of source node
if flow>0, restart from the for loop
else, return max flow


func f (start_node, end_node)

if start node is labeled and flow < capacity
    if end node is not labeled
        compute label of end node
        wx label of end node




*/