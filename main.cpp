#include <bits/stdc++.h>

using namespace std;

#define MAX_JOBS 3000
#define HIRE_COST 240
#define IMPOSSIBLE_DIST ((int)-1e9)
#define MAX_SHIFTS 80
#define MAX_JOBS_PER_SHIFT 50
#define MAX_NUM_CANDIDATES 20

int NUM_CANDIDATES = 6;
int MAX_K = 5;
int CONT_MAX_K = 4;

struct Job {
    int idx;
    int altIdx;
    int x, y, d, p, l, r;
    bool assigned = false;
    int nCand;
    Job *cand[MAX_NUM_CANDIDATES];
    Job *adj[2];
    Job *repl[2];
    Job *crn[2];
    bool isEnding = false;
    int stWorker;

    Job *adjWithRepl(int idx) {
        if (repl[idx] != NULL) {
            return repl[idx];
        } else {
            return adj[idx];
        }
    }

    bool isBase() { return p == 0; }

    int profit() { return d * p * (p + 5); }
};

struct Work {
    int jobIdx;
    int startT;
};

struct TravelRes {
    int profit;
    int nVisited;
    Job *end;
};

int n;
Job base;
Job jobs[MAX_JOBS];
int shifts[MAX_SHIFTS][MAX_JOBS_PER_SHIFT];
int nWorkers = 0;
Work workers[MAX_SHIFTS * 7 * 7][MAX_JOBS_PER_SHIFT];
unordered_set<Job *> corners;
int nChanges;
Job *changes[MAX_JOBS];
int profit[MAX_JOBS];
int nVisited[MAX_JOBS];
int procFlag[MAX_JOBS];
int procFlag2[MAX_JOBS];
int flagCt = 0;
int flagCt2 = 0;
int profitAll = 0;
int nAll = 0;
int nEndings = 0;

#define SHIFT_SIZE(x) (shifts[x][0])
#define WORKER_SIZE(x) (workers[x][0].jobIdx)

void candidates(int p, bool allowReverse = true, bool estimWait = true);

void candidates(int p, Job *cJobs[], int nJobs, bool allowReverse, bool estimWait);

void kOpt(int p);

void recalc();

TravelRes travel(Job *st, bool tryOpposite = true);

bool kOptStart(int nJs, Job *js[]);

bool kOptRec(Job *stJob, int stReplIdx, Job *job, int replIdx, int k);

int kOptGain();

void greedyShifts(int p);

void shiftsToWorkersUsingFreeSpace(int p);

void useFreeSpace(int workerSt, int workerEnd, int timeLimit, Job &to);

void shiftsToWorkers(int p);

void removeUnprofitableCycles(int p);

void collectEndings(int p);

int evaluate();

void outputTour();

void outputAnalytics();

int dist(Job &a, Job &b, int curT, int &outT);

int l1Dist(Job &a, Job &b);

int main(int argc, char **argv) {
    clock_t z = clock();
    ifstream cin(argv[2]);
    ios_base::sync_with_stdio(false);
    std::cin.tie(0);

    cin >> n;
    n--;
    if (n < 600) {
        NUM_CANDIDATES = 7;
        MAX_K = 4;
        CONT_MAX_K = 4;
    } else if (n < 800) {
        NUM_CANDIDATES = 6;
        MAX_K = 4;
        CONT_MAX_K = 4;
    } else if (n < 1100) {
        NUM_CANDIDATES = 5;
        MAX_K = 4;
        CONT_MAX_K = 4;
    } else if (n < 1400) {
        NUM_CANDIDATES = 8;
        MAX_K = 4;
        CONT_MAX_K = 0;
    } else if (n < 1700) {
        NUM_CANDIDATES = 7;
        MAX_K = 4;
        CONT_MAX_K = 0;
    } else {
        NUM_CANDIDATES = 6;
        MAX_K = 4;
        CONT_MAX_K = 0;
    }

    cin >> base.x >> base.y >> base.d >> base.p >> base.l >> base.r;

    for (int i = 0; i < n; i++) {
        Job *j = &jobs[i];
        j->idx = i;
        cin >> j->x >> j->y >> j->d >> j->p >> j->l >> j->r;
    }

    for (int p = 7; p >= 1; p--) {
        for (int i = 0; i < MAX_SHIFTS; i++) {
            shifts[i][0] = 0;
        }
        collectEndings(p);
        greedyShifts(p);
        candidates(p);
        kOpt(p);
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
    cout << "Total Time: " << (double)(clock() - z) / CLOCKS_PER_SEC << endl;
}

void candidates(int p, bool allowReverse, bool estimWait) {
    Job *cJobs[n];
    int nJobs = 0;
    for (int i = 0; i < n + nEndings; i++) {
        Job *job = &jobs[i];
        if (job->p != p || job->assigned) {
            continue;
        }
        cJobs[nJobs++] = job;
    }
    candidates(p, cJobs, nJobs, allowReverse, estimWait);
}

void candidates(int p, Job *cJobs[], int nJobs, bool allowReverse, bool estimWait) {
    auto cmpFunc = [](pair<int, Job *> a, pair<int, Job *> b) {
        return a.first < b.first;
    };
    priority_queue<pair<int, Job *>, vector<pair<int, Job *>>,
                   decltype(cmpFunc)>
        q(cmpFunc);

    for (int i = 0; i < nJobs; i++) {
        Job *job = cJobs[i];
        job->nCand = 0;
        for (int h = 0; h < nJobs; h++) {
            if (h == i) {
                continue;
            }
            Job *to = cJobs[h];
            int d = l1Dist(*job, *to);
            int rest;
            if (!to->isEnding) {
                rest = max(to->l - d - (estimWait ? job->l + job->d : job->r),
                           (job->l + job->d + to->d + d) <= to->r ? 0
                                                                  : __INT_MAX__ - d);
            } else {
                rest = __INT_MAX__ - d;
            }
            if (allowReverse && !job->isEnding) {
                rest = min(rest, max(job->l - d - (estimWait ? to->l + to->d : to->r),
                                     (to->l + job->d + to->d + d) <= job->r ? 0
                                                                            : __INT_MAX__ - d));
            }
            d += rest;
            if (d == __INT_MAX__) {
                continue;
            }

            if (q.size() == NUM_CANDIDATES) {
                if (q.top().first > d) {
                    q.push({d, to});
                    q.pop();
                }
            } else {
                q.push({d, to});
            }
        }

        job->nCand = q.size();
        int j = q.size() - 1;
        while (!q.empty()) {
            job->cand[j--] = q.top().second;
            q.pop();
        }
    }
}

void kOpt(int p) {
    Job *js[n];
    int nJs = 0;
    for (int i = 0; i < n + nEndings; i++) {
        if (jobs[i].p != p || jobs[i].assigned) {
            continue;
        }
        js[nJs++] = &jobs[i];
    }
    corners.clear();
    for (int i = 0; i < MAX_SHIFTS; i++) {
        if (SHIFT_SIZE(i) == 0) {
            continue;
        }
        Job *crn1 = &jobs[shifts[i][1]];
        Job *crn2 = &jobs[shifts[i][SHIFT_SIZE(i)]];
        Job *prev = crn1;
        for (int h = 1; h <= SHIFT_SIZE(i); h++) {
            Job *cur = &jobs[shifts[i][h]];
            cur->adj[0] = prev;
            cur->adj[1] = h == SHIFT_SIZE(i) ? cur : &jobs[shifts[i][h + 1]];
            prev = cur;
        }
        corners.insert(crn1);
        corners.insert(crn2);
    }
    for (int i = n; i < n + nEndings; i++) {
        corners.insert(&jobs[i]);
    }
    recalc();
    while (kOptStart(nJs, js)) {
        nChanges = 0;
        corners.clear();
        for (int i = 0; i < nJs; i++) {
            for (int h = 0; h < 2; h++) {
                if (js[i]->repl[h] != NULL) {
                    js[i]->adj[h] = js[i]->repl[h];
                    js[i]->repl[h] = NULL;
                }
            }
            if (js[i]->adj[0] == js[i] || js[i]->adj[1] == js[i]) {
                corners.insert(js[i]);
            }
        }
        recalc();
        random_shuffle(js, js + nJs);
    }
    for (int i = 0; i < MAX_SHIFTS; i++) {
        shifts[i][0] = 0;
    }
    int shiftI = 0;
    for (Job *corner : corners) {
        if (corner != min(corner->crn[0], corner->crn[1])) {
            continue;
        }
        if (corner->isEnding && nVisited[corner->idx] == 1) {
            continue;
        }
        Job *cur = travel(corner->crn[0], false).profit >
                           travel(corner->crn[1], false).profit
                       ? corner->crn[0]
                       : corner->crn[1];
        Job *prev = cur;
        do {
            shifts[shiftI][0]++;
            shifts[shiftI][SHIFT_SIZE(shiftI)] = cur->idx;
            if (cur->adj[0] == prev) {
                prev = cur;
                cur = cur->adj[1];
            } else {
                prev = cur;
                cur = cur->adj[0];
            }
        } while (prev != cur);
        shiftI++;
    }
}

void recalc() {
    flagCt++;
    profitAll = 0;
    nAll = 0;
    for (Job *corner : corners) {
        if (procFlag[corner->idx] == flagCt) {
            continue;
        }
        TravelRes res = travel(corner);
        if (res.profit == IMPOSSIBLE_DIST) {
            exit(1);
        }
        profit[corner->idx] = res.profit;
        profit[res.end->idx] = res.profit;
        nVisited[corner->idx] = res.nVisited;
        nVisited[res.end->idx] = res.nVisited;
        procFlag[corner->idx] = flagCt;
        procFlag[res.end->idx] = flagCt;
        if (res.profit > 0 || corner->isEnding || res.end->isEnding) {
            profitAll += res.profit;
        }
        nAll += res.nVisited;
        Job *cur = corner;
        Job *prev = cur;
        do {
            cur->crn[0] = corner;
            cur->crn[1] = res.end;
            if (cur->adj[0] == prev) {
                prev = cur;
                cur = cur->adj[1];
            } else {
                prev = cur;
                cur = cur->adj[0];
            }
        } while (cur != res.end || prev != res.end);
    }
}

TravelRes travel(Job *st, bool tryOpposite) {
    flagCt2++;
    Job *cur = st;
    Job *prev = cur;
    int curT, outT;
    TravelRes res;
    res.nVisited = 1;
    res.profit = -dist(base, *cur, 0, curT);
    res.profit += cur->profit();
    procFlag2[cur->idx] = flagCt2;
    bool impossible = false;
    while (true) {
        if (cur->adjWithRepl(0) == prev) {
            prev = cur;
            cur = cur->adjWithRepl(1);
        } else {
            prev = cur;
            cur = cur->adjWithRepl(0);
        }
        if (prev != cur && procFlag2[cur->idx] == flagCt2) {
            res.profit = IMPOSSIBLE_DIST;
            return res;
        }
        procFlag2[cur->idx] = flagCt2;
        if (impossible) {
            if (cur == prev) {
                res = travel(cur, false);
                res.end = cur;
                return res;
            }
        } else {
            if (cur == prev) {
                res.profit -= dist(*cur, base, curT, outT);
                res.end = cur;
                if (tryOpposite) {
                    res.profit = max(res.profit, travel(cur, false).profit);
                }
                return res;
            } else {
                int d = dist(*prev, *cur, curT, outT);
                if (d == IMPOSSIBLE_DIST) {
                    impossible = true;
                    if (!tryOpposite) {
                        res.profit = IMPOSSIBLE_DIST;
                        return res;
                    }
                    continue;
                }
                curT = outT;
                res.nVisited++;
                res.profit -= d;
                res.profit += cur->profit();
            }
        }
    }
}

bool kOptStart(int nJs, Job *js[]) {
    for (int i = 0; i < nJs; i++) {
        Job *j = js[i];
        for (int h = 0; h < 2; h++) {
            if (j->isEnding && h == 0) {
                continue;
            }
            j->repl[h] = j;
            changes[nChanges++] = j;
            if (kOptRec(j, h, j, h, 2)) {
                return true;
            }
            nChanges--;
            j->repl[h] = NULL;
        }
    }
    return false;
}

bool kOptContinue(int k) {
    Job *j1 = changes[0]->crn[0];
    Job *prev = j1;
    do {
        if (j1->repl[0] == NULL && j1->repl[1] == NULL) {
            for (int i = 0; i < 2; i++) {
                if (j1->isEnding && i == 0) {
                    continue;
                }
                j1->repl[i] = j1;
                changes[nChanges++] = j1;
                if (kOptRec(j1, i, j1, i, MAX_K)) {
                    return true;
                }
                nChanges--;
                j1->repl[i] = NULL;
            }
        }
        if (j1->adj[0] == prev) {
            prev = j1;
            j1 = j1->adj[1];
        } else {
            prev = j1;
            j1 = j1->adj[0];
        }
    } while (prev != j1);
    return false;
}

bool kOptRec(Job *stJob, int stReplIdx, Job *j1, int replIdx, int k) {
    Job *j2 = j1->adj[replIdx];
    for (int i = 0; i < j2->nCand; i++) {
        Job *j3 = j2->cand[i];
        if (j3->assigned) {
            continue;
        }
        // don't add what is already added
        if (j2->adj[0] == j3 || j2->adj[1] == j3 || j2->repl[0] == j3 ||
            j2->repl[1] == j3) {
            continue;
        }
        changes[nChanges++] = j3;
        if (j2->adj[0] == j1) {
            j2->repl[0] = j3;
        } else {
            j2->repl[1] = j3;
        }
        for (int h = 0; h < 2; h++) {
            // it shouldn't be deleted before
            if (j3->repl[h] != NULL) {
                continue;
            }
            bool isEnding = (j3->isEnding && h == 0);
            j3->repl[h] = j2;
            Job *j4 = j3->adj[h];
            int ri;
            if (j4->adj[0] == j3) {
                ri = 0;
            } else {
                ri = 1;
            }
            for (int opt = 0; opt < 2; opt++) {
                if (opt == 0 &&
                    (isEnding || stJob->repl[stReplIdx] != stJob || j4->repl[ri] != NULL)) {
                    continue;
                }
                Job *z1 = stJob->repl[stReplIdx];
                Job *z2 = j4->repl[ri];
                stJob->repl[stReplIdx] = opt == 0 ? j4 : z1;
                j4->repl[ri] = opt == 0 ? stJob : (z2 == NULL ? j4 : z2);
                changes[nChanges++] = j4;
                int gain = kOptGain();
                if (gain != IMPOSSIBLE_DIST && gain > 0) {
                    return true;
                }
                if (CONT_MAX_K > k + 1 && kOptContinue(k + 1)) {
                    return true;
                }
                nChanges--;
                stJob->repl[stReplIdx] = z1;
                j4->repl[ri] = z2;
            }

            if (j4->repl[ri] == NULL && MAX_K > k && !isEnding &&
                kOptRec(stJob, stReplIdx, j3, h, k + 1)) {
                return true;
            }
            j3->repl[h] = NULL;
        }
        if (j2->adj[0] == j1) {
            j2->repl[0] = NULL;
        } else {
            j2->repl[1] = NULL;
        }
        nChanges--;
    }
    return false;
}

int kOptGain() {
    flagCt++;
    int profitCur = 0;
    int nCur = 0;
    for (int i = 0; i < nChanges; i++) {
        Job *job = changes[i];
        for (int h = 0; h < 3; h++) {
            Job *crn;
            if (h < 2) {
                crn = job->crn[h];
            } else {
                crn = job;
            }
            if (crn->adjWithRepl(0) != crn && crn->adjWithRepl(1) != crn) {
                continue;
            }
            if (procFlag[crn->idx] == flagCt) {
                continue;
            }
            TravelRes res = travel(crn);
            if (res.profit == IMPOSSIBLE_DIST) {
                return IMPOSSIBLE_DIST;
            }
            procFlag[crn->idx] = flagCt;
            procFlag[res.end->idx] = flagCt;
            if (res.profit > 0 || crn->isEnding || res.end->isEnding) {
                profitCur += res.profit;
            }
            nCur += res.nVisited;
        }
    }
    for (Job *corner : corners) {
        if (procFlag[corner->idx] == flagCt) {
            continue;
        }
        if (corner->adjWithRepl(0) != corner &&
            corner->adjWithRepl(1) != corner) {
            continue;
        }
        if (profit[corner->idx] > 0 || corner->crn[0]->isEnding || corner->crn[1]->isEnding) {
            profitCur += profit[corner->idx];
        }
        nCur += nVisited[corner->idx];
        procFlag[corner->crn[0]->idx] = flagCt;
        procFlag[corner->crn[1]->idx] = flagCt;
    }
    if (nCur != nAll) {
        return IMPOSSIBLE_DIST;
    } else {
        return profitCur - profitAll;
    }
}

void greedyShifts(int p) {
    bool visited[n + nEndings];
    fill(visited, visited + n + nEndings, false);
    int MAX_TIME = 800;
    int shiftI = 0;
    while (true) {
        Job *cJobs[n];
        int cJobsN = 0;
        for (int i = 0; i < n; i++) {
            if (visited[i] || jobs[i].p != p || jobs[i].assigned) {
                continue;
            }
            jobs[i].altIdx = cJobsN;
            cJobs[cJobsN++] = &jobs[i];
        }
        if (cJobsN == 0) {
            return;
        }
        candidates(p, cJobs, cJobsN, false, false);
        int dp[MAX_TIME + 1][cJobsN][3];
        fill(&(dp[0][0][0]), &(dp[0][0][0]) + (MAX_TIME + 1) * cJobsN * 3, -1);
        int outT;
        int maxNum = 0;
        int minEndT;
        int maxT, maxJ;
        for (int i = 0; i < nEndings; i++) {
            if (visited[n + i]) {
                continue;
            }
            for (int h = 0; h < cJobsN; h++) {
                int d = dist(jobs[n + i], *cJobs[h], jobs[n + i].l, outT);
                if (d == IMPOSSIBLE_DIST) {
                    continue;
                }
                outT -= cJobs[h]->d;
                dp[outT][h][0] = 2;
                dp[outT][h][2] = n + i;
                maxNum = 2;
                dist(*cJobs[h], base, outT + cJobs[h]->d, minEndT);
                maxT = outT;
                maxJ = h;
            }
        }
        if (maxNum == 0) {
            for (int i = 0; i < cJobsN; i++) {
                dist(base, *cJobs[i], 0, outT);
                outT -= cJobs[i]->d;
                dp[outT][i][0] = 1;
                maxNum = 1;
                dist(*cJobs[i], base, outT + cJobs[i]->d, minEndT);
                maxT = outT;
                maxJ = i;
            }
        }
        for (int i = 200; i <= MAX_TIME; i++) {
            for (int h = 0; h < cJobsN; h++) {
                if (dp[i][h][0] == -1) {
                    continue;
                }
                flagCt2++;
                int stTime = i;
                int stJob = h;
                do {
                    procFlag2[cJobs[stJob]->idx] = flagCt2;
                    int z = stTime;
                    stTime = dp[stTime][stJob][1];
                    stJob = dp[z][stJob][2];
                } while (stTime != -1);

                int finTime;
                dist(*cJobs[h], *cJobs[h], i, finTime);
                for (int j = 0; j < cJobs[h]->nCand; j++) {
                    Job *to = cJobs[h]->cand[j];
                    if (procFlag2[to->idx] == flagCt2) {
                        continue;
                    }
                    int d = dist(*cJobs[h], *to, finTime, outT);
                    if (d == IMPOSSIBLE_DIST) {
                        continue;
                    }
                    outT = finTime + l1Dist(*cJobs[h], *to);
                    int toIdx = to->altIdx;
                    if (dp[outT][toIdx][0] < dp[i][h][0] + 1) {
                        dp[outT][toIdx][0] = dp[i][h][0] + 1;
                        dp[outT][toIdx][1] = i;
                        dp[outT][toIdx][2] = h;
                        int endT;
                        dist(*to, *to, outT, endT);
                        endT += l1Dist(*to, base);
                        if (dp[outT][toIdx][0] > maxNum || (dp[outT][toIdx][0] == maxNum && endT < minEndT)) {
                            maxNum = dp[outT][toIdx][0];
                            minEndT = endT;
                            maxT = outT;
                            maxJ = toIdx;
                        }
                    }
                }
            }
        }
        shifts[shiftI][0] = maxNum;
        do {
            shifts[shiftI][maxNum--] = cJobs[maxJ]->idx;
            visited[cJobs[maxJ]->idx] = true;
            int z = maxT;
            maxT = dp[maxT][maxJ][1];
            maxJ = dp[z][maxJ][2];
        } while (maxT != -1);
        if (maxJ != -1) {
            shifts[shiftI][maxNum--] = maxJ;
            visited[maxJ] = true;
        }
        shiftI++;
    }
}

void shiftsToWorkersUsingFreeSpace(int p) {
    for (int i = 0; i < MAX_SHIFTS; i++) {
        for (int h = 1; h <= SHIFT_SIZE(i); h++) {
            jobs[shifts[i][h]].assigned = true;
        }
    }
    for (int i = 0; i < MAX_SHIFTS; i++) {
        if (SHIFT_SIZE(i) == 0) {
            continue;
        }
        int curT, outT;
        int workerSt, workerEnd;
        Job *stJob = &jobs[shifts[i][1]];

        if (!stJob->isEnding) {
            workerSt = nWorkers;
            workerEnd = nWorkers + p - 1;
            dist(base, *stJob, 0, curT);
            for (int j = workerSt; j <= workerEnd; j++) {
                workers[j][0].jobIdx = 1;
                workers[j][1].jobIdx = shifts[i][1];
                workers[j][1].startT = curT - stJob->d;
            }
            nWorkers += p;
        } else {
            workerSt = stJob->stWorker;
            workerEnd = stJob->stWorker + p - 1;
            curT = stJob->l;
        }

        for (int h = 2; h <= SHIFT_SIZE(i); h++) {
            dist(jobs[shifts[i][h - 1]], jobs[shifts[i][h]], curT, outT);
            curT = outT;
            useFreeSpace(workerSt, workerEnd, curT - jobs[shifts[i][h]].d,
                         jobs[shifts[i][h]]);
            for (int j = workerSt; j <= workerEnd; j++) {
                workers[j][0].jobIdx++;
                workers[j][WORKER_SIZE(j)].jobIdx = shifts[i][h];
                workers[j][WORKER_SIZE(j)].startT = curT - jobs[shifts[i][h]].d;
            }
        }
    }
}

void useFreeSpace(int workerSt, int workerEnd, int timeLimit, Job &to) {
    int nW = workerEnd - workerSt + 1;
    int workerArriveT[nW];
    int minArriveT[nW];
    int tmp[nW];
    while (true) {
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
            std::sort(tmp, tmp + nW);
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
                workers[h][WORKER_SIZE(h)].startT =
                    minT - l1Dist(*minJob, to) - minJob->d;
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
                workers[nWorkers][workerPos].startT =
                    curT - jobs[shifts[i][h]].d;
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
        if (jobs[shifts[i][1]].isEnding) {
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

void collectEndings(int p) {
    int counter[n];
    fill(counter, counter + n, 0);
    nEndings = 0;
    for (int i = 0; i < nWorkers; i++) {
        int jobIdx = workers[i][WORKER_SIZE(i)].jobIdx;
        int startT = workers[i][WORKER_SIZE(i)].startT;
        counter[jobIdx]++;
        if (counter[jobIdx] == p) {
            Job *j = &jobs[n + nEndings];
            j->idx = n + nEndings;
            j->x = jobs[jobIdx].x;
            j->y = jobs[jobIdx].y;
            j->d = 0;
            j->p = p;
            j->l = startT + jobs[jobIdx].d;
            j->r = j->l;
            j->adj[0] = j;
            j->adj[1] = j;
            j->isEnding = true;
            j->assigned = false;
            j->stWorker = i - p + 1;
            nEndings++;
        }
    }
}

int evaluate() {
    bool visited[n];
    fill(visited, visited + n, false);
    int cost = 0;
    for (int i = 0; i < nWorkers; i++) {
        cost -= HIRE_COST;
        int startT =
            workers[i][1].startT - l1Dist(base, jobs[workers[i][1].jobIdx]);
        int sz = WORKER_SIZE(i);
        int endT = workers[i][sz].startT +
                   l1Dist(base, jobs[workers[i][sz].jobIdx]) +
                   jobs[workers[i][sz].jobIdx].d;
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
        int startT =
            workers[i][1].startT - l1Dist(base, jobs[workers[i][1].jobIdx]);
        int sz = WORKER_SIZE(i);
        int endT = workers[i][sz].startT +
                   l1Dist(base, jobs[workers[i][sz].jobIdx]) +
                   jobs[workers[i][sz].jobIdx].d;
        cout << "start " << startT << " " << 1 << endl;

        for (int h = 1; h <= sz; h++) {
            Job *job = &jobs[workers[i][h].jobIdx];
            cout << "arrive " << workers[i][h].startT << " " << job->idx + 2
                 << endl;
            cout << "work " << workers[i][h].startT << " "
                 << workers[i][h].startT + job->d;
            cout << " " << job->idx + 2 << endl;
        }
        cout << "arrive " << endT << " " << 1 << endl;
        cout << "end" << endl;
    }
}

int dist(Job &a, Job &b, int curT, int &outT) {
    int l1 = l1Dist(a, b);
    if (a.isBase()) {
        if (b.isEnding) {
            outT = b.l;
            return 0;
        } else {
            outT = b.l + b.d;
            return (l1 + b.d + HIRE_COST) * b.p;
        }
    } else {
        if (b.isEnding) {
            return IMPOSSIBLE_DIST;
        }
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

int l1Dist(Job &a, Job &b) { return abs(a.x - b.x) + abs(a.y - b.y); }