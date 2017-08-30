import os
import subprocess
import sys
import shutil
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


def build(build_path, generator, options):
    mkdir_p(build_path)
    with cd(build_path):
        initial_cmake = ['cmake', SCRIPT_PATH]
        if generator:
            initial_cmake.extend(['-G', generator])
        for key in options:
            val = 'ON' if options[key] else 'OFF'
            initial_cmake.append('-D%s=%s' %(key, val))
        subprocess.check_call(initial_cmake)
        subprocess.check_call(['cmake', '--build', '.', '--config', 'Debug'])
        subprocess.check_call(['cmake', '--build', '.', '--config', 'Release'])


def main():
    os.chdir(SCRIPT_PATH)
    if sys.platform.startswith('win'):
        generator = 'Visual Studio 14 2015'
        build(os.path.join(SCRIPT_PATH, 'builds', 'win32-static'), generator, {})
        build(os.path.join(SCRIPT_PATH, 'builds', 'win32-dynamic'), generator, {'BUILD_DYNAMIC_LIB': True})
        generator = 'Visual Studio 14 2015 Win64'
        build(os.path.join(SCRIPT_PATH, 'builds', 'win64-static'), generator, {})
        build(os.path.join(SCRIPT_PATH, 'builds', 'win64-dynamic'), generator, {'BUILD_DYNAMIC_LIB': True})
        # todo: this in some better way
        src_dll = os.path.join(SCRIPT_PATH, 'builds', 'win64-dynamic', 'src', 'Release', 'discord-rpc.dll')
        dst_dll = os.path.join(SCRIPT_PATH, 'examples\\button-clicker\\Assets\\Resources\\discord-rpc.dll')
        shutil.copy(src_dll, dst_dll)
        dst_dll = os.path.join(SCRIPT_PATH, 'examples\\unrealstatus\\Plugins\\discordrpc\\Binaries\\ThirdParty\\discordrpcLibrary\\Win64\\discord-rpc.dll')
        shutil.copy(src_dll, dst_dll)


if __name__ == '__main__':
    sys.exit(main())
