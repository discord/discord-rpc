#!/usr/bin/env python

import os
import subprocess
import sys
import shutil
import zipfile
from contextlib import contextmanager
import click


SCRIPT_PATH = os.path.dirname(os.path.abspath(__file__))

def platform():
    """ a name for the platform """
    if sys.platform.startswith('win'):
        return 'win'
    elif sys.platform == 'darwin':
        return 'osx'
    elif sys.platform.startswith('linux'):
        return 'linux'
    raise Exception('Unsupported platform ' + sys.platform)


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
        click.secho('Making ' + path, fg='yellow')
        os.makedirs(path)


@click.group(invoke_without_command=True)
@click.pass_context
@click.option('--clean', is_flag=True)
def cli(ctx, clean):
    if ctx.invoked_subcommand is None:
        ctx.invoke(libs, clean=clean)
        ctx.invoke(archive)


@cli.command()
def unity():
    pass


@cli.command()
def unreal():
    pass


def build_lib(build_name, generator, options):
    """ Create a dir under builds, run build and install in it """
    build_path = os.path.join(SCRIPT_PATH, 'builds', build_name)
    install_path = os.path.join(SCRIPT_PATH, 'builds', 'install', build_name)
    mkdir_p(build_path)
    mkdir_p(install_path)
    with cd(build_path):
        initial_cmake = [
            'cmake',
            SCRIPT_PATH,
            '-DCMAKE_INSTALL_PREFIX=%s' % os.path.join('..', 'install', build_name)
        ]
        if generator:
            initial_cmake.extend(['-G', generator])
        for key in options:
            val = 'ON' if options[key] else 'OFF'
            initial_cmake.append('-D%s=%s' % (key, val))
        subprocess.check_call(initial_cmake)
        subprocess.check_call(['cmake', '--build', '.', '--config', 'Debug'])
        subprocess.check_call(['cmake', '--build', '.', '--config', 'Release', '--target', 'install'])


@cli.command()
def archive():
    archive_file_path = os.path.join(SCRIPT_PATH, 'builds', 'discord-rpc-%s.zip' % platform())
    archive_file = zipfile.ZipFile(archive_file_path, 'w', zipfile.ZIP_DEFLATED)
    archive_src_base_path = os.path.join(SCRIPT_PATH, 'builds', 'install')
    archive_dst_base_path = 'discord-rpc'
    with cd(archive_src_base_path):
        for path, subdirs, filenames in os.walk('.'):
            for fname in filenames:
                fpath = os.path.join(path, fname)
                archive_file.write(fpath, os.path.normpath(os.path.join(archive_dst_base_path, fpath)))


@cli.command()
@click.option('--clean', is_flag=True)
def libs(clean):
    os.chdir(SCRIPT_PATH)

    if clean:
        shutil.rmtree('builds', ignore_errors=True)

    mkdir_p('builds')

    plat = platform()

    if plat == 'win':
        generator32 = 'Visual Studio 14 2015'
        generator64 = 'Visual Studio 14 2015 Win64'
        build_lib('win32-static', generator32, {})
        build_lib('win32-dynamic', generator32, {'BUILD_SHARED_LIBS': True, 'USE_STATIC_CRT': True})
        build_lib('win64-static', generator64, {})
        build_lib('win64-dynamic', generator64, {'BUILD_SHARED_LIBS': True, 'USE_STATIC_CRT': True})
    elif plat == 'osx':
        build_lib('osx-static', None, {})
        build_lib('osx-dynamic', None, {'BUILD_SHARED_LIBS': True})
    elif plat == 'linux':
        build_lib('linux-static', None, {})
        build_lib('linux-dynamic', None, {'BUILD_SHARED_LIBS': True})


if __name__ == '__main__':
    sys.exit(cli())
