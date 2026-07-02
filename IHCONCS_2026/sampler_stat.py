#!/usr/bin/env python3
import sys

def read_values(path):
    values = []
    with open(path, "r", encoding="utf-8") as f:
        lines = f.readlines()[1:]

    for line in lines:
        line = line.strip()
        parts = line.split()

        if len(parts) < 3:
            continue

        values.append(int(parts[-1]))

    return values


def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <file1> <file2>")
        sys.exit(1)

    RF_values = read_values(sys.argv[1])
    DFS_values = read_values(sys.argv[2])

    RF_values_non_cumulative = [RF_values[i] - RF_values[i - 1] if RF_values[i] - RF_values[i - 1] > 0 else RF_values[i] for i in range(1, len(RF_values))]

    average = sum(RF_values_non_cumulative) / len(RF_values_non_cumulative)
    print("Average attempts: \t\t", average)

    average = sum(DFS_values) / len(DFS_values)
    print("Average recursive calls: \t", average)
    print("Total recursive calls: \t\t", sum(DFS_values))


if __name__ == "__main__":
    main()
