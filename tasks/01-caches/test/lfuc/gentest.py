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


class lfu:
    def __init__(self, p_size: int) -> None:
        self.m_size = p_size
        self.m_freq_map = {}
        self.m_key_freq_map = {}
        self.m_hits = 0
        self.m_used = 0

    def lookup(self, p_item):
        # Case 1. Element is present in the cache. First we increment "self.hits". Next we promote the element:
        # 1) Remove p_item from corresponding list.
        # 2) Update entry in "self.m_key_freq_map" to the incremented frequency.
        # 3) Insert p_item to the next list.
        if p_item in self.m_key_freq_map:
            self.m_hits += 1
            freq = self.m_key_freq_map[p_item]
            self.m_freq_map[freq].remove(p_item)
            if len(self.m_freq_map[freq]) == 0:
                self.m_freq_map.pop(freq)
            self.m_key_freq_map[p_item] = freq + 1
            if freq + 1 not in self.m_freq_map:
                self.m_freq_map[freq + 1] = []
            self.m_freq_map[freq + 1].insert(0, p_item)
            return

        # Case 2. Cache is not full and element is not present currently.
        if self.m_used < self.m_size:
            self.m_key_freq_map[p_item] = 1
            if 1 not in self.m_freq_map:
                self.m_freq_map[1] = []
            self.m_freq_map[1].insert(0, p_item)
            self.m_used += 1
            return

        # Case 3. Cache is full. Then we choose an element to evict and insert the new one.
        least_freq = min(self.m_freq_map.keys())
        to_evict = self.m_freq_map[least_freq].pop()
        if len(self.m_freq_map[least_freq]) == 0:
            self.m_freq_map.pop(least_freq)
        self.m_key_freq_map.pop(to_evict)
        self.m_key_freq_map[p_item] = 1
        if 1 not in self.m_freq_map:
            self.m_freq_map[1] = []
        self.m_freq_map[1].insert(0, p_item)


def get_lfu_hits(p_list: list, p_size: int) -> int:
    cache = lfu(p_size)

    for i in p_list:
        cache.lookup(i)

    return cache.m_hits


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

    ans = get_lfu_hits(p_test, p_size)
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
