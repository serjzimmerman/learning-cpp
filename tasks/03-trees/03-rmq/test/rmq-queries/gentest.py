##
# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
# retain this notice you can do whatever you want with this stuff. If we meet
# some day, and you think this stuff is worth it, you can buy me a beer in
# return.
# ----------------------------------------------------------------------------
##

import argparse
import json
import os
import numpy as np


def generate_ans(p_items: list, p_req: list) -> list:
    ans = []
    for req in p_req:
        ans.append(p_items[np.argmin(p_items[req[0] : (req[1] + 1)]) + req[0]])
    return ans


def save_test(
    output_path: str,
    p_items: list,
    p_req: list,
    p_idx: int,
    p_test_fmt: str = "test{}.dat",
    p_ans_fmt: str = "test{}.dat.ans",
) -> None:
    test_string = "{} ".format(len(p_items))
    np.random.shuffle(p_items)
    for i in p_items:
        test_string += str(i) + " "

    test_string += str(len(p_req)) + " "
    for j in p_req:
        test_string += "{} {} ".format(j[0], j[1])

    with open(os.path.join(output_path, p_test_fmt.format(p_idx)), "w") as test_fp:
        test_fp.write(test_string)

    ans = generate_ans(p_items, p_req)
    ans_string = ""
    for k in ans:
        ans_string += "{} ".format(k)

    with open(os.path.join(output_path, p_ans_fmt.format(p_idx)), "w") as test_fp:
        test_fp.write(ans_string)


def generate_request(p_list: np.array, p_config: dict) -> list:
    length = np.random.default_rng().integers(p_config["lower"], p_config["upper"])

    req = []
    for i in range(length):
        lower = np.random.default_rng().integers(0, len(p_list) - 2)
        upper = np.random.default_rng().integers(lower + 1, len(p_list) - 1)
        req.append([lower, upper])

    return req


def generate_normal(
    p_config: dict,
    p_old_idx: int,
    p_test_fmt: str = "normal{}.dat",
    p_ans_fmt: str = "normal{}.dat.ans",
) -> int:
    normal_dict = p_config["normal"]

    mean = normal_dict["mean"]
    deviation = normal_dict["deviation"]
    length = normal_dict["length"]

    for i in range(normal_dict["number"]):
        sample = list(
            map(int, (np.random.default_rng().normal(mean, deviation, length)))
        )
        sample.sort()
        requests = generate_request(sample, normal_dict["requests"])
        save_test(
            p_config["output_path"], sample, requests, p_old_idx, p_test_fmt, p_ans_fmt
        )

        p_old_idx += 1
    return p_old_idx


def generate_uniform(
    p_config: dict,
    p_old_idx: int,
    p_test_fmt: str = "uniform{}.dat",
    p_ans_fmt: str = "uniform{}.dat.ans",
) -> int:
    uniform_dict = p_config["uniform"]

    upper = uniform_dict["upper"]
    lower = uniform_dict["lower"]
    length = uniform_dict["length"]

    for i in range(uniform_dict["number"]):
        sample = list(map(int, np.random.default_rng().integers(lower, upper, length)))
        sample.sort()

        requests = generate_request(sample, uniform_dict["requests"])
        save_test(
            p_config["output_path"], sample, requests, p_old_idx, p_test_fmt, p_ans_fmt
        )

        p_old_idx += 1
    return p_old_idx


def generate_triangle(
    p_config: dict,
    p_old_idx: int,
    p_test_fmt: str = "triangle{}.dat",
    p_ans_fmt: str = "triangle{}.dat.ans",
) -> int:
    triangle_dict = p_config["triangle"]

    left = triangle_dict["left"]
    right = triangle_dict["right"]
    mode = triangle_dict["mode"]
    length = triangle_dict["length"]

    for i in range(triangle_dict["number"]):
        sample = list(
            map(
                int,
                np.random.default_rng().triangular(
                    left=left, right=right, mode=mode, size=length
                ),
            )
        )
        sample.sort()

        requests = generate_request(sample, triangle_dict["requests"])
        save_test(
            p_config["output_path"], sample, requests, p_old_idx, p_test_fmt, p_ans_fmt
        )

        p_old_idx += 1
    return p_old_idx


def generate_tests(p_config: dict) -> None:
    # Create output directory if it doesn't exist.
    os.makedirs(p_config["output_path"], exist_ok=True)

    idx = generate_normal(p_config, 0)
    idx = generate_uniform(p_config, idx)
    idx = generate_triangle(p_config, idx)


def main():
    parser = argparse.ArgumentParser(description="Generate belady end-to-end tests.")
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
