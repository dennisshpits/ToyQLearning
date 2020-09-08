# ToyQLearning
small RL project to show how tabular QLearing works on a grid world

## Build Steps
You can either run make directly or you can execute the bash script:
```bash
./build.sh
```
This will produce an executable binary that you can run which will output a q-table.

## Background Information
We will implement an example program using Matrices to show how **Tabular** QLearning works. We create a grid world. All possible states are finite in that you can only be in a single cell in the grid. The state vector is one dimensional and discrete. 

### Basic Explanation
After an agent performs an action in an Enviornment, it will have a new State and will be given a reward.
We want to create an optimal value function for each state in this world. We call this v*.

### Optimal state-value functions
The value of a state under the optimal policy is the max of that state under any policy. Optimal value of the state maximizes the reward that I get right away plus the optimal value of the state I end up in.

### q function
q(s,a) = (1-alpha) * q(s,a) + alpha * (r(s,a) + (Ymaxa,q(s',a'))) where (Ymaxa,q(s',a')) is the maximum amount of return based on the q table for an action. **The future is built into the equation through this.** r is the reward function.
