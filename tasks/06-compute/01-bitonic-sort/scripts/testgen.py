#!/usr/bin/python

# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
# retain this notice you can do whatever you want with this stuff. If we meet
# some day, and you think this stuff is worth it, you can buy us a beer in
# return.
# ----------------------------------------------------------------------------

from argparse import ArgumentParser
import numpy as np


def parse_cmd_args():
    parser = ArgumentParser(
        prog="bitonic-measure",
        description="Measure performance of our bitonic sort alorithms",
    )

    parser.add_argument(
        "-f",
        "--folder",
        dest="folder",
        required=True,
        help="Folder to write to",
        metavar="",
    )

    parser.add_argument(
        "--count",
        dest="count",
        help="Number of tests to generate",
        default=12,
        metavar="",
    )
    parser.add_argument(
        "--min",
        dest="min_n",
        help="Minimum test sequence length",
        default=0,
        metavar="",
    )
    parser.add_argument(
        "--max",
        dest="max_n",
        help="Maximum test sequence length",
        default=2**20,
        metavar="",
    )
    parser.add_argument(
        "--lower",
        dest="lower",
        help="Minimum test sequence length",
        default=-(2**25),
        metavar="",
    )
    parser.add_argument(
        "--upper",
        dest="upper",
        help="Maximum test sequence length",
        default=2**25,
        metavar="",
    )

    return parser.parse_args()


def main():
    args = parse_cmd_args()

    for i in range(args.count):
        arr = np.random.randint(
            args.lower, args.upper, np.random.randint(args.min_n, args.max_n)
        )

        with open(args.folder + "/test{}.dat".format(i), "w") as input_data:
            input_data.write("{} ".format(len(arr)))
            input_data.write(" ".join([str(j) for j in arr]))

        arr.sort()

        with open(args.folder + "/test{}.dat.ans".format(i), "w") as output_data:
            output_data.write(" ".join([str(j) for j in arr]))


if __name__ == "__main__":
    main()
