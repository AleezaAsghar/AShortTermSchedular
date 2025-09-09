#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>

using namespace std;

const int MAX_PROCESSES = 100;
const int MAX_GANTT_ENTRIES = 1000;

const int STARVATION_THRESHOLD = 5;

struct Process {
    int pid;
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int priority;
    int startTime;
    int completionTime;
    int waitingTime;
    int turnaroundTime;
    int responseTime;
    int currentQueue;
    int queueWaitTime;
    bool completed;
};

string intToString(int value) {
    stringstream ss;
    ss << value;
    return ss.str();
}

string getQueueName(int queue) {
    switch (queue) {
    case 0: return "Q0 (Round Robin)";
    case 1: return "Q1 (SJF)";
    case 2: return "Q2 (Priority)";
    case 3: return "Q3 (SRTF)";
    default: return "Unassigned";
    }
}

template <typename Q0, typename Q1, typename Q2, typename Q3>
void printQueueStates(const Q0& q0, const Q1& q1, const Q2& q2, const Q3& q3) {
    cout << "Queue States:\n";
    cout << "  " << getQueueName(0) << ": ";
    if (q0.empty()) cout << "Empty";
    else {
        queue<Process> tempQ0 = q0;
        while (!tempQ0.empty()) {
            cout << "P" << tempQ0.front().pid << " ";
            tempQ0.pop();
        }
    }
    cout << endl;
    cout << "  " << getQueueName(1) << ": ";
    if (q1.empty()) cout << "Empty";
    else {
        auto tempQ1 = q1;
        while (!tempQ1.empty()) {
            cout << "P" << tempQ1.top().pid << " ";
            tempQ1.pop();
        }
    }
    cout << endl;
    cout << "  " << getQueueName(2) << ": ";
    if (q2.empty()) cout << "Empty";
    else {
        auto tempQ2 = q2;
        while (!tempQ2.empty()) {
            cout << "P" << tempQ2.top().pid << " ";
            tempQ2.pop();
        }
    }
    cout << endl;
    cout << "  " << getQueueName(3) << ": ";
    if (q3.empty()) cout << "Empty";
    else {
        auto tempQ3 = q3;
        while (!tempQ3.empty()) {
            cout << "P" << tempQ3.top().pid << " ";
            tempQ3.pop();
        }
    }
    cout << endl;
}

void getProcessInput(Process processes[], int& numProcesses, int& quantumQ0) {
    cout << "Enter the number of processes (max " << MAX_PROCESSES << "): ";
    cin >> numProcesses;
    while (numProcesses <= 0 || numProcesses > MAX_PROCESSES) {
        cout << "Please enter a number between 1 and " << MAX_PROCESSES << ": ";
        cin >> numProcesses;
    }

    cout << "Enter the quantum time for " << getQueueName(0) << ": ";
    cin >> quantumQ0;
    while (quantumQ0 <= 0) {
        cout << "Please enter a positive quantum time: ";
        cin >> quantumQ0;
    }

    for (int i = 0; i < numProcesses; i++) {
        processes[i].pid = i + 1;
        cout << "\nProcess " << processes[i].pid << ":\n";
        cout << "Enter Arrival Time: ";
        cin >> processes[i].arrivalTime;
        while (processes[i].arrivalTime < 0) {
            cout << "Arrival time cannot be negative. Enter again: ";
            cin >> processes[i].arrivalTime;
        }
        cout << "Enter Burst Time: ";
        cin >> processes[i].burstTime;
        while (processes[i].burstTime <= 0) {
            cout << "Burst time must be positive. Enter again: ";
            cin >> processes[i].burstTime;
        }
        cout << "Enter Priority (1-10, lower is higher priority): ";
        cin >> processes[i].priority;
        while (processes[i].priority < 1 || processes[i].priority > 10) {
            cout << "Priority must be between 1 and 10. Enter again: ";
            cin >> processes[i].priority;
        }
        processes[i].remainingTime = processes[i].burstTime;
        processes[i].currentQueue = -1;
        processes[i].queueWaitTime = 0;
        processes[i].completed = false;
        processes[i].startTime = -1;
        processes[i].completionTime = 0;
        processes[i].waitingTime = 0;
        processes[i].turnaroundTime = 0;
        processes[i].responseTime = 0;
    }
}

struct SJFComparator {
    bool operator()(const Process& a, const Process& b) {
        return a.burstTime > b.burstTime;
    }
};

struct PriorityComparator {
    bool operator()(const Process& a, const Process& b) {
        return a.priority > b.priority;
    }
};

struct SRTFComparator {
    bool operator()(const Process& a, const Process& b) {
        return a.remainingTime > b.remainingTime;
    }
};

void mlfqScheduler(Process processes[], int numProcesses, int quantumQ0) {
    int currentTime = 0;
    int completedProcesses = 0;
    int ganttIndex = 0;
    int cpuBusyTime = 0;
    string ganttChart[MAX_GANTT_ENTRIES];

    queue<Process> q0;
    priority_queue<Process, vector<Process>, SJFComparator> q1;
    priority_queue<Process, vector<Process>, PriorityComparator> q2;
    priority_queue<Process, vector<Process>, SRTFComparator> q3;

    cout << "\nStarting MLFQ Scheduler...\n";

    while (completedProcesses < numProcesses) {
        cout << "\nTime " << currentTime << ":\n";

        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].arrivalTime <= currentTime && !processes[i].completed && processes[i].currentQueue == -1) {
                processes[i].currentQueue = 0;
                processes[i].queueWaitTime = 0;
                q0.push(processes[i]);
                cout << "  P" << processes[i].pid << " arrived and assigned to " << getQueueName(0) << endl;
                processes[i] = processes[i];
            }
        }

        for (int i = 0; i < numProcesses; i++) {
            if (!processes[i].completed && processes[i].currentQueue != -1) {
                processes[i].queueWaitTime++;
                processes[i] = processes[i];
            }
        }

        printQueueStates(q0, q1, q2, q3);

        if (!q0.empty()) {
            Process p = q0.front();
            q0.pop();
            cout << "  Executing P" << p.pid << " from " << getQueueName(0) << endl;
            if (p.startTime == -1) {
                p.startTime = currentTime;
                p.responseTime = currentTime - p.arrivalTime;
            }
            int execTime = min(quantumQ0, p.remainingTime);
            p.remainingTime -= execTime;
            cpuBusyTime += execTime;
            if (ganttIndex < MAX_GANTT_ENTRIES) {
                ganttChart[ganttIndex++] = "P" + intToString(p.pid) + "(" + intToString(currentTime) + "-" + intToString(currentTime + execTime) + ")";
            }
            currentTime += execTime;

            if (p.remainingTime <= 0) {
                p.completionTime = currentTime;
                p.turnaroundTime = p.completionTime - p.arrivalTime;
                p.waitingTime = p.turnaroundTime - p.burstTime;
                p.completed = true;
                completedProcesses++;
                cout << "  P" << p.pid << " completed\n";
            }
            else {
                p.currentQueue = 1;
                p.queueWaitTime = 0;
                q1.push(p);
                cout << "  P" << p.pid << " demoted to " << getQueueName(1) << " (remaining time: " << p.remainingTime << ")\n";
            }
            processes[p.pid - 1] = p;
        }
        else if (!q1.empty()) {
            Process p = q1.top();
            q1.pop();
            cout << "  Executing P" << p.pid << " from " << getQueueName(1) << endl;
            if (p.startTime == -1) {
                p.startTime = currentTime;
                p.responseTime = currentTime - p.arrivalTime;
            }
            int execTime = p.remainingTime;
            p.remainingTime = 0;
            cpuBusyTime += execTime;
            if (ganttIndex < MAX_GANTT_ENTRIES) {
                ganttChart[ganttIndex++] = "P" + intToString(p.pid) + "(" + intToString(currentTime) + "-" + intToString(currentTime + execTime) + ")";
            }
            currentTime += execTime;

            p.completionTime = currentTime;
            p.turnaroundTime = p.completionTime - p.arrivalTime;
            p.waitingTime = p.turnaroundTime - p.burstTime;
            p.completed = true;
            completedProcesses++;
            cout << "  P" << p.pid << " completed\n";
            processes[p.pid - 1] = p;
        }
        else if (!q2.empty()) {
            Process p = q2.top();
            q2.pop();
            cout << "  Executing P" << p.pid << " from " << getQueueName(2) << endl;
            if (p.startTime == -1) {
                p.startTime = currentTime;
                p.responseTime = currentTime - p.arrivalTime;
            }
            int execTime = p.remainingTime;
            p.remainingTime = 0;
            cpuBusyTime += execTime;
            if (ganttIndex < MAX_GANTT_ENTRIES) {
                ganttChart[ganttIndex++] = "P" + intToString(p.pid) + "(" + intToString(currentTime) + "-" + intToString(currentTime + execTime) + ")";
            }
            currentTime += execTime;

            p.completionTime = currentTime;
            p.turnaroundTime = p.completionTime - p.arrivalTime;
            p.waitingTime = p.turnaroundTime - p.burstTime;
            p.completed = true;
            completedProcesses++;
            cout << "  P" << p.pid << " completed\n";
            processes[p.pid - 1] = p;
        }
        else if (!q3.empty()) {
            Process p = q3.top();
            q3.pop();
            cout << "  Executing P" << p.pid << " from " << getQueueName(3) << endl;
            if (p.startTime == -1) {
                p.startTime = currentTime;
                p.responseTime = currentTime - p.arrivalTime;
            }
            int execTime = 1;
            p.remainingTime -= execTime;
            cpuBusyTime += execTime;
            if (ganttIndex < MAX_GANTT_ENTRIES) {
                ganttChart[ganttIndex++] = "P" + intToString(p.pid) + "(" + intToString(currentTime) + "-" + intToString(currentTime + execTime) + ")";
            }
            currentTime += execTime;

            bool preempt = false;
            for (int i = 0; i < numProcesses; i++) {
                if (processes[i].arrivalTime <= currentTime && !processes[i].completed && processes[i].currentQueue == -1) {
                    processes[i].currentQueue = 0;
                    processes[i].queueWaitTime = 0;
                    q0.push(processes[i]);
                    cout << "  P" << processes[i].pid << " arrived and assigned to " << getQueueName(0) << endl;
                    processes[i] = processes[i];
                    preempt = true;
                }
            }
            if (!q0.empty() || !q1.empty() || !q2.empty()) {
                preempt = true;
            }

            if (p.remainingTime <= 0) {
                p.completionTime = currentTime;
                p.turnaroundTime = p.completionTime - p.arrivalTime;
                p.waitingTime = p.turnaroundTime - p.burstTime;
                p.completed = true;
                completedProcesses++;
                cout << "  P" << p.pid << " completed\n";
            }
            else if (preempt) {
                p.queueWaitTime = 0;
                q3.push(p);
                cout << "  P" << p.pid << " preempted and reinserted into " << getQueueName(3) << " (remaining time: " << p.remainingTime << ")\n";
            }
            else {
                p.queueWaitTime = 0;
                q3.push(p);
                cout << "  P" << p.pid << " reinserted into " << getQueueName(3) << " (remaining time: " << p.remainingTime << ")\n";
            }
            processes[p.pid - 1] = p;
        }
        else {
            currentTime++;
            if (ganttIndex < MAX_GANTT_ENTRIES) {
                ganttChart[ganttIndex++] = "Idle(" + intToString(currentTime - 1) + "-" + intToString(currentTime) + ")";
            }
            cout << "  CPU idle\n";
        }

        auto promoteQueue = [&](auto& srcQueue, int srcQueueNum, auto& destQueue, int destQueueNum) {
            vector<Process> temp;
            while (!srcQueue.empty()) {
                Process p = srcQueue.top();
                srcQueue.pop();
                if (p.queueWaitTime >= STARVATION_THRESHOLD) {
                    p.currentQueue = destQueueNum;
                    p.queueWaitTime = 0;
                    cout << "  P" << p.pid << " promoted from " << getQueueName(srcQueueNum) << " to " << getQueueName(destQueueNum) << endl;
                    destQueue.push(p);
                }
                else {
                    temp.push_back(p);
                }
                processes[p.pid - 1] = p;
            }
            for (const auto& p : temp) {
                srcQueue.push(p);
            }
        };

        promoteQueue(q3, 3, q2, 2);
        promoteQueue(q2, 2, q1, 1);
        promoteQueue(q1, 1, q0, 0);
    }

    cout << "\nGantt Chart:\n| ";
    for (int i = 0; i < ganttIndex; i++) {
        cout << ganttChart[i] << " | ";
    }
    cout << endl;

    double avgWaitingTime = 0, avgTurnaroundTime = 0, avgResponseTime = 0;
    cout << "\nProcess Details:\n";
    cout << setw(5) << "PID" << setw(10) << "Arrival" << setw(10) << "Burst" << setw(10) << "Priority"
        << setw(12) << "Completion" << setw(10) << "Waiting" << setw(12) << "Turnaround" << setw(10) << "Response" << endl;
    for (int i = 0; i < numProcesses; i++) {
        cout << setw(5) << processes[i].pid << setw(10) << processes[i].arrivalTime << setw(10) << processes[i].burstTime
            << setw(10) << processes[i].priority << setw(12) << processes[i].completionTime << setw(10) << processes[i].waitingTime
            << setw(12) << processes[i].turnaroundTime << setw(10) << processes[i].responseTime << endl;
        avgWaitingTime += processes[i].waitingTime;
        avgTurnaroundTime += processes[i].turnaroundTime;
        avgResponseTime += processes[i].responseTime;
    }
    avgWaitingTime /= numProcesses;
    avgTurnaroundTime /= numProcesses;
    avgResponseTime /= numProcesses;
    double cpuUtilization = (currentTime > 0) ? (100.0 * cpuBusyTime / currentTime) : 0.0;

    cout << fixed << setprecision(2);
    cout << "\nAverage Waiting Time: " << avgWaitingTime << " units\n";
    cout << "Average Turnaround Time: " << avgTurnaroundTime << " units\n";
    cout << "Average Response Time: " << avgResponseTime << " units\n";
    cout << "CPU Utilization: " << cpuUtilization << "%\n";
}

int main() {
    int numProcesses, quantumQ0;
    Process processes[MAX_PROCESSES];
    getProcessInput(processes, numProcesses, quantumQ0);
    mlfqScheduler(processes, numProcesses, quantumQ0);
    return 0;
}