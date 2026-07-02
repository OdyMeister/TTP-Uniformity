import argparse
import os
import sys


def main():
    parser = argparse.ArgumentParser(
        description='Remove the first n*2 characters from each line of a file and write the result to a new file.'
    )
    parser.add_argument('n', type=int, help='Number of character pairs to remove from each line.')
    parser.add_argument('input_file', help='Path to the input file.')
    parser.add_argument('output_file', help='Path to the output file.')
    args = parser.parse_args()

    if args.n < 0:
        parser.error('n must be non-negative')

    if not os.path.isfile(args.input_file):
        parser.error(f'Input file not found: {args.input_file}')

    cut = args.n * 2

    with open(args.input_file, 'r', encoding='utf-8') as infile, open(args.output_file, 'w', encoding='utf-8') as outfile:
        count = 0
        for line in infile:
            if count % 10**6 == 0 and count > 0:
                print(f'Processed {count//10**6} million lines so far...')
            if cut >= len(line):
                # Preserve line count by writing an empty line if the original line ended with a newline.
                outfile.write('\n' if line.endswith('\n') else '')
            else:
                outfile.write(line[cut:])
            count += 1

    print(f'Wrote trimmed file to: {args.output_file}')


if __name__ == '__main__':
    main()
