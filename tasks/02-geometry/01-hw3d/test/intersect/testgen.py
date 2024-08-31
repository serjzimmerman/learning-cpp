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
import glm
import fcl


def generate_random_triangle(
    p_mean: float, p_std: float, p_halfwidths: np.array, p_round: int
) -> np.array:
    # Random lengths for distance of points A, B and C from origin
    length_a = np.abs(np.random.default_rng().normal(p_mean, p_std))
    length_b = np.abs(np.random.default_rng().normal(p_mean, p_std))
    length_c = np.abs(np.random.default_rng().normal(p_mean, p_std))

    # Create respective vectors for points in a sort 120 degree intervals. TODO[]: Improve
    a = glm.vec3(length_a, 0, 0)
    b = glm.vec3(-np.sqrt(3) / 2 * length_b, length_b / 2, 0)
    c = glm.vec3(-np.sqrt(3) / 2 * length_c, -length_c / 2, 0)

    # Create a transofrmation matrix that moves the points to a random point inside the bounding volume and rotates sporadically
    mat = glm.mat4(1)
    translation_vec = glm.vec3(
        np.random.default_rng().uniform(-p_halfwidths[0], p_halfwidths[0]),
        np.random.default_rng().uniform(-p_halfwidths[1], p_halfwidths[1]),
        np.random.default_rng().uniform(-p_halfwidths[2], p_halfwidths[2]),
    )
    mat = glm.translate(mat, translation_vec)

    # Randombly rotate triangles
    for i in range(3):
        rotating_vec = glm.vec3(np.random.default_rng().uniform(-1, 1, 3))
        mat = glm.rotate(
            mat, np.random.default_rng().uniform(0, 2 * np.pi), rotating_vec
        )

    return np.array(
        [
            np.around(np.array(mat * a), p_round),
            np.around(np.array(mat * b), p_round),
            np.around(np.array(mat * c), p_round),
        ]
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


def generate_test_string(p_triangles: list, p_round: int) -> str:
    test = "{} ".format(len(p_triangles))
    fmt_string = "{{:.{}f}} ".format(p_round)
    for i in p_triangles:
        for j in range(3):
            for k in range(3):
                test += fmt_string.format(i[j][k])
    return test


def generate_ans_string(p_ans: list) -> str:
    ans = ""
    for i in p_ans:
        ans += "{} ".format(i)
    return ans


def generate_ans(p_triangles: list) -> list:
    triangles = []
    objs = []
    # Fill geometry and object lists

    id_index_map = {}
    obj_index_map = {}
    for i in range(len(p_triangles)):
        tri = p_triangles[i]
        geom = fcl.BVHModel()
        tris = np.array([[0, 1, 2]])

        geom.beginModel(len(tri), len(tris))
        geom.addSubModel(np.array([tri[0], tri[1], tri[2]]), tris)
        geom.endModel()

        obj = fcl.CollisionObject(geom)
        triangles.append(geom)
        objs.append(obj)

        id_index_map[id(geom)] = i
        obj_index_map[id(obj)] = i

    manager = fcl.DynamicAABBTreeCollisionManager()
    manager.registerObjects(objs)
    manager.setup()

    crequest = fcl.CollisionRequest(
        num_max_contacts=len(p_triangles) ** 2, enable_contact=True
    )
    cdata = fcl.CollisionData(crequest, fcl.CollisionResult())
    manager.collide(cdata, fcl.defaultCollisionCallback)

    objs_in_collision = set()
    # FIXME: Find out what this code does
    # for i in range(len(p_triangles)):
    #     for j in range(i + 1, len(p_triangles)):
    #         if fcl.collide(objs[i], objs[j]):
    #             if obj_index_map[id(objs[i])] not in objs_in_collision:
    #                 objs_in_collision.add(i)
    #             if obj_index_map[id(objs[j])] not in objs_in_collision:
    #                 objs_in_collision.add(j)

    for contact in cdata.result.contacts:
        coll_geom_0 = contact.o1
        coll_geom_1 = contact.o2

        if id_index_map[id(coll_geom_0)] not in objs_in_collision:
            objs_in_collision.add(id_index_map[id(coll_geom_0)])
        if id_index_map[id(coll_geom_1)] not in objs_in_collision:
            objs_in_collision.add(id_index_map[id(coll_geom_1)])

    return [x for x in objs_in_collision]


def generate_tests(p_config: dict):
    # Create output directory if it doesn't exist.
    os.makedirs(p_config["output_path"], exist_ok=True)
    for group in p_config["groups"]:
        half_dict = group["half"]
        halflengths = np.array([half_dict["x"], half_dict["y"], half_dict["z"]])
        for c in range(group["number"]):
            length = np.random.default_rng().integers(
                int(group["length"]["min"]), int(group["length"]["max"])
            )
            triangles = []
            for i in range(length):
                triangles.append(
                    generate_random_triangle(
                        group["mean"], group["std"], halflengths, int(group["round"])
                    )
                )
            ans = generate_ans(triangles)
            save_test_ans(
                p_config["output_path"],
                generate_test_string(triangles, int(group["round"])),
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
