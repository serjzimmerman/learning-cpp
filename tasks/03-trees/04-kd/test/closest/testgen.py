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


def generate_random_point(p_halfwidths: np.array, p_round: int, p_dim: int) -> np.array:
    return np.array(
        np.around(
            np.random.default_rng().uniform(-1 * p_halfwidths, 1 * p_halfwidths, p_dim),
            p_round,
        )
    )


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


def generate_test_string(p_points: list, p_queries: list, p_round: int) -> str:
    fmt_string = "{{:.{}f}} ".format(p_round)
    test = "{}\n".format(len(p_points))

    for i in p_points:
        for j in i:
            test += fmt_string.format(j)
        test += "\n"

    test += "{}\n".format(len(p_queries))
    for i in p_queries:
        for j in i:
            test += fmt_string.format(j)
        test += "\n"

    return test


def generate_ans_string(p_indexes: list) -> str:
    ans_str = ""
    for i in p_indexes:
        ans_str += "{}\n".format(i)
    return ans_str


def closest_point(p_points: np.array, p_query: np.array):
    dist = [np.inner(i - p_query, i - p_query) for i in p_points]
    return np.argmin(dist)


def generate_ans(p_points: np.array, p_queires: np.array):
    return [closest_point(p_points, i) for i in p_queires]


def generate_tests(p_config: dict):
    # Create output directory if it doesn't exist.
    os.makedirs(p_config["output_path"], exist_ok=True)
    for group in p_config["groups"]:
        half_dict = group["half"]
        halflengths = np.array(
            [half_dict["x"], half_dict["y"], half_dict["z"], half_dict["w"]]
        )
        for c in range(group["number"]):
            length = np.random.default_rng().integers(
                int(group["length"]["min"]), int(group["length"]["max"])
            )

            points = []
            for i in range(length):
                points.append(generate_random_point(halflengths, group["round"], 4))

            queries = []
            num_queries = group["queries"]
            for i in range(num_queries):
                queries.append(generate_random_point(halflengths, group["round"], 4))

            ans = generate_ans(points, queries)

            save_test_ans(
                p_config["output_path"],
                generate_test_string(points, queries, int(group["round"])),
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
