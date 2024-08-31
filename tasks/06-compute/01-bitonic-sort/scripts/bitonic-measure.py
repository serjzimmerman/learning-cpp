#!/usr/bin/python

# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
# retain this notice you can do whatever you want with this stuff. If we meet
# some day, and you think this stuff is worth it, you can buy us a beer in
# return.
# ----------------------------------------------------------------------------

from argparse import ArgumentParser
import re
import json
import subprocess
import matplotlib.pyplot as plt


def parse_cmd_args():
    parser = ArgumentParser(
        prog="bitonic-measure",
        description="Measure performance of our bitonic sort alorithms",
    )

    parser.add_argument(
        "-i",
        "--input",
        dest="input",
        required=True,
        help="input binary file",
        metavar="",
    )

    parser.add_argument(
        "-o", "--output", dest="output", help="Raw data output file", metavar=""
    )

    parser.add_argument(
        "--min",
        dest="min_n",
        help="Minimum test sequence length (in powers of 2)",
        default=17,
        metavar="",
    )
    parser.add_argument(
        "--max",
        dest="max_n",
        help="Maximum test sequence length (in powers of 2)",
        default=25,
        metavar="",
    )

    parser.add_argument(
        "--lsz", dest="lsz", help="Local memory size to use", metavar=""
    )

    return parser.parse_args()


def execute_test(binname: str, kernel: str, n: int, lsz: int) -> str:
    args = (
        binname,
        "--kernel={}".format(kernel),
        "--num={}".format(n),
        "--lsz={}".format(lsz),
    )
    popen = subprocess.Popen(args, stdout=subprocess.PIPE)
    popen.wait()
    output = popen.stdout.read().decode("utf-8")
    return output


def run_test_json_text(binname: str, kernel: str, n: int, lsz: int) -> str:
    output_text = execute_test(binname, kernel, n, lsz)
    output_numbers = list(map(int, re.findall(r"\d+", output_text)))[-4:]
    json_source = f"""{{
\"test\" : {{
        \"lsz\" : {lsz},
        \"len\" : {output_numbers[0]},
        \"std_time\" : {output_numbers[1]},
        \"gpu_wall\" : {output_numbers[2]},
        \"gpu_pure\" : {output_numbers[3]}
}}
}}"""
    return json_source


def run_all_tests_for_kernel(
    binname: str, kernel: str, min_n: int, max_n: int, lsz: int
) -> str:
    json_list = []
    for n in range(min_n, max_n + 1):
        json_list.append(run_test_json_text(binname, kernel, n, lsz))
    return f'"{kernel}s" : [' + ", \n".join(json_list) + "]\n"


def run_all_tests(
    binname: str, kernels_list: list, min_n: int, max_n: int, max_lsz: int
) -> str:
    json_source_list = []
    for kernel in kernels_list:
        json_source_list.append(
            run_all_tests_for_kernel(binname, kernel, min_n, max_n, max_lsz)
        )
    return "{\n" + ",\n".join(json_source_list) + "}"


def write_to_measures_json_file(filename: str, json_obj: str) -> None:
    with open(filename, "w") as output:
        output.write(json_obj)


def plot_measurements_of_kernel(lsz: int, kernel_list: list, data) -> None:
    fig, ax = plt.subplots()

    ax.set_xscale("symlog", base=2)
    ax.grid()

    first = True
    lens, gpu_times, cpu_times = [], [], []

    for kernel in kernel_list:
        gpu_times.clear()
        lens.clear()
        for test in data[kernel + "s"]:
            lens.append(test["test"]["len"])
            gpu_times.append(test["test"]["gpu_wall"] / 1000)
            if first:
                cpu_times.append(test["test"]["std_time"] / 1000)
        plt.plot(lens, gpu_times, marker="o", label=f"{kernel} bitonic sort")
        first = False

    plt.plot(
        lens, cpu_times, marker="o", label="cpu sort (either std:: or __gnu_parallel::)"
    )

    plt.xlabel("Length of the test, (log2 scale)")
    plt.ylabel("Time spent, s")

    plt.title("Local size = {}".format(lsz))

    ax.legend()
    plt.show()


def plot_measurements(lsz: int, filename: str, kernel_list: list) -> None:
    with open(filename) as json_file:
        data = json.load(json_file)
    plot_measurements_of_kernel(lsz, kernel_list, data)


def main():
    args = parse_cmd_args()
    kernel_list = ["local", "naive"]
    json_source = run_all_tests(
        args.input, kernel_list, int(args.min_n), int(args.max_n), int(args.lsz)
    )
    write_to_measures_json_file(args.output, json_source)
    plot_measurements(args.lsz, args.output, kernel_list)


if __name__ == "__main__":
    main()
