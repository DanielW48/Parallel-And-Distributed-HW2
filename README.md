## Problem 1
We will implement the strategy similar to that of the one we went over with the room and the switch in class.

We will assign the task of "counting" the number of people who have certainly visited the labyrinth to the first thread. All other threads besides the first thread will follow the following procedure: when they enter the labyrinth, if and only if this is their first time seeing a cupcake at the end of the labyrinth, they will eat it and leave the spot empty. Otherwise they will leave it the way they found it. If they show up for the first few times and there is no cupcake, then they will leave the spot be. Also if they show up and there is a cupcake but they have already eaten one before, they will leave the cupcake there for someone else to find.

The process of the first thread will be as follows: if they get to the end of the labyrinth and there is no cupcake in the spot, then they will add one to their mental counter, request a new cupcake be placed, leave it alone, and leave. Otherwise, they will leave it the way they found it. When the first thread's mental counter reaches exactly N - 1, then he knows we are done and can alert the minotaur.

This process ensures that each person that eats a cupcake for their first time is correctly counted exactly once by the first thread. Since each person only eats at most one cupcake, then all N - 1 in the first thread's mental counter represent N - 1 different and distinct people that have already visited the labyrinth, the first thread can be sure that everyone has been at least once. When N is greater than 1, we know that in order for the counter to reach N - 1, the first thread will have to have visited the labyrinth at least once to change the counter from 0.

This strategy fails when N is exactly equal to 1, but a simpler strategy can be crafted for a case like this, in which the first thread would alert the minotaur after his first visit to the labyrinth.

In the code, we use a condition_variable and a mutex to alert threads of when they should be running certain processes. The minotaur will continue to assign random turns to people, perhaps the same people and in any order. This will be represented by the main thread. This thread will use a condition_variable to wait for the condition that the current turn variable will be reset to -1. Then it will assign a current turn, change its value, and notify all other threads to check to see if it is their turn. For the other threads, their process will be to continuously wait for their turn or until the game has finished using a similar conditon_variable. when it is their turn, they will run their procedure, then reset the current turn variable to -1 (perhaps also set a flag to signal that the game is over if this thread is the first thread), and then notify all other threads of the changes.


## Problem 2
