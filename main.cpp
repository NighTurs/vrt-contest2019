#include <bits/stdc++.h>

using namespace std;

#define MAX_JOBS 2000
#define EXTRA_BASES 100
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

void outputTour();

int dist(Job &a, Job &b, int curT, int &outT);

int l1Dist(Job &a, Job &b);

int main(int argc,  char** argv) {

    ifstream cin(argv[1]);
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
    outputTour();
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

int dist(Job &a, Job &b, int curT, int &outT) {
    int l1 = l1Dist(a, b);
    if (a.isBase()) {
        if (b.isBase()) {
            outT = 0;
            return 0;
        } else {
            outT = max(b.l - l1, 0) + b.d + l1;
            return (outT + HIRE_COST) * b.p;
        }
    } else {
        if (b.isBase()) {
            outT = 0;
            return l1 * b.p;
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