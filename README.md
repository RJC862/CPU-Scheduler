# Project 2: Simulatorlab Scheduling Policies

## Release Date: 09/15/2026, 6 PM Due Date: 10/03/2026, 6 PM

**Please read this document carefully before you start coding.** Most bugs in this project come from small misunderstandings about event ordering, preemption, or what a "completion" event means. A careful read up front will save you a lot of debugging time later.

## Getting Started and Repository Rules

This project will be done in **groups of up to two students**. You may complete the project individually or with one partner.

**[Accept Project 2 on Classroom50] (https://classroom50.org/cmpsc473-fall2026/cmpsc-473-fall-2026/assignments/project-2/accept)**

This will create a private repository for your group. If you are working with a partner, only **one group member should create the repository**, after which the other group member should be added as a collaborator.

Clone your repository onto a Linux machine:

```bash
git clone <your-repository-url>
cd <repository-directory>
```

You will use CSE Linux machines for this project, including the lab machines in W135 Westgate. Documentation for lab access is available online at https://www.eecs.psu.edu/cse-student-lab-access/index.aspx and on Canvas.

You may work with your group member, if applicable, but you may not seek help from anyone outside your group. You must acknowledge and explain any online resources, including AI tools, that you used while completing this project.

Some important notes:

* Do not commit compiled binaries, core dumps, editor backup files, or other generated files.
* Commit your work incrementally. A single commit containing the entire completed assignment does not constitute good use of version control.
* If you are working with a partner, both group members are expected to contribute to the project.
* Your repository must build from a clean checkout using the commands specified in this handout.

---

## Introduction

### What You Will Build

In this assignment, you will be simulating a set of scheduling policies within a discrete event simulation framework. The only files you will be modifying are:
1. linked_list.c
2. linked_list.h (optional)
3. schedulerFCFS.c
4. schedulerLCFS.c
5. schedulerSJF.c
6. schedulerMLFQ.c

**You should NOT make any changes in any file besides these six files.** Changing linked_list.h is optional if you want to adjust the struct definitions or add additional functions, but you should NOT change any of the existing function definitions. We have marked locations in the code where you should make changes.

In this assignment, you will implement the **linked list** and **all four scheduling policies: FCFS, LCFS, SJF, and MLFQ**. **There is one submission for the whole assignment.** For each scheduling policy, you will implement the following functions:
1. `Create` – creates the scheduler info struct. We already defined this struct at the top of each scheduler file; you just need to add any fields you'd like and initialize them in this function.
2. `Destroy` – destroys the scheduler info struct.
3. `ScheduleJob` – handles a new job arrival to the queue.
4. `CompleteJob` – handles completing a job and removing it from the queue.

The policies for this project are:
1. **FCFS**: first-come-first-served, **non-preemptive**.
2. **LCFS**: last-come-first-served, **non-preemptive**.
3. **SJF**: shortest-job-first, **non-preemptive**. If multiple waiting jobs have the same size, use the smaller job id first.
4. **MLFQ**: simplified multi-level feedback queue, **preemptive**, with the rules described below.

All the code relies upon the linked list, so you should start by coding that. The linked list supports a sorted mode based on a user-defined comparison function. It is expected that insertions will be O(n), and that is acceptable for this assignment.

You are welcome to define other functions in those files to help structure the code.

### How the Simulator Calls Your Code

In the discrete event simulator, events (i.e., job arrivals and completions) occur over simulated time. When a job arrives, the simulator calls your `ScheduleJob` function. When a scheduled completion event occurs, the simulator calls your `CompleteJob` function. In both cases, the simulator passes in the current simulated time. This introduces you to the style of programming known as event-driven programming. Rather than thinking about the program as a sequence of code that executes linearly, you work with code snippets that happen at different points/events in time.

Your scheduler should keep track of jobs that have arrived but not yet completed. This usually means tracking the currently running job, if any, and the jobs waiting in that policy's queue.

**Arrivals are scheduled by the trace system (handled automatically). Completions are scheduled by your scheduler (your responsibility).**

Use `schedulerScheduleNextCompletion` to tell the simulator when it should next call your `CompleteJob` function for the currently running job. **There should be at most one scheduled completion event at a time; do not schedule one event for every job in the queue.**

When that event occurs, the simulator removes it and calls your `CompleteJob` function. Your scheduler should update its state, choose the next waiting job if one exists, and schedule that job's next completion.

For non-preemptive policies, a new arrival usually waits if another job is already running. For preemptive policies such as MLFQ, a new arrival may make the currently scheduled completion event incorrect. In that case, use `schedulerCancelNextCompletion`, update your scheduler state, and schedule the new next completion event. **For MLFQ, a scheduled "completion" event may mean that a 1-unit time slice ended; it does not always mean the job has finished all of its work.**

## Support routines

The framework for managing the discrete event simulation has already been provided for you, and you won’t need to call most of the functions. Here are the most relevant functions you should be using:

### Functions You Should Use

1. linked list functions that you’re implementing.
2. job.h functions that provide you job info. **You should use the job functions, not the job struct directly**. The testing infrastructure will change the job field names to detect if you're using the field names directly, so it may result in some compilation errors. We only guarantee that the job function interface will not change.
3. common library functions such as malloc/free.

### Completion Events

1. `schedulerScheduleNextCompletion` schedules the time when the next job will complete, and at this time, your `CompleteJob` function will be called. It is your responsibility to figure out when you should be calling this function for scheduling future job completions. **IMPORTANT: This is only for the next completion, not all future completions.** Even if your queue has 5 jobs, you are not immediately scheduling completions for all 5 jobs. You only track the completion for whatever job should complete next. When that job completes, then you'll consider what other job will complete next.
2. `schedulerCancelNextCompletion` cancels the next completion event, which was previously scheduled by `schedulerScheduleNextCompletion`. **If a job completes, you should not be calling this function.** This function is only meant to be called if you want to cancel the completion event **before** your `CompleteJob` function is called (i.e., this is used for preemption).

**You likely won’t need to use any other functions**, so if you start randomly calling other functions in the starter code, you may run into unexpected behavior.

## Important details

### Ordering and Tie-Breaking

1. Assume jobs arrive one at a time, even if they have the same arrival time. Assume the ordering of arrivals is the order in which the `ScheduleJob` function is called.
2. In all policies that require breaking ties, you should use the job id as a tie-breaker. For example, in SJF, if two jobs have the same size, then you can use the job id to break the tie, where the smaller job id would be the smaller size. Note that there are no ties for arrival time since arrivals are ordered by the order in which the `ScheduleJob` function is called.
3. Within the exact same time step, **job completion events are scheduled to complete before job arrival events**. For example, if there is a completion at time 17 and an arrival at time 17, the completion code is called first. As a result, a non-preemptive policy may start the next already-waiting job before considering the job that arrives at time 17.

### Zero-Length Jobs

**Job sizes may be zero.** A zero-length job should complete at its arrival time or when it is selected to run, depending on the scheduling policy state.

## MLFQ policy details

### Required MLFQ Rules

This project uses a **simplified MLFQ**. Implement exactly these rules:
1. **Every priority level has a time slice of 1.**
2. Smaller priority-level numbers are higher priority. **New jobs enter at the highest-priority level, level 0.**
3. If a job runs for a full time slice and still has remaining work, it is demoted by one level.
4. If a job finishes before or exactly at the end of its time slice, it completes and should not be reinserted into the queue.
5. Round-robin order within a priority level is determined by when a job enters that level. For example, if job Q is demoted to priority level 7 first and then job P is demoted to priority level 7 second, then when priority level 7 jobs are served, MLFQ should run Q before P.
6. **There is no priority boosting, no aging, and no periodic reset of priorities.**

### Preemption and Remaining Time

1. A newly arriving level-0 job preempts a currently running job only if the currently running job is at a lower priority level. When this happens, cancel the previously scheduled completion event, update the running job's remaining time, return that job to the appropriate priority level, and schedule the new highest-priority job.
2. **Remember that the job's remaining time is not automatically updated by the simulator.** If your policy depends on remaining time, your scheduler code must update it.

## Evaluation and testing your code

### Grading

You will receive zero points if:
1. You don’t show your partial work by periodically adding, committing, and pushing your code to GitHub
2. Your code does not compile/build

Your grade is split evenly between functional correctness and memory correctness:
1. **50% functional correctness:** your output must match the expected output for each test case.
2. **50% Valgrind/memory correctness:** each test case must pass under Valgrind with no reported memory errors or leaks counted by the grading script.

**If your code does not compile, the tests cannot run.** If your simulator crashes on an individual test case, that test case will be marked failed and the grading script will continue to the remaining tests when possible.

### Trace Files

The public trace files in the traces directory are sample tests intended to help you understand and debug the assignment. **They are not exhaustive.** The final grading will include additional instructor tests, including edge cases not present in the public sample traces. If you'd like, you can generate additional test cases to test your code, which involves generating trace files and expected output.

Trace files are located in the traces directory and are named starting with the policy_ and ending with .csv. For example, FCFS_1.csv is a trace for the FCFS policy.

The trace files contain three columns separated by commas. You can refer to the existing trace files for examples. The first column is the job id, and it should be unique and increasing. The second column is the arrival time, and it must be in sorted order. This means that earlier jobs must be in earlier lines in the trace. The third column is the job size. All the data should be suitable for the uint64_t type, so you cannot use floating point numbers or negative numbers.

The expected output is also located in the traces directory and are named starting with the trace filename followed by “.expected”. For example, the expected output for FCFS_1.csv is FCFS_1.csv.expected.

The expected output files contain two columns separated by a comma. The first column contains the job id in sorted order. The second column contains the completion time.

### Running Tests

To run a trace file, you would run make to compile the program and run:
`./simulator traceFile outFile scheduler`

For example, the following runs a trace with the FCFS policy:
`./simulator traces/FCFS_1.csv traces/FCFS_1.csv.out FCFS`

You can then compare the results by performing a diff with the expected output:
`diff traces/FCFS_1.csv.out traces/FCFS_1.csv.expected`

In addition to testing with trace files, we also have provided a linked list test, which is compiled as the linked_list_test program.

To automatically run all of the public sample tests including all traces in the traces directory (assuming they’re appropriately named), then you would run the following command in the project directory:
`make test`

make test will run grade.py, which copies your code to a sandbox and compiles/runs all of your code. You can find the output of your simulator in the sandbox within the sandbox/traces directory.

The grading script is intended to run in a **Linux environment with gcc, make, Python 3, and Valgrind installed.** Valgrind results can be affected by unusual local environment settings such as preloaded graphical libraries; use the course environment if your local machine reports unrelated system-library leaks.

If you have an error with removing the sandbox, this is most likely due to having a file open within the sandbox directory (e.g., looking at a log file). Close any programs that have open files in the sandbox directory, and then the make test should work. You can also try make clean if this doesn't work.




## Hints

1. This assignment exposes you to event-driven programming, which requires a different mental model. When the events (arrival, completion) occur, your code should just be reacting to that event. You do not need to think about how everything fits together. You just need to address the current event, which may involve scheduling a future completion event.
2. You should only be performing operations that apply to the current simulated time. If something happens in the future, it will be addressed at that future time.
3. In event-driven programming, there is not a linear flow of function calls. Everything is based on callbacks, where function pointers are called at the appropriate time based on when the event was scheduled (by an earlier event). For example, the program starts by scheduling the first job arrival, and at the first job arrival, the second job arrival is scheduled (by the provided code). The sequence of job arrivals and completions is thus dynamically managed in a simulator queue of events (which is already implemented).
4. The job has a remaining time field that you can set through the `jobSetRemainingTime` function. **The remaining time is NOT automatically updated.** It is up to your code to track this info if needed. You are not required to use this field and can ignore it, but for some policies, it may be helpful to track.
5. The linked list contains a generic void* data pointer. You can simply cast this to whatever type you want.
6. If you want a sorted list, you will likely need to create comparison functions. You can define one in the associated scheduler file and create a linked list with this comparison function.
7. **Do not directly use struct fields in the starter code.** Functions have already been provided to access anything you may need. The functions that we’ve provided have been intentionally designed to shield you from accidentally messing with the struct elements in unexpected ways.
8. Similarly, we’ve also designed the code so that only the functions listed in the support routines section are needed. This is meant to help you avoid using functions in unintended ways, as connecting all the components requires complex interactions with callback function pointers.
