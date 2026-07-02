import os
import argparse
from pathlib import Path

n_teams = 0
time_stamps = True

# Define paths
folder_prefix = ""
source_folder = ""
output_file = ""

def combine_files():
    # Combine all .txt files
    with open(output_file, "a") as outfile:
        count = 0

        for txt_file in sorted(Path(source_folder).glob("*.txt")):
            
            with open(txt_file, "r") as infile:
                lines = infile.readlines()
                for line in lines:
                    if time_stamps and not line.startswith("2026"):
                        continue
                    elif not time_stamps and line.startswith("2026"):
                        continue

                    outfile.write(line)

                    count += 1
                    if count % 10**6 == 0:
                        print(f"Processed {count//10**6} million lines so far...")

    print(f"Combined {len(list(Path(source_folder).glob('*.txt')))} files, containing {count} lines, into {output_file}")


def main():
    parser = argparse.ArgumentParser(description="Count duplicate lines and frequency of counts.")
    parser.add_argument("n_teams", type=int, help="Number of teams")
    parser.add_argument("file", help="Path to the file to analyze")
    args = parser.parse_args()

    global n_teams
    global folder_prefix
    global source_folder
    global output_file

    n_teams = args.n_teams
    folder_prefix = args.file
    source_folder = folder_prefix + "\\out_" + str(n_teams)

    if not time_stamps:
        output_file = folder_prefix + "sampler_n=" + str(n_teams) + ".txt"
    else:
        output_file = folder_prefix + "sampler_stats_n=" + str(n_teams) + ".txt"

    combine_files()


if __name__ == "__main__":
    main()