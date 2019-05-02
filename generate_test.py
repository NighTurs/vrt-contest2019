import random
import argparse

def rndAxis():
    return random.randint(0, 100)

def rndD():
    return random.randint(5, 30)

def rndP():
    return random.randint(1, 7)

def rndLR():
    l = random.randint(200, 800 - 60)
    diff = random.randint(60, 300)
    r = min(800, l + diff)
    return l, r


def main(seed):
    random.seed(seed)
    n = random.randint(500, 2000)
    print(n)
    print(rndAxis(), rndAxis(), 0, 0, 0, 0)

    for i in range(n):
        l, r = rndLR()
        print(rndAxis(), rndAxis(), rndD(), rndP(), l, r)
        

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('--seed', type=int, help='Seed')
    args = parser.parse_args()
    main(args.seed)