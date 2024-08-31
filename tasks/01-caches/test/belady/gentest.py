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


class fixed_set:
    def __init__(self, p_size: int):
        self.m_size = p_size
        self.m_set = set()

    def __contains__(self, p_item):
        return p_item in self.m_set

    def __iter__(self):
        for item in self.m_set:
            yield item

    def full(self) -> bool:
        return len(self.m_set) == self.m_size

    def insert(self, p_item):
        if len(self.m_set) > self.m_size:
            raise Exception("U dumb")

        self.m_set.add(p_item)

    def remove(self, p_item):
        self.m_set.remove(p_item)


def find_latest_used(p_list: list, p_curr_set: fixed_set, p_curr_pos: int):
    latest_item = None
    latest_used = None

    cut_list = p_list[p_curr_pos:]

    for item in p_curr_set:
        if item not in cut_list:
            return item

        if latest_item is None:
            latest_item = item
            latest_used = cut_list.index(item)
            continue

        new_index = cut_list.index(item)
        if new_index > latest_used:
            latest_used = new_index
            latest_item = item

    if latest_item is None:
        raise Exception("U dumb")

    return latest_item


def get_optimal_hits(p_list: list, p_size: int) -> int:
    currently_cached = fixed_set(p_size)

    hits = 0

    for curr in range(0, len(p_list)):
        item = p_list[curr]

        if item in currently_cached:
            hits += 1
            continue

        if not currently_cached.full():
            currently_cached.insert(item)
            continue

        latest_used = find_latest_used(p_list, currently_cached, curr)

        currently_cached.remove(latest_used)
        currently_cached.insert(item)

    return hits


def save_test_ans(
    output_path: str,
    p_test: list,
    p_size: int,
    p_idx: int,
    p_test_fmt: str = "test{}.dat",
    p_ans_fmt: str = "ans{}.dat",
) -> None:
    test_string = "{} {} ".format(p_size, len(p_test))

    for item in p_test:
        test_string += "{} ".format(item)

    with open(os.path.join(output_path, p_test_fmt.format(p_idx)), "w") as test_fp:
        test_fp.write(test_string)

    ans = get_optimal_hits(p_test, p_size)
    ans_string = "{}".format(ans)

    with open(os.path.join(output_path, p_ans_fmt.format(p_idx)), "w") as test_fp:
        test_fp.write(ans_string)


def generate_normal(p_config: dict, p_old_idx: int) -> int:
    normal_dict = p_config["normal"]

    mean = normal_dict["mean"]
    deviation = normal_dict["deviation"]
    length = normal_dict["length"]

    for i in range(normal_dict["number"]):
        sample = list(
            map(int, list(np.random.default_rng().normal(mean, deviation, length)))
        )

        size = np.random.default_rng().integers(
            p_config["size"]["lower"], p_config["size"]["upper"]
        )
        save_test_ans(p_config["output_path"], sample, size, p_old_idx)

        p_old_idx += 1
    return p_old_idx


def generate_uniform(p_config: dict, p_old_idx: int) -> int:
    uniform_dict = p_config["uniform"]

    upper = uniform_dict["upper"]
    lower = uniform_dict["lower"]
    length = uniform_dict["length"]

    for i in range(uniform_dict["number"]):
        sample = list(
            map(int, list(np.random.default_rng().integers(lower, upper, length)))
        )

        size = np.random.default_rng().integers(
            p_config["size"]["lower"], p_config["size"]["upper"]
        )
        save_test_ans(p_config["output_path"], sample, size, p_old_idx)

        p_old_idx += 1
    return p_old_idx


def generate_triangle(p_config: dict, p_old_idx: int) -> int:
    triangle_dict = p_config["triangle"]

    left = triangle_dict["left"]
    right = triangle_dict["right"]
    mode = triangle_dict["mode"]
    length = triangle_dict["length"]

    for i in range(triangle_dict["number"]):
        sample = list(
            map(
                int,
                list(
                    np.random.default_rng().triangular(
                        left=left, right=right, mode=mode, size=length
                    )
                ),
            )
        )

        size = np.random.default_rng().integers(
            p_config["size"]["lower"], p_config["size"]["upper"]
        )
        save_test_ans(p_config["output_path"], sample, size, p_old_idx)

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
