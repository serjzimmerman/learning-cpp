##
# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
# retain this notice you can do whatever you want with this stuff. If we meet
# some day, and you think this stuff is worth it, you can buy us a beer in
# return.
# ----------------------------------------------------------------------------
##

import argparse
import json
import os
import numpy as np


def save_test_ans(
    output_path: str,
    p_test: str,
    p_ans: str,
    p_idx: int,
    p_test_fmt: str = "naive{}.dat",
    p_ans_fmt: str = "naive{}.dat.ans",
) -> None:
    with open(os.path.join(output_path, p_test_fmt.format(p_idx)), "w") as test_fp:
        test_fp.write(p_test)

    with open(os.path.join(output_path, p_ans_fmt.format(p_idx)), "w") as test_fp:
        test_fp.write(p_ans)


def count_less_than(p_list: list, p_num: int):
    acc = 0
    for i in range(len(p_list)):
        if p_list[i] < p_num:
            acc += 1
        else:
            break
    return acc


def generate_random_test(p_config: dict):
    length = np.random.default_rng().integers(
        p_config["elements"]["min"], p_config["elements"]["max"]
    )
    elements = list(
        set(
            np.random.default_rng().integers(
                p_config["integers"]["min"], p_config["integers"]["max"], length
            )
        )
    )
    length = len(elements)

    test_str = ""
    for i in elements:
        test_str += "k {} ".format(i)

    elements.sort()
    queries_min = np.random.default_rng().integers(
        p_config["min_queries"]["min"], p_config["min_queries"]["max"]
    )

    ans_str = ""
    for i in range(queries_min):
        index = np.random.default_rng().integers(1, length)
        ans_str += "{} ".format(elements[index - 1])
        test_str += "m {} ".format(index)

    queries_less = np.random.default_rng().integers(
        p_config["less_queries"]["min"], p_config["less_queries"]["max"]
    )

    for i in range(queries_less):
        key = index = np.random.default_rng().integers(elements[1], elements[-1])
        ans_str += "{} ".format(count_less_than(elements, key))
        test_str += "n {} ".format(key)

    return [test_str, ans_str]


def generate_tests(p_config: dict) -> None:
    # Create output directory if it doesn't exist.
    os.makedirs(p_config["output_path"], exist_ok=True)
    number = int(p_config["number"])

    for i in range(number):
        l = generate_random_test(p_config)
        save_test_ans(p_config["output_path"], l[0], l[1], i + 1)


def main():
    parser = argparse.ArgumentParser(description="Generate quieries end-to-end tests.")
    parser.add_argument(
        "config_path", metavar="config", type=str, nargs=1, help="Path to config file."
    )
    args = parser.parse_args()

    config_path = str(args.config_path[0])
    if not os.path.isfile(config_path):
        raise Exception("File does not exist :(")

    with open(config_path) as json_file:
        config_settings = json.load(json_file)
    generate_tests(config_settings)


if __name__ == "__main__":
    main()
