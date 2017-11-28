#!/usr/bin/env python

import os
import subprocess
import sys
import shutil
import zipfile
from contextlib import contextmanager
import click


SCRIPT_PATH = os.path.dirname(os.path.abspath(__file__))
# we use Buildkite which sets this env variable by default
IS_BUILD_MACHINE = os.environ.get('CI', '') == 'true'


def get_platform():
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
    """ click wrapper for command line stuff """
    if ctx.invoked_subcommand is None:
        ctx.invoke(libs, clean=clean)
        ctx.invoke(archive)


@cli.command()
def unity():
    """ todo: build unity project """
    pass


@cli.command()
def unreal():
    """ todo: build unreal project """
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
        click.echo('--- Building ' + build_name)
        subprocess.check_call(initial_cmake)
        if not IS_BUILD_MACHINE:
            subprocess.check_call(['cmake', '--build', '.', '--config', 'Debug'])
        subprocess.check_call(['cmake', '--build', '.', '--config', 'Release', '--target', 'install'])


@cli.command()
def archive():
    """ create zip of install dir """
    click.echo('--- Archiving')
    archive_file_path = os.path.join(SCRIPT_PATH, 'builds', 'discord-rpc-%s.zip' % get_platform())
    archive_file = zipfile.ZipFile(archive_file_path, 'w', zipfile.ZIP_DEFLATED)
    archive_src_base_path = os.path.join(SCRIPT_PATH, 'builds', 'install')
    archive_dst_base_path = 'discord-rpc'
    with cd(archive_src_base_path):
        for path, _, filenames in os.walk('.'):
            for fname in filenames:
                fpath = os.path.join(path, fname)
                dst_path = os.path.normpath(os.path.join(archive_dst_base_path, fpath))
                click.echo('Adding ' + dst_path)
                archive_file.write(fpath, dst_path)


@cli.command()
@click.option('--tool', type=click.Path(exists=True, file_okay=True, dir_okay=False))
@click.argument('install_paths', required=True, nargs=-1)
def sign(install_paths, tool):
    """ Do code signing or within specified directory using our cert """
    platform = get_platform()
    signable_extensions = set()
    if platform == 'win':
        signable_extensions.add('.dll')
        sign_command_base = [
            tool,
            'sign',
            '/n', 'Hammer & Chisel Inc.',
            '/a',
            '/tr', 'http://timestamp.digicert.com/rfc3161',
            '/as',
            '/td', 'sha256',
            '/fd', 'sha256',
        ]
    elif platform == 'osx':
        signable_extensions.add('.dylib')
        sign_command_base = [
            tool,
            '--keychain', os.path.expanduser('~/Library/Keychains/login.keychain'),
            '-vvvv',
            '--deep',
            '--force',
            '--sign', 'Developer ID Application: Hammer & Chisel Inc. (53Q6R32WPB)',
        ]
    else:
        click.secho('Not signing things on this platform yet', fg='red')
        return

    click.echo('--- Signing')
    for install_path in install_paths:
        for fname in os.listdir(install_path):
            ext = os.path.splitext(fname)[1]
            if ext not in signable_extensions:
                continue
            fpath = os.path.join(install_path, fname)
            click.echo('Sign ' + fpath)
            sign_command = sign_command_base + [fpath]
            subprocess.check_call(sign_command)


@cli.command()
@click.option('--clean', is_flag=True)
def libs(clean):
    """ Do all the builds for this platform """
    if clean:
        shutil.rmtree('builds', ignore_errors=True)

    mkdir_p('builds')

    platform = get_platform()

    if platform == 'win':
        generator32 = 'Visual Studio 14 2015'
        generator64 = 'Visual Studio 14 2015 Win64'
        static_options = {}
        dynamic_options = {
            'BUILD_SHARED_LIBS': True,
            'USE_STATIC_CRT': True,
            'SIGN_BUILD': IS_BUILD_MACHINE
        }
        build_lib('win32-static', generator32, static_options)
        build_lib('win32-dynamic', generator32, dynamic_options)
        build_lib('win64-static', generator64, static_options)
        build_lib('win64-dynamic', generator64, dynamic_options)
    elif platform == 'osx':
        build_lib('osx-static', None, {})
        build_lib('osx-dynamic', None, {'BUILD_SHARED_LIBS': True, 'SIGN_BUILD': IS_BUILD_MACHINE})
    elif platform == 'linux':
        build_lib('linux-static', None, {})
        build_lib('linux-dynamic', None, {'BUILD_SHARED_LIBS': True})


if __name__ == '__main__':
    os.chdir(SCRIPT_PATH)
    sys.exit(cli())
