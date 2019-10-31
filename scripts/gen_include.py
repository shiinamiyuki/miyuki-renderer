import os
import shutil
import sys

INCLUDE_PATH = '../include'
API_PATH = '../src/api'


def gen():
    if os.path.exists(INCLUDE_PATH):
        shutil.rmtree('../include')
    shutil.copytree(API_PATH, INCLUDE_PATH + '/miyuki/api')


def clean():
    if os.path.exists(INCLUDE_PATH):
        shutil.rmtree('../include')


if __name__ == '__main__':
    if len(sys.argv) == 2:
        if sys.argv[1] == '-c':
            clean()
        else:
            print('usage: gen_include [-c]')
    else:
        gen()
