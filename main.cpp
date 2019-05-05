#include <bits/stdc++.h>

using namespace std;

#define MAX_JOBS 2000
#define HIRE_COST 240
#define IMPOSSIBLE_DIST -1
#define MAX_SHIFTS 40
#define MAX_JOBS_PER_SHIFT 50
#define NUM_CANDIDATES 10

struct Job {
    int idx;
    int x, y, d, p, l, r;
    bool assigned = false;
    int nCand;
    Job* cand[NUM_CANDIDATES];
        
    bool isBase() {
        return d == 0;
    }

    int profit() {
        return d * p * (p + 5);
    }
};

struct Work {
    int jobIdx;
    int startT;
};

int n;
Job base;
Job jobs[MAX_JOBS];
int shifts[MAX_SHIFTS][MAX_JOBS_PER_SHIFT];
int nWorkers = 0;
Work workers[MAX_SHIFTS * 7 * 7][MAX_JOBS_PER_SHIFT];

#define SHIFT_SIZE(x) (shifts[x][0])
#define WORKER_SIZE(x) (workers[x][0].jobIdx)

void candidates();

void greedyShifts(int p);

void shiftsToWorkersUsingFreeSpace(int p);

void useFreeSpace(int workerSt, int workerEnd, int timeLimit, Job &to);

void shiftsToWorkers(int p);

void removeUnprofitableCycles(int p);

int evaluate();

void outputTour();

void outputAnalytics();

int dist(Job &a, Job &b, int curT, int &outT);

int l1Dist(Job &a, Job &b);

int main(int argc,  char** argv) {

    ifstream cin(argv[2]);
    ios_base::sync_with_stdio(false);
    std::cin.tie(0);
    
    cin >> n;
    n--;

    cin >> base.x >> base.y >> base.d >> base.p >> base.l >> base.r;

    for (int i = 0; i < n; i++) {
        Job* j = &jobs[i];
        j->idx = i;
        cin >> j->x >> j->y >> j->d >> j->p >> j->l >> j->r;
    }
    candidates();

    for (int p = 7; p >= 1; p--) {
        for (int i = 0; i < MAX_SHIFTS; i++) {
            shifts[i][0] = 0;
        }
        greedyShifts(p);
        removeUnprofitableCycles(p);
        // shiftsToWorkers(p);
        shiftsToWorkersUsingFreeSpace(p);
    }
    if (argc > 1) {
        if (argv[1][0] == '1') {
            cout << evaluate() << endl;    
        } else if (argv[1][0] == '0') {
            outputTour();
        }
    } else {
        outputTour();
    }
}

void candidates() {
    auto cmpFunc = [](pair<int, Job*> a, pair<int, Job*> b) { return a.first < b.first; };
    priority_queue<pair<int, Job*>, vector<pair<int, Job*> >, decltype(cmpFunc)> q(cmpFunc);

    for (int i = 0; i < n; i++) {
        Job *job = &jobs[i];
        job->nCand = 0;
        for (int h = 0; h < n; h++) {
            if (h == i || jobs[h].p != job->p) {
                continue;
            }
            Job *to = &jobs[h];
            int d = l1Dist(*job, *to);
            d += min(max(to->l - (job->l + job->d + d), (job->l + job->d + to-> d + d) <= to->r ? 0 : __INT_MAX__ - d),
                    max(job->l - (to->l + to->d + d), (to->l + job->d + to->d + d) <= job->r ? 0 : __INT_MAX__ - d));
            
            if (q.size() == NUM_CANDIDATES - 1) {
                if (q.top().first > d) {
                    q.push({d, to});    
                    q.pop();
                }
            } else {
                q.push({d, to});
            }
        }

        job->nCand = q.size() + 1;
        job->cand[q.size()] = job;
        int j = q.size() - 1;
        while (!q.empty()) {
            job->cand[j--] = q.top().second;
            q.pop();
        }
    }
}

void greedyShifts(int p) {
    Job *prev = &base;
    int shiftIdx = 0;
    int shiftPos = 1;
    int curT = 0, outT = 0;

    while (true) {
        int minT = __INT_MAX__;
        Job *minJob = NULL;
        for (int i = 0; i < n; i++) {
            Job *job = &jobs[i];
            if (job->assigned || job->p != p) {
                continue;
            }
            if (dist(*prev, jobs[i], curT, outT) != IMPOSSIBLE_DIST && outT < minT) {
                minT = outT;
                minJob = &jobs[i];
            }
        }
        if (minT == __INT_MAX__ && prev == &base) {
            break;
        }
        if (minT == __INT_MAX__) {
            shiftIdx++;
            shiftPos = 1;
            prev = &base;
            curT = 0;
            continue;
        }
        shifts[shiftIdx][shiftPos] = minJob->idx;
        shifts[shiftIdx][0] = shiftPos;
        minJob->assigned = true;
        shiftPos++;
        curT = minT;
        prev = minJob;
    }
}

void shiftsToWorkersUsingFreeSpace(int p) {
    for (int i = 0; i < MAX_SHIFTS; i++) {
        if (SHIFT_SIZE(i) == 0) {
            continue;
        }
        int curT, outT;
        int workerSt = nWorkers;
        int workerEnd = nWorkers + p - 1;
        dist(base, jobs[shifts[i][1]], 0, curT);
        for (int j = workerSt; j <= workerEnd; j++) {
            workers[j][0].jobIdx = 1;
            workers[j][1].jobIdx = shifts[i][1];
            workers[j][1].startT = curT - jobs[shifts[i][1]].d;
        }

        for (int h = 2; h <= SHIFT_SIZE(i); h++) {
            dist(jobs[shifts[i][h - 1]], jobs[shifts[i][h]], curT, outT);
            curT = outT;
            useFreeSpace(workerSt, workerEnd, curT - jobs[shifts[i][h]].d, jobs[shifts[i][h]]);
            for (int j = workerSt; j <= workerEnd; j++) {
                workers[j][0].jobIdx++;
                workers[j][WORKER_SIZE(j)].jobIdx = shifts[i][h];
                workers[j][WORKER_SIZE(j)].startT = curT - jobs[shifts[i][h]].d;
            }
        }
        useFreeSpace(workerSt, workerEnd, __INT_MAX__ - 1, base);
        nWorkers += p;
    }
}

void useFreeSpace(int workerSt, int workerEnd, int timeLimit, Job &to) {
    int nW = workerEnd - workerSt + 1;
    int workerArriveT[nW];
    int minArriveT[nW];
    int tmp[nW];
    while (true) {
        int maxP = 0;
        int minT = __INT_MAX__;
        Job *minJob = NULL;
        for (int i = 0; i < n; i++) {
            if (jobs[i].assigned || jobs[i].p > nW) {
                continue;
            }
            int outT;
            int count = 0;
            for (int h = workerSt; h <= workerEnd; h++) {
                Work *work = &workers[h][WORKER_SIZE(h)];
                int startT = work->startT + jobs[work->jobIdx].d;
                int d = dist(jobs[work->jobIdx], jobs[i], startT, outT);
                if (d == IMPOSSIBLE_DIST) {
                    workerArriveT[h - workerSt] = __INT_MAX__;
                    continue;
                }
                workerArriveT[h - workerSt] = outT + l1Dist(jobs[i], to);
                if (workerArriveT[h - workerSt] <= timeLimit) {
                    count++;
                }
            }
            if (count < jobs[i].p) {
                continue;
            }
            memcpy(tmp, workerArriveT, sizeof(int) * nW);
            sort(tmp, tmp + nW);
            int earliest = tmp[jobs[i].p - 1];
            if (earliest < minT) {
                minT = earliest;
                minJob = &jobs[i];
                memcpy(minArriveT, workerArriveT, sizeof(int) * nW);
            }
        }
        if (minT <= timeLimit) {
            int toTake = minJob->p;
            minJob->assigned = true;
            for (int h = workerSt; h <= workerEnd; h++) {
                if (minArriveT[h - workerSt] > minT || toTake == 0) {
                    continue;
                }
                workers[h][0].jobIdx++;
                workers[h][WORKER_SIZE(h)].jobIdx = minJob->idx;
                workers[h][WORKER_SIZE(h)].startT = minT - l1Dist(*minJob, to) - minJob->d;
                toTake--;
            }
        } else {
            break;
        }
    }
}

void shiftsToWorkers(int p) {
    for (int i = 0; i < MAX_SHIFTS; i++) {
        if (SHIFT_SIZE(i) == 0) {
            continue;
        }
        for (int j = 0; j < p; j++) {
            int curT, outT;
            dist(base, jobs[shifts[i][1]], 0, curT);
            workers[nWorkers][1].jobIdx = shifts[i][1];
            workers[nWorkers][1].startT = curT - jobs[shifts[i][1]].d;
            int workerPos = 1;

            for (int h = 2; h <= SHIFT_SIZE(i); h++) {
                workerPos++;
                workers[nWorkers][workerPos].jobIdx = shifts[i][h];
                dist(jobs[shifts[i][h - 1]], jobs[shifts[i][h]], curT, outT);
                curT = outT;
                workers[nWorkers][workerPos].startT = curT - jobs[shifts[i][h]].d;
            }
            workers[nWorkers][0].jobIdx = workerPos;
            nWorkers++;
        }
    }
}

void removeUnprofitableCycles(int p) {
      for (int i = 0; i < MAX_SHIFTS; i++) {
        if (SHIFT_SIZE(i) == 0) {
            continue;
        }
        
        int curT = 0, outT;
        int profit = 0;
        Job *prev = &base;
        for (int h = 1; h <= SHIFT_SIZE(i); h++) {
            Job *cur = &jobs[shifts[i][h]];
            profit -= dist(*prev, *cur, curT, outT);
            curT = outT;
            profit += cur->profit();
            prev = cur;
        }
        profit -= dist(*prev, base, curT, outT);
        if (profit < 0) {
            shifts[i][0] = 0;
        }
    }
}

int evaluate() {
    bool visited[n] {false};
    int cost = 0;
    for (int i = 0; i < nWorkers; i++) {
        cost -= HIRE_COST;
        int startT = workers[i][1].startT - l1Dist(base, jobs[workers[i][1].jobIdx]);
        int sz = WORKER_SIZE(i);
        int endT = workers[i][sz].startT + l1Dist(base, jobs[workers[i][sz].jobIdx]) + jobs[workers[i][sz].jobIdx].d;
        cost -= endT - startT;
        for (int h = 1; h <= sz; h++) {
            Job *job = &jobs[workers[i][h].jobIdx];
            if (visited[job->idx]) {
                continue;
            }
            visited[job->idx] = true;
            cost += job->profit();
        }
    }
    return cost;
}

void outputTour() {
    for (int i = 0; i < nWorkers; i++) {
        int startT = workers[i][1].startT - l1Dist(base, jobs[workers[i][1].jobIdx]);
        int sz = WORKER_SIZE(i);
        int endT = workers[i][sz].startT + l1Dist(base, jobs[workers[i][sz].jobIdx]) + jobs[workers[i][sz].jobIdx].d;
        cout << "start " << startT << " " << 1 << endl;

        for (int h = 1; h <= sz; h++) {
            Job *job = &jobs[workers[i][h].jobIdx];
            cout << "arrive " << workers[i][h].startT << " " << job->idx + 2 << endl;
            cout << "work " << workers[i][h].startT << " " << workers[i][h].startT + job->d;
            cout << " " << job->idx + 2 << endl;
        }
        cout << "arrive " << endT << " " << 1 << endl; 
        cout << "end" << endl;
    }
}

int dist(Job &a, Job &b, int curT, int &outT) {
    int l1 = l1Dist(a, b);
    if (a.isBase()) {
        if (b.isBase()) {
            outT = 0;
            return 0;
        } else {
            outT = b.l + b.d;
            return (l1 + b.d + HIRE_COST) * b.p;
        }
    } else {
        if (b.isBase()) {
            outT = 0;
            return l1 * a.p;
        } else {
            outT = max(curT + l1, b.l) + b.d;
            if (outT > b.r) {
                return IMPOSSIBLE_DIST;
            } 
            return (outT - curT) * b.p;
        }
    }
}

int l1Dist(Job &a, Job &b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}