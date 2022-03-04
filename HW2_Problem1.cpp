#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <iostream>
#include <atomic>
using namespace std;

const int NUM_THREADS = 75;
const bool PRINT_OUTPUT = false;

// we will keep some global variables to keep track of the condition of things,
// and also a mutex and a condition_variable to help us govern things and to
// notify threads of changes in the variables
mutex labyrinthMutex;
condition_variable cv;
bool cupcakeExists;
bool grabbedCupcake[NUM_THREADS];
int numSeenByFirst;

// we will make these variables atomic to keep the operations on them safer,
// as these are the variables used in predicates of some threads' wait calls
atomic<bool> finished;
atomic<int> currentTurn;

void doThreadProcess(int threadIdx){
    // execute this code while we have not verified that every thread has seen the labyrinth
    while(true){
        // wait our turn
        unique_lock<mutex> myLock(labyrinthMutex);
        cv.wait(myLock, [&]{ return finished || currentTurn == threadIdx; });

        // some threads will be waiting for their next turn, but it will not come as the
        // task has been completed; break our loop here if this is the case
        if(finished){
            break;
        }

        // we are done waiting and the game has not finished, so it must be our turn:
        // make our changes:

        // if we are the first thread, we will be responsible for counting
        // the number of threads that have eaten a cupcake; our strategy
        // will ensure that each thread eats a cupcake at most once, so when
        // our count reaches NUM_THREADS - 1, we know we have all visited the
        // labyrinth at least once
        if(threadIdx == 0){
            if(!cupcakeExists){
                // this means that someone has eaten a cupcake; increment our counter
                ++numSeenByFirst;
                // set up a new cupcake
                cupcakeExists = true;
                // update if we have finished everything
                if(numSeenByFirst == NUM_THREADS - 1){
                    finished = true;
                }

                if(PRINT_OUTPUT){
                    cout << "\t" << threadIdx << " sees a missing cupcake and increments counter to " << numSeenByFirst << "\n";
                }
            }
        }
        // otherwise, we are a normal thread
        else{
            // if there is a cupcake here and this thread has
            // yet to ever grab a cupcake, grab it
            if(cupcakeExists && !grabbedCupcake[threadIdx]){
                // eat cupcake
                cupcakeExists = false;
                // store that we have grabbed one
                grabbedCupcake[threadIdx] = true;

                if(PRINT_OUTPUT){
                    cout << "\t" << threadIdx << " is eating a cake" << "\n";
                }
            }
        }

        // set currentTurn back to -1 before unlocking
        currentTurn = -1;

        // we have just finished in the labyrinth, notify the other threads
        myLock.unlock();
        cv.notify_all();
    }
}

int main(){
    // set up our initial values for our global variables:
    // it is safe to mess with these variables as we haven't created our threads yet

    // a cupcake starts in the labyrinth
    cupcakeExists = true;
    // our current turn will start at -1 so when creating our threads, no thread
    // jumps the gun before it is actually assigned to enter the labyrinth by the
    // minotaur
    currentTurn = -1;
    // nobody has grabbed a cake yet
    fill(begin(grabbedCupcake), end(grabbedCupcake), false);
    // first thread has seen no missing cupcakes yet
    numSeenByFirst = 0;
    // we are not finished
    finished = false;

    // we will now create our threads
    vector<thread*> ourThreads(NUM_THREADS);
    for(int i = 0; i < NUM_THREADS; ++i){
        ourThreads[i] = new thread(doThreadProcess, i);
    }

    // this main thread will act as the minotaur; we will randomly assign a current
    // turn, allowing that thread into the labyrinth; once they have exited the
    // labyrinth, we will reassign a current turn and repeat; our stopping condition
    // will be when the guests tell us that they have all definitively been in the
    // labyrinth at least once, which will be indicated by our global *finished* flag
    while(true){
        // we will wait for our current turn to be reset back to -1 before reassigning
        // the turn to someone else; if current turn is not -1, this means someone is
        // currently in the labyrinth
        unique_lock<mutex> myLock(labyrinthMutex);
        cv.wait(myLock, []{ return currentTurn == -1; });

        // if the game has finished, stop asssigning turns
        if(finished){
            break;
        }

        // assign a random person between [0, NUM_THREADS - 1] a turn in the labyrinth
        currentTurn = rand() % NUM_THREADS;
        if(PRINT_OUTPUT){
            cout << "current turn: " << currentTurn << "\n";
        }

        // we have finished changing our shared global variables, unlock lock and notify
        // the other threads to check their predicates
        myLock.unlock();
        cv.notify_all();
    }

    // we have finished everything, join all threads
    for(int i = 0; i < NUM_THREADS; ++i){
        ourThreads[i]->join();
    }

    return 0;
}