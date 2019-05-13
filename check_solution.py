import argparse
from dataclasses import dataclass

@dataclass
class Job:
    x: int
    y: int
    d: int
    p: int
    l: int
    r: int

def signalError(message):
    print(message)
    exit(1)

def main(inFile, outFile):
    jobs = []
    with open(inFile, "r") as f:
        lines = f.readlines()
        n = int(lines[0])
        for line in lines[1:]:
            jobs.append(Job(*[int(x) for x in line.split(' ')]))

    works = []    
    for i in range(len(jobs)):
        works.append([0, 0, 0])
    with open(outFile, "r") as f:
        lines = f.readlines()
        processing = False
        for line in lines:
            vals = line[:-1].split(' ')
            if vals[0] == 'start':
                if processing:
                    signalError('Unended worker')
                processing = True
                curT = int(vals[1])
                idx = int(vals[2])
                if idx != 1:
                    signalError('Should start at base')
                j = jobs[0]
            elif vals[0] == 'arrive':
                if not processing:
                    signalError('Unstarted worker')
                t = int(vals[1])
                idx = int(vals[2]) - 1
                if curT + abs(j.x - jobs[idx].x) + abs(j.y - jobs[idx].y) > t:
                    signalError('Cant arrive in time' )
                curT = t
                j = jobs[idx]
            elif vals[0] == 'work':
                tSt = int(vals[1])
                tEnd = int(vals[2])
                idx = int(vals[3]) - 1
                if j != jobs[idx]:
                    signalError('Not in possition')
                if tSt < curT:
                    signalError('Bad work time')
                if tEnd - tSt != j.d:
                    signalError('Bad working span')
                if works[idx][0] != 0 and (works[idx][1] != tSt or works[idx][2] != tEnd):
                    signalError('Unsync working')
                curT = tEnd
                works[idx][0] += 1
                works[idx][1] = tSt
                works[idx][2] = tEnd
            elif vals[0] == 'end':
                if j != jobs[0]:
                    signalError('Ending not in base')
                processing = False
            else:
                signalError("Unknown command")
    for i in range(len(jobs)):
        if works[i][0] != 0 and works[i][0] != jobs[i].p:
            signalError("Bad p")
    print("fine")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', type=str, help='Input file')
    parser.add_argument('--output', type=str, help='Output file')
    args = parser.parse_args()
    main(args.input, args.output)
