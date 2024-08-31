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


def generate_matrix_rand_det_int(
    p_right: int, p_minsize: int, p_maxsize: int, p_shuffles: int, p_additions: int
):
    # Generate random array of matrix's diag elements
    rng = np.random.default_rng()

    size = rng.integers(p_minsize, p_maxsize)

    diag_elements = rng.integers(low=1, high=p_right, size=size)
    print(diag_elements)
    for i in diag_elements:
        i *= np.random.choice([1, 2])

    matrix = np.diag(diag_elements)
    determinant = np.prod(diag_elements)

    for i in range(p_additions):
        pair = np.random.choice(a=np.arange(size), size=2, replace=False)
        sign = np.random.choice([-1, 1])
        matrix[pair[0]] += sign * matrix[pair[1]]

    for i in range(p_shuffles):
        pair = np.random.choice(a=np.arange(size), size=2, replace=False)
        matrix[[pair[0], pair[1]]] = matrix[[pair[1], pair[0]]]
        determinant *= -1

    return [matrix, determinant]


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


def generate_test_string(p_matrix: np.array) -> str:
    test = "{} \n".format(np.shape(p_matrix)[0])

    for row in p_matrix:
        for elem in row:
            test += "{}\t".format(elem)
        test += "\n"
    return test


def generate_ans_string(p_ans) -> str:
    ans = "{}".format(p_ans)
    return ans


def generate_tests(p_config: dict):
    # Create output directory if it doesn't exist.
    os.makedirs(p_config["output_path"], exist_ok=True)
    for group in p_config["groups"]:
        for c in range(group["number"]):
            test_ans_list = generate_matrix_rand_det_int(
                group["diag"]["right"],
                group["size"]["min"],
                group["size"]["max"],
                group["shuffles"],
                group["additions"],
            )

            ans = test_ans_list[1]
            test = test_ans_list[0]

            save_test_ans(
                p_config["output_path"],
                generate_test_string(test),
                generate_ans_string(ans),
                c,
                group["test_fmt_string"],
                group["ans_fmt_string"],
            )


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
