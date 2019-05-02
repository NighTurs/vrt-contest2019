#include <bits/stdc++.h>

using namespace std;

#define MAX_JOBS 2000
#define EXTRA_BASES 200
#define HIRE_COST 240
#define IMPOSSIBLE_DIST -1

typedef struct {
    int idx;
    int x, y, d, p, l, r;

    bool isBase() {
        return d == 0;
    }
} Job;


int n;
Job jobs[MAX_JOBS + EXTRA_BASES];
int tour[MAX_JOBS + EXTRA_BASES];

void greedyTour();

int evaluateTour();

void outputTour();

void outputAnalytics();

int dist(Job &a, Job &b, int curT, int &outT);

int l1Dist(Job &a, Job &b);

int main(int argc,  char** argv) {

    ifstream cin(argv[2]);
    ios_base::sync_with_stdio(false);
    std::cin.tie(0);
    
    cin >> n;

    for (int i = 0; i < n; i++) {
        Job* j = &jobs[i];
        j->idx = i;
        cin >> j->x >> j->y >> j->d >> j->p >> j->l >> j->r;
    }

    // add extra bases
    for (int i = n; i < n + EXTRA_BASES; i++) {
        Job* j = &jobs[i];
        j->idx = i; 
        j->x = jobs[0].x;
        j->y = jobs[0].y;
        j->d = 0;
    }

    n = n + EXTRA_BASES;

    greedyTour();
    if (argc > 1) {
        if (argv[1][0] == '1') {
            cout << evaluateTour() << endl;    
        } else if (argv[1][0] == '0') {
            outputTour();
        } else if (argv[1][0] == '2') {
            outputAnalytics();
        }
    } else {
        outputTour();
    }
}

void greedyTour() {
    bool visited[n] = {false};
    Job *cur = &jobs[0];
    visited[0] = true;
    tour[0] = 0;
    int tourIdx = 1;
    int curT = 0, outT = 0;

    while (true) {
        int minT = __INT_MAX__;
        Job *minJob = NULL;
        for (int i = 0; i < n - EXTRA_BASES; i++) {
            if (visited[i] || (!cur->isBase() && cur->p != jobs[i].p)) {
                continue;
            }
            if (dist(*cur, jobs[i], curT, outT) != IMPOSSIBLE_DIST && outT < minT) {
                minT = outT;
                minJob = &jobs[i];
            }
        }
        if (minT == __INT_MAX__) {
            for (int i = n - EXTRA_BASES; i < n; i++) {
                if (visited[i]) {
                    continue;
                }
                minJob = &jobs[i];
                minT = 0;
                break;
            }
        }
        if (minT == __INT_MAX__) {
            break;
        }
        tour[tourIdx++] = minJob->idx;
        curT = minT;
        cur = minJob;
        visited[minJob->idx] = true;
    }
}

int evaluateTour() {
    int cost = 0;
    int curT = 0, outT;
    for (int i = 1; i < n; i++) {
        cost -= dist(jobs[tour[i - 1]], jobs[tour[i]], curT, outT);
        curT = outT;
        if (!jobs[tour[i]].isBase()) {
            cost += jobs[tour[i]].d * jobs[tour[i]].p * (jobs[tour[i]].p + 5);
        }
    }
    return cost;
}

void outputTour() {
    int i = 0;
    int lastBaseIdx = 0;
    int curT = 0;
    int loop = 0;
    bool shouldEnd = false;
    while (i < n) {
        Job *cur = &jobs[tour[i]];
        Job *prev = i == 0 ? NULL : &jobs[tour[i - 1]];
        Job *next = i == n - 1 ? NULL : &jobs[tour[i + 1]];
        
        if (cur->isBase()) {
            if (shouldEnd) {
                int l1 = l1Dist(*prev, *cur);
                cout << "arrive " << curT + l1 << " " << 1 << endl; 
                cout << "end" << endl;
                loop--;
                if (loop > 0) {
                    i = lastBaseIdx;
                    shouldEnd = false;
                    continue;
                }
            }
            if (next == NULL || next->isBase()) {
                i++;
                shouldEnd = false;
                continue;
            }
            int outT;
            int l1 = l1Dist(*cur, *next);
            dist(*cur, *next, 0, outT);
            curT = outT - next->d - l1;
            cout << "start " << curT << " " << 1 << endl;
            if (loop == 0) {
                loop = next->p;
                lastBaseIdx = i;
            }
        } else {
            int l1 = l1Dist(*prev, *cur);
            cout << "arrive " << curT + l1 << " " << cur->idx + 1 << endl;
            int outT;
            dist(*prev, *cur, curT, outT);
            curT = outT;
            cout << "work " << curT - cur->d << " " << curT << " " << cur->idx + 1 << endl;
        }
        shouldEnd = true;
        i++;
    }
}

void outputAnalytics() {

    int ctByBase[n] {0};
    int baseReturnCost[n] {0}; 
    int jobBaseIdx[n] {0};
    int jobProfit[n] {0};
    int jobExpenses[n] {0};

    int cost = 0;
    int curT = 0, outT;
    int lastBaseIdx = 0;

    for (int i = 1; i < n; i++) {
        Job *a = &jobs[tour[i - 1]];
        Job *b = &jobs[tour[i]];

        jobExpenses[b->idx] = dist(*a, *b, curT, outT);
        if (a->isBase()) {
            jobExpenses[b->idx] -= b->p * HIRE_COST;
        }
        jobBaseIdx[b->idx] = lastBaseIdx;
        if (b->isBase()) {
            baseReturnCost[lastBaseIdx] = jobExpenses[b->idx];
            lastBaseIdx = b->idx;
        } else {
            ctByBase[lastBaseIdx]++;
        }
        curT = outT;
        if (!b->isBase()) {
            jobProfit[b->idx] = b->d * b->p * (b->p + 5);
        }
    }
    for (int i = 0; i < n; i++) {
        if (jobs[i].isBase()) {
            continue;
        }
        cout << i << ',' << jobs[i].d << ',' << jobs[i].p << ',' << jobProfit[i] << ',';
        cout << jobExpenses[i] << ',' <<  ctByBase[jobBaseIdx[i]] << ',' << baseReturnCost[jobBaseIdx[i]] << endl;
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