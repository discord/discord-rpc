#!/usr/bin/env python

import click
import os
import subprocess
import sys
import shutil
import zipfile
from contextlib import contextmanager


SCRIPT_PATH = os.path.dirname(os.path.abspath(__file__))


@contextmanager
def cd(new_dir):
    """ Temporarily change current directory """
    if new_dir:
        old_dir = os.getcwd()
        os.chdir(new_dir)
    yield
    if new_dir:
        os.chdir(old_dir)


def mkdir_p(path):
    """ mkdir -p """
    if not os.path.isdir(path):
        os.makedirs(path)


def build_lib(build_name, generator, options):
    build_path = os.path.join(SCRIPT_PATH, 'builds', build_name)
    install_path = os.path.join(SCRIPT_PATH, 'builds', 'install', build_name)
    mkdir_p(build_path)
    mkdir_p(install_path)
    with cd(build_path):
        initial_cmake = ['cmake', SCRIPT_PATH, '-DCMAKE_INSTALL_PREFIX=%s' % os.path.join('..', 'install', build_name)]
        if generator:
            initial_cmake.extend(['-G', generator])
        for key in options:
            val = 'ON' if options[key] else 'OFF'
            initial_cmake.append('-D%s=%s' %(key, val))
        subprocess.check_call(initial_cmake)
        subprocess.check_call(['cmake', '--build', '.', '--config', 'Debug'])
        subprocess.check_call(['cmake', '--build', '.', '--config', 'Release', '--target', 'install'])


def create_archive():
    archive_file_path = os.path.join(SCRIPT_PATH, 'builds', 'discord-rpc-%s.zip' % sys.platform)
    archive_file = zipfile.ZipFile(archive_file_path, 'w', zipfile.ZIP_DEFLATED)
    archive_src_base_path = os.path.join(SCRIPT_PATH, 'builds', 'install')
    archive_dst_base_path = 'discord-rpc'
    with cd(archive_src_base_path):
        for path, subdirs, filenames in os.walk('.'):
            for fname in filenames:
                fpath = os.path.join(path, fname)
                archive_file.write(fpath, os.path.normpath(os.path.join(archive_dst_base_path, fpath)))


@click.command()
@click.option('--clean', is_flag=True)
def main(clean):
    os.chdir(SCRIPT_PATH)

    if clean:
        shutil.rmtree('builds', ignore_errors=True)

    mkdir_p('builds')

    if sys.platform.startswith('win'):
        generator32 = 'Visual Studio 14 2015'
        generator64 = 'Visual Studio 14 2015 Win64'

        build_lib('win32-static', generator32, {})
        build_lib('win32-dynamic', generator32, {'BUILD_SHARED_LIBS': True, 'USE_STATIC_CRT': True})
        build_lib('win64-static', generator64, {})
        build_lib('win64-dynamic', generator64, {'BUILD_SHARED_LIBS': True, 'USE_STATIC_CRT': True})

        # todo: this in some better way
        src_dll = os.path.join(SCRIPT_PATH, 'builds', 'win64-dynamic', 'src', 'Release', 'discord-rpc.dll')
        dst_dll = os.path.join(SCRIPT_PATH, 'examples', 'button-clicker', 'Assets', 'Resources', 'discord-rpc.dll')
        shutil.copy(src_dll, dst_dll)
        dst_dll = os.path.join(SCRIPT_PATH, 'examples', 'unrealstatus', 'Plugins', 'discordrpc', 'Binaries', 'ThirdParty', 'discordrpcLibrary', 'Win64', 'discord-rpc.dll')
        shutil.copy(src_dll, dst_dll)
    elif sys.platform == 'darwin':
        build_lib('osx-static', None, {})
        build_lib('osx-dynamic', None, {'BUILD_SHARED_LIBS': True})

    create_archive()


if __name__ == '__main__':
    sys.exit(main())
