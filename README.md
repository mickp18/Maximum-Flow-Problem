


# Maximum-Flow-Problem
WELCOME TEXT ANF GENEREAL INFO
Welcome, this is a project in C++ reg te MAX FLOW PROBLEM, solved with FF, 2 implemenetations 
Below you can find the benchamark files by us (with graph generator found in file `X.py`)
At the end you can see the perfoarmances of the != programs run with specs.....

```
A DOCUMENTATION text file (written in Word, Latex, Mark-down, etc.) including a
short “designer manual”, i.e., a document including:
o All main design choices (how the reading part has been performed, how the data
structure has been organized, how the parallelism has been designed, etc.)
o The experimental evaluation of the tools, i.e., tables or graphics reporting a
reasonable set of experimental results. The experimental evidence should include
a comparison (in terms of memory and of elapsed time) between the original
sequential version of the tool and the 2-3 parallel versions with different
parallelization levels (i.e., with 1, 2, 4, 8, etc., threads) and different size of the
input graph (up to millions of nodes).
o Experiments should be run on synthetically generated graphs and on flow
networks benchmarks usually publicly available from the WWW.
```

- input files with datasets (test datasets): .......
table with file name - # nodes - # edges - max flow value
To RUN
A README text file (written in plain ASCII) including a short “user manual”, i.e., a
document describing how to compile and run the program, under which system, which
API, etc.
SEQ
cmds for SEQ
PAR
cmds for PAR

## Graph Datasets for Testing

| Dataset             | # Nodes      | # Edges        | Max Flow          |
|:--------------------|-------------:|---------------:| ----------------: |
|second_test          | 3            | 3              | 3                 |
|florida              | 7            | 5              | 7                 |
|airports_500_dag     | 500   (0.5K) | 2980    (3K)   | 13926      (14K)  |
|dag_1000_2000        | 1000  (1K)   | 2000    (2K)   | 14                |
|dag_1000_3000        | 1000  (1K)   | 2987    (3K)   | 51                |
|dag_1000_6000        | 1000  (1K)   | 6000    (6K)   | 142        (0.1K) |
|dag_1000_60000       | 1000  (1K)   | 59959   (60K)  | 5299       (5K)   |
|dag_1000_30000       | 1000  (1K)   | 29097   (30K)  | 2173       (2K)   |
|dag_1000_300000      | 1000  (1K)   | 225563  (250K) | 20108      (20K)  |
|dag_5000_30000       | 5000  (5K)   | 29964   (30K)  | 307        (0.3K) |
|dag_5000_60000       | 5000  (5K)   | 59837   (60K)  | 731        (0.7K) |
|dag_5000_600000      | 5000  (5K)   | 585715  (600K) | 10434      (10K)  |
|dag_10000_60000      | 10000 (10K)  | 59959   (60K)  | 133        (0.1K) |
|dag_10000_600000     | 10000 (10K)  | 596388  (600K) | 5076       (5K)   |
|dag_10000_1000000    | 10000 (10K)  | 989896  (1M)   | 9168       (9K)   |
|dag_10000_6000000    | 10000 (10K)  | 5654501 (6M)   | 54796      (50K)  |
|critical             | 4            | 5              | 2000000000 (2G)   |

## Commands

Run the following commands in a Linux environment to compile and run the program from the root directory:

- To compile sequential version: 
`./scripts/run_FF_seq.sh [input file] [output file]`  
Example: `./scripts/run_FF_seq.sh airports_500_dag.txt output.txt`
- To run sequential version:
`./src/MaxFlowSolver ./input/[input file] ./output/[output file]`  
Example: `./src/MaxFlowSolver ./inputs/dag_1000_6000.txt ./output/output.txt`  

- To compile parallel version: 
`./scripts/run_FF_par.sh`  
Example: `./scripts/run_FF_par.sh airports_500_dag.txt output.txt`  
- To run parallel version:
`./src/MaxFlowSolverPP ./input/[input file] ./output/[output file]`  
Example: `./src/MaxFlowSolverPP ./inputs/dag_1000_6000.txt ./output/output.txt`

## Ford-Fulkerson - SEQUENTIAL Version

### List of classes

#### Class MaxFlowSolver

The `MaxFlowSolver` class is designed to solve the maximum flow problem in a flow network using the Ford-Fulkerson method with depth-first search (DFS).

##### Attributes

1. `n`: The number of nodes in the graph.
2. `s`: The source node, it is assumed to be at index 0.
3. `t`: The sink node, it is assumed to be at index n-1.
4. `input_file_path`: The file path to read the graph from.
5. `max_flow`: The maximum flow value to compute, initialized to -1.
6. `graph`: The graph represented as an adjacency list, stored in a vector of lists of Edge pointers.
7. `visit_flag`: An integer flag used for visiting nodes, initialized to 1.
8. `visited`: A vector of integers to keep track of visited nodes.
9. `solved`: A boolean indicating whether the network flow algorithm has run, initialized to false.
10. `INF`: A constant representing infinity, used to avoid overflow, calculated as half of the maximum value of a long integer.

##### Methods

1. Constructor (`MaxFlowSolver(string input_file_path)`):
    1. Sets the max_flow to 0
    2. Sets the solved flag to false
    3. Reads the graph from the input file using the readGraph() method
    4. Initializes a visited vector with size n (number of nodes)
    5. Assumes a source node at index 0 and a sink node at index n-1

2. `getMaxFlow()`: Returns the maximum flow value computed by the solver.

3. `readGraph()`: reads a graph from a file specified by input_file_path and stores it in an adjacency list.
    1. It opens the file and checks if it's successfully opened. If not, it prints an error message and returns an empty graph.
    2. It reads the first line of the file, which is assumed to contain the number of nodes in the graph.
    3. It then reads the rest of the file line by line, where each line is assumed to be in the format "node1 node2 capacity".
    4. For each line, it creates two Edge objects: one for the forward edge and one for the residual edge (with zero capacity).
    5. It sets the residual edge of the forward edge and vice versa.
    6. It adds the forward edge to the adjacency list of the start node and the residual edge to the adjacency list of the end node.
    7. Finally, it returns the constructed adjacency list.

4. `printGraph()`: prints the graph, excluding residual edges, in the format "edge - edge, capacity".
It iterates over each node in the graph and then over each edge connected to that node. If the edge is not a residual edge (!edge->isResidual()), it prints the edge's details using the toString() method.

5. `printGraphToFile(string fout)`: writes the non-residual edges of a graph to a file specified by fout. It opens the file, checks if it's successfully opened, and then iterates over each node and edge in the graph, writing the edge's string representation to the file if it's not a residual edge.

6. `printGraphResidual()`: prints the residual graph, but it seems to have a bug. It is supposed to print the residual capacity of each edge, but it calls toString() which is not defined in the provided context. It should likely call a function that returns a string representation of the edge with its residual capacity.

7. `setVisited(int i)`: marks a node i as visited in the graph by setting its corresponding value in the visited array to the current visit_flag.

8. `isVisited(int i)`: checks if a node i has been visited by comparing its visited status (visited[i]) to the current visit flag (visit_flag). If they match, the node has been visited, and the function returns true. Otherwise, it returns false.

9. `markAllNodesAsUnvisited()`: This function increments a flag (visit_flag) to mark all nodes in a graph as unvisited. The idea is that each node checks this flag to determine if it has been visited or not. By incrementing the flag, all nodes are effectively marked as unvisited.

10. `solve()`: computes the maximum flow in a flow network using the Ford-Fulkerson algorithm with Depth-First Search (DFS).
    1. It starts by calling the dfs() function from the source node (this->s) with an initial flow of infinity (INF).
    2. As long as the dfs() function returns a non-zero flow (f), it increments the visit_flag and adds the flow to the total max_flow.
    3. The dfs() function is called repeatedly from the source node until no more augmenting paths are found (i.e., f becomes 0).  
This process effectively finds and augments all possible paths from the source to the sink in the flow network, resulting in the maximum possible flow.

11. `dfs(int node, long flow)`: Performs a depth-first search to find augmenting paths. It recursively explores the graph, starting from a given node, to find an augmenting path from the source to the sink.
    1. If the current node is the sink (this->t), it returns the current flow.
    2. Marks the current node as visited.
    3. Iterates over all edges connected to the current node.
    4. For each edge, checks if it has remaining capacity and if the end node has not been visited yet.
    5. If both conditions are true, recursively calls dfs on the end node with the minimum of the current flow and the edge's remaining capacity.
    6. If the recursive call returns a positive flow (i.e., an augmenting path is found), augments the flow on the current edge and returns the flow.
    7. If no augmenting path is found, returns 0.

12. `bfs()`: Currently not implemented (returns 0).

#### Class Edge

The `Edge` class represents a directed edge in a flow network, with properties such as start node, end node, capacity, flow, and residual edge.

##### Attributes

1. `start_node`: The starting node of the edge.
2. `end_node`: The ending node of the edge.
3. `flow`: The current flow value of the edge, initialized to 0.
4. `capacity`: The maximum capacity of the edge.
5. `residual`: A pointer to the residual edge, created only when a path is found.
6. `hasThread`: An atomic boolean flag indicating whether a thread has been generated for the edge, initialized to false.

##### Methods

1. Constructor `Edge(int start_node, int end_node, long capacity)`: Initializes an edge with the given start node, end node, and capacity.
2. `getStartNode()`: Returns the start node of the edge.
3. `getEndNode()`: Returns the end node of the edge.
4. `setResidual(Edge *edge_residual)`: Sets the residual edge of the current edge.
5. `isResidual()`: Checks if the edge belongs to the residual network (i.e., its capacity is 0).
6. `getCapacity()`: Returns the capacity of the edge.
7. `getFlow()`: Returns the current flow of the edge.
8. `getRemainingCapacity()`: Returns the remaining capacity of the edge (i.e., `capacity - flow`).
9. `augment(long bottleneck)`: Adds more flow to the edge and updates the residual edge accordingly.
10. `toString()`: Returns a string representation of the edge in the format `node1 -> node2, Flow: flow, Cap: capacity`.
11. `toStringFile()`: Returns a string representation of the edge in a file-friendly format.
12. `getHasThread()`: Returns the value of the `hasThread` flag.
13. `setHasThread()`: Sets the `hasThread` flag to true.  


## Ford-Fulkerson -  PARALLEL Version

### List of classes

#### Class MaxFlowSolverParallelPool

A class to solve the maximum flow problem in parallel by using a thread pool to coordinate multiple threads

##### Attributes

1. `n`: The number of nodes in the graph.
2. `s, t`: The source and sink nodes.
3. `input_file_path`: The path to the input file.
4. `max_flow`: The maximum flow value computed.
5. `graph`: The graph represented as an adjacency list.
6. `visited`: A vector to keep track of visited nodes.
7. `augmenter_thread_exists`: A flag to indicate if the augmenter thread exists.
8. `solved`: A flag to indicate if the network flow algorithm has run.
9. `INF`: A constant to avoid overflow.
10. `graph_lock`: A mutex to lock the graph.
11. `cv`, `cv_augment`, `cv_nodes`: Condition variables for synchronization.
12. `mx_cv, mx_cv_augment`: Mutexes for the condition variables.
13. `threads`: A vector of threads.
14. `num_generated, num_blocked, num_waiting_label, num_running`: Atomic integers to keep track of thread counts.
15. `pending_jobs`: An atomic integer to keep track of pending jobs.
16. `start`: A reference time instant.
17. `nodes`: A vector of Node pointers.
18. `done, sink_reached`: Atomic booleans to indicate completion and sink reachability.
19. `visit_flag`: An integer to keep track of visit flags.
20. `threads`: A vector of threads.
21. `mx_print`: A mutex to lock printing.
22. `mng`, `mnb`, `mx_node`: Mutexes.
23. `start`: A reference time instant.
24. `pending_jobs`: An atomic integer to keep track of pending jobs.

##### Methods

1. Constructor `MaxFlowSolverParallelPool(string input_file_path)`: It initializes the object's member variables with default values, including:
    1. `input_file_path`: set to the provided input file path
    2. `max_flow`: set to 0
    3. `solved`: set to false (indicating the solver has not run yet)
    4. `graph`: populated by reading a graph from a file (using the readGraph() function)
    5. `visited`: initialized as a vector of integers with size n (number of nodes)
    6. Various atomic flags (`done`, `sink_reached`, `num_generated`, etc.): set to false or 0
    7. It also assumes a source node (index 0) and a sink node (index n-1) in the graph.
2. `long getMaxFlow()`:  returns the value of the max_flow variable. The `this` keyword refers to the current object and  `max_flow` is a member variable of the object.
3. `vector<list<Edge *>> readGraph()`: reads a graph from a file and stores it in an adjacency list representation:
    1. It opens the file specified by input_file_path.
    2. If the file cannot be opened, it prints an error message and returns an empty list.
    3. It reads the first line of the file, which is expected to contain the number of nodes in the graph.
    4. It then reads the remaining lines of the file, each of which is expected to contain three integers: the start node, end node, and capacity of an edge.
    5. For each edge, it creates two Edge objects: one for the forward edge and one for the residual edge (with zero capacity).
    6. It adds these edges to the adjacency list representation of the graph.
    7. It also creates Node objects for each node in the graph, if they don't already exist.
    8. Finally, it returns the adjacency list representation of the graph.
4. `void printGraph()`: prints the original graph, excluding residual edges, in the format `edge - edge, capacity`. It iterates over each node in the graph and then over each edge connected to that node. If the edge is not a residual edge (i.e., `edge->isResidual()` returns `false`), it prints the edge's details using the `toString()` method.
5. `void printGraphToFile(string fout)`: writes the non-residual edges of a graph to a file. It takes a file name as input, opens the file, and writes each non-residual edge to the file in a specific format (`edge->toStringFile()`). If the file cannot be opened, it prints an error message and exits the function.
6. `void printGraphResidual()`: prints the residual graph, where each edge is displayed with its residual capacity. However, it does not actually filter out non-residual edges or print the residual capacity specifically. It simply prints all edges in the graph, relying on the toString() method of the Edge class to format the output.
7. `void setVisited(int i)`: marks a node i as visited by setting its corresponding value in the visited array to the current `visit_flag`. The `visit_flag` is used to keep track of the current traversal or iteration, allowing the function to distinguish between visited nodes in different traversals.
8. `bool isVisited(int i)`: checks if a node i has been visited by comparing its visited status (`this->visited[i]`) with the current visit flag (`this->visit_flag`). If they match, the node has been visited, and the function returns `true`; otherwise, it returns `false`.
9. `void markAllNodesAsUnvisited()`: increments a flag (`visit_flag`) to mark all nodes as unvisited. 
10. `auto computeTime()`: calculates the time elapsed since a certain starting point (`this->start`) in nanoseconds.
11. `void thread_function(ThreadPool &thread_pool, int u, int v, Edge *edge)`: a recursive function that is executed by multiple threads in parallel. Each thread is responsible for exploring a portion of the graph and updating the flow values.
    1. The function takes a ThreadPool object, two node IDs u and v, and an Edge object as input.
    2. It checks if the sink node has been reached or if the current node is the source node. If either condition is true, it returns immediately.
    3. It locks the nodes u and v using a shared mutex to ensure thread safety.
    4. It attempts to assign a label to the nodes u and v using the assign_label function. If the label cannot be assigned, it unlocks the nodes and returns.
    5. If the label is assigned successfully, it checks if the sink node has been reached. If so, it sets the sink_reached flag to true and returns.
    6. If the sink node has not been reached, it explores the neighbors of node v and checks if any of them have remaining capacity and are not labeled. If such a neighbor is found, it creates a new job to explore that neighbor and adds it to the thread pool.
    7. Finally, it unlocks the nodes u and v and returns.
The code uses a recursive approach to explore the graph, with each thread creating new jobs to explore neighboring nodes. The `assign_label` function is used to update the flow values and labels of the nodes. The `sink_reached` flag is used to indicate when the sink node has been reached, at which point the algorithm can terminate.
12. `bool assign_label(Node *n_u, Node *n_v, Edge *edge)`:It assigns a label to a node (`n_v`) based on the label of another node (`n_u`) and the edge between them. The label assignment depends on the following conditions:
    1. If `n_u` is labeled and `n_v` is not, the function checks if the edge has remaining capacity. If it does, it assigns a label to `n_v` with a flow value equal to the minimum of `n_u`'s flow and the edge's remaining capacity.
    2. If `n_v ` is labeled and `n_u` is not, the function checks if the residual edge has a flow greater than 0. If it does, it assigns a label to `n_u` with a flow value equal to the minimum of `n_v`'s flow and the negative of the edge's flow.
The function returns `true` if a label is assigned and `false` otherwise.
13. `void solve()`: The solve function is the main entry point for the algorithm.
    1. Initialization: The function starts by setting the label of the source node and creating a thread pool.
    2. Job creation: It then creates jobs for each edge of the source node with remaining capacity and adds them to the thread pool.
    3. Thread pool execution: The thread pool executes the jobs concurrently, and each job runs the `thread_function`.
    4. Waiting for completion: The main thread waits for all jobs to complete and checks if the sink has been reached.
    5. Augmentation: If the sink has been reached, the algorithm augments the flow along the path found by the threads.
    6. Reset and repeat: The algorithm resets the labels, clears the queue, and wakes up the threads to repeat the process until no more augmenting paths are found.
    7. Cleanup: Finally, the function stops the thread pool, frees resources, and deletes the nodes.
The code uses a thread pool to parallelize the execution of the algorithm, which can significantly improve performance for large flow networks.
14. `long augment()`: It updates the flow along an augmenting path from the sink node `t` to the source node `s`. It iteratively updates the flow on each edge of the path, adding the minimum flow `sink_flow` to the forward edges and subtracting it from the backward edges, effectively augmenting the flow along the path. The function returns the augmented flow `sink_flow`.
15. `bool sinkCapacityLeft()`: checks if there is any edge with remaining capacity that points to the sink node (`this->t`) in the graph. If such an edge is found, it immediately returns `true`. If no such edge is found after checking all edges, it returns `false`.
16. `bool sourceCapacityLeft()`: checks if there is any edge originating from the source node (`this->s`) with remaining capacity greater than 0. If such an edge exists, it immediately returns `true`. If no such edge is found after checking all edges from the source node, it returns `false`. 
17. `void resetLabels()`: resets the labels of all the nodes in a graph, except for the source node. It iterates over all the nodes in the graph and checks if the current node is not the source node and if the node has a label. If both conditions are true, it calls the `resetLabel()` function on the node. Finally, it sets the value of `sink_reached` to `false`.

#### Class ThreadPool

##### Attributes

1. `should_terminate`: a boolean indicating whether the thread pool should terminate or not.
2. `queue_mutex`: a mutex (mutual exclusion lock) used to protect access to the job queue.
3. `mutex_condition`: a condition variable used to signal threads waiting on the job queue.
4. `threads`: a vector of threads that make up the thread pool.
5. `jobs`: a queue of jobs (functions) that are waiting to be executed by the thread pool.
6. `cv_completion`: a condition variable used to signal threads waiting for completion.
7. `completion_mutex`: a mutex (mutual exclusion lock) used to protect access to the completion condition variable.
8. `active_tasks`: an atomic integer indicating the number of active tasks in the thread pool.
9. `monitor`: a ThreadMonitor object used to monitor the thread pool.

##### Methods

1. `void Start()`: initializes a thread pool by creating a number of threads equal to the system's hardware concurrency (i.e., the number of CPU cores). Each thread is set to execute the `ThreadLoop` function, which is a member function of the ThreadPool class.
2. `void ThreadLoop()`: This is the main loop of a thread in a thread pool. It continuously checks for jobs in the queue and executes them.
    1. The thread waits on a condition variable `mutex_condition` until a job is available in the queue (`jobs.empty()` is `false`) or termination is requested (should_terminate is `true`).
    2. If termination is requested and no jobs are available, the thread exits the loop.
    3. Otherwise, the thread dequeues a job from the queue and executes it.
    4. After executing the job, the thread decrements the active_tasks counter. If no more tasks are active, it notifies all waiting threads using cv_completion.
In essence, this loop ensures that threads in the pool remain idle until work is available, and then execute jobs efficiently while handling termination requests.
3. `void QueueJob(const std::function<void()> &job)`: It takes a `std::function<void()>` as a parameter, which represents a function that takes no arguments and returns void. The method first acquires a lock on a `std::mutex` named `queue_mutex` to ensure thread safety. It then checks if the `should_terminate` flag is set to `true`. If it is, the method returns immediately, preventing further job addition. If the `should_terminate` flag is `false`, the method pushes the job function onto a queue (`jobs.push(job)`) and increments the `active_tasks` counter. After that, it notifies a `std::condition_variable` named `mutex_condition` to signal that a job has been added to the queue.
4. `void Stop()`: stops the thread pool by:
    1. Setting a termination flag (`should_terminate`) while holding the `queue_mutex` lock.
    2. Notifying all waiting threads using `mutex_condition.notify_all()`.
    3. Joining (waiting for) each active thread in the threads vector to finish.
    4. Clearing the threads vector.
This ensures that all threads in the pool are properly terminated and cleaned up.
5. `bool busy()`: checks if the thread pool has any pending jobs in its queue. It does this by acquiring a lock on the `queue_mutex`, checking if the jobs queue is empty, and returning the opposite of that result (i.e., `true` if the queue is not empty, `false` otherwise).
6. `void clearQueue()`: clears the job queue in a thread pool by swapping it with an empty queue, while ensuring thread safety through a mutex lock.
7. `void waitForCompletion()`: waits until all jobs in the thread pool are completed.
    1. It checks if there are no jobs in the queue (`jobs.empty()`) and no active tasks (`active_tasks.load() == 0`) under the `queue_mutex` lock. If both conditions are `true`, it returns immediately.
    2. If there are jobs or active tasks, it unlocks the `queue_mutex` and locks the `completion_mutex`.
    3. It then waits on the `cv_completion` condition variable under the `completion_mutex` lock until notified by another thread.
The purpose of this function is to block the calling thread until all tasks in the thread pool are finished, allowing for synchronization and ensuring that all work is completed before proceeding.
8. `int getActiveThreads()`: returns the number of active tasks (i.e., threads currently executing jobs) in the thread pool.
9. `void notify()`: notifies all threads waiting on the mutex_condition condition variable, allowing them to wake up and continue execution.


#### Class Node

The `Node` class represents a node in a graph, with attributes such as an ID, a label, and synchronization primitives for thread-safety.

##### Attributes

1. `id`: an integer representing the node's ID.
2. `label`: a pointer to a label_t struct, which contains the node's label information: `pred_id` (an integer representing the predecessor node ID), `sign` (an integer representing the predecessor node ID), and `flow` (a long integer representing the flow value associated with the node).
3. `labeled`: an atomic boolean indicating whether the node has been labeled or not.
4. `mx_node`: a mutex (mutual exclusion lock) used to protect access to the node's data.
5. `mx_cv`: a condition variable used to signal threads waiting on the node's condition variable.
6. `cv`: a condition variable used to wait on the node's condition variable.

##### Methods

1. Constructor `Node(int id)`: Initializes a new node with the given ID and sets its label to an empty struct.
2. `getId()`: Returns the node's ID.
3. `setLabel(int pred_id, char sign, long labelflow)`: Sets the node's label based on the given predecessor node ID, sign, and flow value.
4. `resetLabel()`: Resets the node's label to an empty struct and sets the labeled flag to false.
5. `freeLabel()`: Frees the memory allocated for the node's label.
6. `setSourceLabel()`: Sets the node's label as a source node with a predecessor ID of -1, sign of NULL, and flow value of INF.
7. `isSource()`: Returns true if the node is a source node (i.e., its predecessor ID is -1).
8. `isSink(int t)`: Returns true if the node is a sink node (i.e., its ID is equal to the given sink node ID).
9. `getLabel()`: Returns a pointer to the node's label struct.
10. `isLabeled()`: Returns true if the node has a label set.
11. `lockSharedMutex()`: Acquires a shared lock on the node's mutex.
12. `unlockSharedMutex()`: Releases a shared lock on the node's mutex.
13. `waitOnNodeCV()`: Waits on the node's condition variable until the node is no longer labeled.
14. `signalNodeCV()`: Signals the node's condition variable to all waiting threads, allowing them to proceed.

#### Class Edge

The `Edge` class used in the parallel version of the Max Flow algorithm is the same class used for the sequential version. See above for further details.


## Tests Results and Performance Analysis

### Results

The following table provides a summary of the time (in microseconds) taken by the sequential and parallel versions of the Max Flow algorithm on different datasets.

| Dataset             | # Nodes      | # Edges        | Max Flow          | Time (micros) - Sequential | Time (micros) - Parallel (8 threads)|
|:--------------------|-------------:|---------------:| ----------------: | -------------------------: | ----------------------------------: |
|second_test          | 3            | 3              | 3                 | 2                          | 2804      (3 ms)                    |
|florida              | 7            | 5              | 7                 | 7                          | 4439      (4 ms)                    |
|airports_500_dag     | 500   (0.5K) | 2980    (0.5K) | 13926      (14K)  | 3026         (3.03 s)      | 21802     (22 s)                    |
|dag_1000_2000        | 1000  (1K)   | 2000    (2K)   | 14                | 141                        | 6919      (7 ms)                    |
|dag_1000_3000        | 1000  (1K)   | 2987    (3K)   | 51                | 745                        | 7186      (7 ms)                    |
|dag_1000_6000        | 1000  (1K)   | 6000    (6K)   | 142        (0.1K) | 17665        (17.6 ms)     | 68250     (68 ms)                   |   
|dag_1000_60000       | 1000  (1K)   | 59959   (60K)  | 5299       (5K)   | 132451986    (2.2 min)     | 166779    (166 ms)                  |
|dag_1000_30000       | 1000  (1K)   | 29097   (30K)  | 2173       (2K)   | 22422245     (22.42 s)     | 3667745   (3.67s)                   |
|dag_1000_300000      | 1000  (1K)   | 225563  (250K) | 20108      (20K)  | 1937035051   (32.2 min)    | 268964465 (4.48 min)                |
|dag_5000_30000       | 5000  (5K)   | 29964   (30K)  | 307        (0.3K) | 368566       (0.37 s)      | 369560    (0.37 s)                  |
|dag_5000_60000       | 5000  (5K)   | 59837   (60K)  | 731        (0.7K) | 10256613     (10.25 s)     | 2602841   (2.60 s)                  |
|dag_5000_600000      | 5000  (5K)   | 585715  (600K) | 10434      (10K)  |                            | 500135551 (8.33 min)                |
|dag_10000_60000      | 10000 (10K)  | 59959   (60K)  | 133        (0.1K) | 91574        (92 ms)       | 166779    (166 ms)                  |
|dag_10000_600000     | 10000 (10K)  | 596388  (600K) | 5076       (5K)   | 1819786709   (30.3 min)    | 293312595 (4.88 min)                |
|dag_10000_1000000    | 10000 (10K)  | 989896  (1M)   | 9168       (9K)   |                            | 890133587 (14.8 mins)               |
|dag_10000_6000000    | 10000 (10K)  | 5654501 (6M)   | 54796      (50K)  |                            |                                     |
|critical             | 4            | 5              | 2000000000 (2G)   | 8                          | 2847      (3 ms)                    |

The tests were run on a machine with the following specifications:
- Processor: Intel(R) Core(TM) i7-1065G7 CPU @ 1.30GHz
- RAM: 16 GB
- OS: Ubuntu (WSL 2 on Windows 11)  

### Performance Analysis
