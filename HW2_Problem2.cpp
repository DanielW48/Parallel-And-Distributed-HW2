#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <iostream>
#include <cassert>
#include <atomic>
using namespace std;

const int NUM_THREADS = 10;
const int INIT_QUEUE_SIZE = 50;
const bool PRINT_OUTPUT = false;

mutex showroomMutex;
condition_variable cv;
queue<int> showroomQueue;

atomic<bool> ready;
atomic<int> currentTurn;
bool finished;

void doThreadProcess(int threadIdx){
    while(true){
        // wait for main thread to be ready, and for either the showroom visits to finish
        // or it to be our turn
        unique_lock<mutex> myLock(showroomMutex);
        cv.wait(myLock, [&]{ return ready && (finished || currentTurn == threadIdx); });

        // break our loop if visits have finished
        if(finished){
            break;
        }

        // this is unecessary, but helps make sure our algorithm is working as we expect
        assert(currentTurn == threadIdx && threadIdx == showroomQueue.front());

        // take ourselves out of the queue
        showroomQueue.pop();

        // check out the vase
        if(PRINT_OUTPUT){
            cout << threadIdx << " is checking out the vase" << "\n";
        }

        // if there is another guy in the queue still waiting, make it his turn
        if(showroomQueue.size() > 0){
            currentTurn = showroomQueue.front();
        }
        // otherwise, we are finished and it is nobody's turn, so update to be so
        else{
            finished = true;
            currentTurn = -1;
        }

        myLock.unlock();
        cv.notify_all();
    }
}

int main(){
    // set our globals
    ready = false;
    currentTurn = -1;
    finished = false;

    // make our threads
    vector<thread*> ourThreads(NUM_THREADS);
    for(int i = 0; i < NUM_THREADS; ++i){
        ourThreads[i] = new thread(doThreadProcess, i);
    }

    // set up our queue
    {
        lock_guard<mutex> setupLock(showroomMutex);
        // initialize the queue with some random guys
        for(int i = 0; i < INIT_QUEUE_SIZE; ++i){
            int guy = rand() % NUM_THREADS;
            showroomQueue.push(guy);
        }
        
        // the initial value of current turn will be the first guy in the queue;
        // since nobody comes before him, he must be alerted by the minotaur (our
        // main thread)
        if(showroomQueue.size() > 0){
            currentTurn = showroomQueue.front();
        }
        else{
            finished = true;
            currentTurn = -1;
        }

        // ready up
        ready = true;

        // unlock and notify our guests
    }
    cv.notify_all();

    // now we will wait for our queue to empty; since our threads take care of
    // notifying each other and alerting the next in line, we do not need to
    // do anything as the main thread except wait;
    // the last guy in line will be in charge of telling the minotaur that everyone
    // is done
    unique_lock<mutex> minotaurLock(showroomMutex);
    cv.wait(minotaurLock, []{ return finished; });
    
    // unlock as we have no need to keep this mutex locked now
    minotaurLock.unlock();

    if(PRINT_OUTPUT){
        cout << "visits to the showroom are complete." << "\n";
    }

    // join em up
    for(int i = 0; i < NUM_THREADS; ++i){
        ourThreads[i]->join();
    }

    return 0;
}