# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
# retain this notice you can do whatever you want with this stuff. If we meet
# some day, and you think this stuff is worth it, you can buy us a beer in
# return.
# ----------------------------------------------------------------------------

from argparse import ArgumentParser
from pathlib import Path
import re
import json


def parse_cmd_args():
    parser = ArgumentParser(
        prog="kernel2hpp",
        description="Convert OpenCL .cl kernels to C++ .hpp headers to embed kernel code directly in your projects",
    )

    parser.add_argument(
        "-i", "--input", dest="input", required=True, help="input kernel", metavar=""
    )

    parser.add_argument(
        "-o", "--output", dest="output", help="output header", metavar=""
    )

    return parser.parse_args()


def main():
    args = parse_cmd_args()

    with open(args.input) as input:
        kernel_source = input.read()

    filename_without_ext = Path(args.input).with_suffix("").name

    pragmas = re.findall(r"@(\w+).*\((.*)\)", kernel_source)

    pragmap = {}
    for i in pragmas:
        pragmap[i[0]] = json.loads(i[1])

    kernel_class_name = (
        str(filename_without_ext)
        if "kernel" not in pragmap
        else pragmap["kernel"]["name"]
    )
    entry = pragmap["kernel"]["entry"]
    output_path = Path(args.output if args.output is not None else "./")
    macros = {} if "macros" not in pragmap else pragmap["macros"]
    functor_args = ", ".join(pragmap["signature"])

    if output_path.is_dir():
        output_file = Path(str(output_path) + kernel_class_name).with_suffix(".hpp")
    else:
        output_file = output_path

    output_file.parent.mkdir(exist_ok=True, parents=True)
    output_file = str(output_file)

    header_text = (
        "#include <CL/opencl.hpp>\n#include <string>\n#include <utils.hpp>\n\n"
    )
    header_text += "struct {} {{ \n".format(kernel_class_name)
    header_text += "\tusing functor_type = cl::KernelFunctor<{}>;\n\n".format(
        functor_args
    )
    source_args = ["{} {}_param".format(i["type"], i["name"]) for i in macros]

    header_text += '\tstatic std::string source({}) {{\n\t\tstatic const std::string {}_source = R"(\n{})";\n\n'.format(
        ", ".join(source_args), kernel_class_name, kernel_source
    )
    for i in macros:
        macro_name = i["name"]
        header_text += '\t\tauto {0}_macro_def = clutils::kernel_define("{0}", {0}_param);\n'.format(
            macro_name
        )

    header_text += "\t\treturn "

    names = ["{}_macro_def".format(i["name"]) for i in macros]
    names.append("{}_source".format(kernel_class_name))

    header_text += " + ".join(names) + ";\n"

    header_text += "\t}\n\n"
    header_text += '\tstatic std::string entry() {{ return "{}"; }}\n'.format(entry)

    header_text += "};\n"

    with open(output_file, "w") as oput:
        oput.write(header_text)

    print(pragmap)


if __name__ == "__main__":
    main()
