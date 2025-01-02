#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

using namespace std;

int n = 20;
atomic<bool> done(false); // Use atomic to ensure thread-safe access

class Object {
private:
    mutex m;
public:
    bool label = false;

     bool tryLabel(int thread_id, int obj_id) {
        if (m.try_lock()) { // Attempt to acquire the lock
            lock_guard<mutex> l(m, adopt_lock); // Adopt the lock (ensure it gets released)
            if (!label) {
                cout << "Thread " << thread_id << " labeling object " << obj_id << endl;
                label = true; // Label the object
                return true;  // Successfully labeled
            } else {
                cout << "Thread " << thread_id << " found object " << obj_id << " already labeled" << endl;
            }
        } else {
            cout << "Thread " << thread_id << " skipped object " << obj_id << " (locked)" << endl;
        }
        return false; // Lock was not acquired or object was already labeled
    }
};

void fthread(int thread_id, vector<Object>& objs);

int main() {
    vector<Object> objs(n);
    vector<thread> threads;
    int max = std::thread::hardware_concurrency();
    cout << max << endl;

    for (int i = 0; i < 9; ++i) {
        threads.emplace_back(fthread, i, ref(objs)); // Pass objects by reference
    }

    for (auto& t : threads) {
        t.join();
    }

    cout << "All threads have finished." << endl;
    return 0;
}

void fthread(int thread_id, vector<Object>& objs) {
    for (int x = 0; x < objs.size(); ++x) {
        if (done.load()) { // Check if another thread has finished
            cout << "Thread " << thread_id << " stopping early." << endl;
            return;
        }
        cout << "Thread " << thread_id << " trying to label object " << x << endl;
        if ( objs[x].tryLabel(thread_id, x)) {
            cout << "Thread " << thread_id << " successfully labeled object " << x << endl;
           // this_thread::sleep_for(chrono::seconds(1));
        };
    }

  
    cout << "Thread " << thread_id << " completed labeling." << endl;
    done.store(true); // Signal other threads to stop
    cout << "Thread " << thread_id << " finished all objects." << endl;
    return;
}


/*
// GLOBAL
vector<pthread_t> thread_ids;


// NEI THREAD CHE CREANO ALTRI THREAD
//Crea un thread e salva l'id
pthread_t tid;
if (!pthread_create(&tid, NULL, fthread, NULL)) {
    thread_ids.push_back(tid);
};

// NEL THREAD CHE HA RAGGIUNTO IL SINK
//Rimuove l'id dal vettore
thread_ids.erase(find(v.begin(), v.end(), pthread_self()));

//Cancella tutti gli altri thread creati
for (pthread_t tid : thread_ids) {
    if (pthread_cancel(tid)) {
        //Errore in cancellazione qua
    } else {
        //Tutto a posto :D
    }
}

*/


/*

T1                  cv=False                   T2
(sink)                                      (node x)
  .             if sink reached:                .
  .                 signal(cv) // cv=True
  .                  augment()
                else:
                    if (!cv):                    .
                        i1                       .
                    else:
                        return
                    if (!cv):                    .
                        i2                       . (ongoing)
                    else:
                        return
                    if (!cv)                     .
                        i3
                    else:
                        return                   .

// Notation: start -> end
// check if flow < capacity
// check if end node has label



// BEGIN
atomic<bool> done

while (!done) {
    check if start node U has label
        if yes -> continue
        if no  -> wait for some other thread to set the label on the start node -> CV
    check if end node V has label
        if yes -> return
        if no  -> try to lock it
            if lock successful:
                check flow (U,V)< capacity (U,V)  || check reverse_flow i.e. flow(V,U) > 0
                compute label accordingly
                assign label to node
                unlock mutex
                signal conditional variable
            else remain blocked on the mutex till it gets unlocked
    check if end node is sink
        if yes ->
            if augment():
                reset labels
            else:
                done = true // no more
            next_iteration.notify_all() wake all threads (to restart)                    
        if no  -> 
            for edge (u,v) in edges(end_node, neighbours(u())
                if v is labeled
                    continue
                if v is not labeled
                    generate threads for (u,v)
            wait on CV of next_iteration
}
return












    mutex m
    conditional_variable cv

    function(){
        unique_lock<mutex> l(m);
        cv.wait(l, lambda function)
    }

*/




/***
// GENERIC ALGORITHM
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