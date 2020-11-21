# ProcessMemoryManagement

The project simulate different process scheduling and memory management that mimic the process allocation of a CPU and manage memory allocation among running process.

The Program is written in C and implemented based on the algorithm below

Scheduling algorithm:
- First-come first-served (ff)
- Round-robin (rr)
- shortest remaining job first (cs)

Memory Management:
- Unlimited Memory (u)
- Swapping-X (p)
- Virtual Memory (v)
- Most Frequently Used (vm)

## Program Execution

The run the program, clone the repository and use the linux terminal to compile the program.
A makefile is provided to compile using the command: *make*


To run program execution is called  scheduler and run based on 5 parameters:
- **-f filename** will specify the name of the file describing the processes.
- **-a scheduling-algorithm** where scheduling algorithm is one of {ff,rr,cs} where cs is the third scheduling algorithm that you will choose to implement.
- **-m memory-allocation** where memory-allocation is one of {u,p,v,cm} where u indicates unlimited memory and cm is the process/memory replacement algorithm that you will implement.
- **-s memory-size** where memory-size is an integer indicating the size of memory in KB. This option can be ignored in the case of unlimited memory, i.e., when -m u.
- **-q quantum** where quantum is an integer (in seconds). The parameter will be used only for round-robin scheduling algorithm with the default value set to 10 seconds.


Given filename as *processes.txt* with the following information.<br>
Each column represent as *time-arrived, process-id, memory-size-req, job-time* respectively.

0 4 96 30<br>
3 2 32 40<br>
5 1 100 20<br>
20 3 4 30<br>

Example of the execution line
*Example:* ./scheduler -f processes.txt -a ff -s 200 -m p.

## Program Result

Running with the *processes.txt* would give the following result<br>
*./scheduler -f processes.txt -a ff -m u*
<br><br>

0, RUNNING, id=4, remaining-time=30<br>
30, FINISHED, id=4, proc-remaining=3<br>
30, RUNNING, id=2, remaining-time=40<br>
70, FINISHED, id=2, proc-remaining=2<br>
70, RUNNING, id=1, remaining-time=20<br>
90, FINISHED, id=1, proc-remaining=1<br>
90, RUNNING, id=3, remaining-time=30<br>
120, FINISHED, id=3, proc-remaining=0<br>
Throughput 2, 1, 3<br>
Turnaround time 71<br>
Time overhead 4.25 2.56<br>
Makespan 120<br>

