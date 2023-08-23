import argparse
import os
from typing import *

T = TypeVar('T')

SOURCE_FILES_SEARCH_DIRECTORIES = ['src', 'test']
LICENSE_HEADER_END = ' * along with this program. If not, see http://gnu.org/licenses'

def is_source_file(file: str) -> bool:
    return file.endswith('.cpp') or file.endswith('.h')


def find_all_source_files(directories: list[str]) -> str:
    for directory in directories:
        for root, found_dirs, found_files in os.walk(directory):
            for file in found_files:
                if is_source_file(file):
                    path = os.path.join(root, file)
                    yield path


def find_license_header_end(lines: list[str]) -> Optional[int]:
    for idx, line in enumerate(lines):
        if LICENSE_HEADER_END in line:
            return idx + 2 # One more line with '*/' and idx past end
    return None


def remove_range(a_list: list[T], begin: int, end: int) -> list[str]:
    return a_list[0:begin] + a_list[end:]


def remove_license_header(lines: list[str]) -> list[str]:
    return remove_range(lines, begin=0, end=find_license_header_end(lines))


def add_license_header_at_beginning(lines: list[str], new_license_header: list[str]) -> list[str]:
    return new_license_header + lines


def update_license_header(lines: list[str], new_license_header: list[str]) -> list[str]:
    return add_license_header_at_beginning(remove_license_header(lines), new_license_header)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('license_header', help='Path to the new license header file', type=str)
    args = parser.parse_args()
    new_license_header = open(args.license_header).readlines()
    for source_file in find_all_source_files(SOURCE_FILES_SEARCH_DIRECTORIES):
        print(source_file)
        lines = update_license_header(open(source_file).readlines(), new_license_header)
        open(source_file, 'w', newline='\n').writelines(lines)


if __name__ == '__main__':
    main()
