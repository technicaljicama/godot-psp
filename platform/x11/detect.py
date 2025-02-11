
import os
import sys
import platform


def is_active():
    return True


def get_name():
    return "X11"


def can_build():

    if (os.name != "posix"):
        return False

    if sys.platform == "darwin":
        return False  # no x11 on mac for now

    x11_error = os.system("pkg-config --version > /dev/null")
    if (x11_error):
        print("pkg-config not found.. x11 disabled.")
        return False

    x11_error = os.system("pkg-config x11 --modversion > /dev/null ")
    if (x11_error):
        print("X11 not found.. x11 disabled.")
        return False

    x11_error = os.system("pkg-config xcursor --modversion > /dev/null ")
    if (x11_error):
        print("xcursor not found.. x11 disabled.")
        return False

    x11_error = os.system("pkg-config xinerama --modversion > /dev/null ")
    if (x11_error):
        print("xinerama not found.. x11 disabled.")
        return False

    x11_error = os.system("pkg-config xrandr --modversion > /dev/null ")
    if (x11_error):
        print("xrandr not found.. x11 disabled.")
        return False

    return True  # X11 enabled


def get_opts():

    return [
        ('use_llvm', 'Use llvm compiler', 'no'),
        ('use_static_cpp', 'Link libgcc and libstdc++ statically for better portability', 'no'),
        ('use_sanitizer', 'Use llvm compiler sanitize address', 'no'),
        ('use_leak_sanitizer', 'Use llvm compiler sanitize memory leaks', 'no'),
        ('use_lto', 'Use link time optimization', 'no'),
        ('pulseaudio', 'Detect & Use pulseaudio', 'yes'),
        ('udev', 'Use udev for gamepad connection callbacks', 'no'),
        ('debug_release', 'Add debug symbols to release version', 'no'),
        ('touch', 'Enable touch events', 'yes'),
    ]


def get_flags():

    return []


def configure(env):

    is64 = sys.maxsize > 2**32

    if (env["bits"] == "default"):
        if (is64):
            env["bits"] = "64"
        else:
            env["bits"] = "32"

    env.Append(CPPPATH=['#platform/x11'])
    if (env["use_llvm"] == "yes"):
        if 'clang++' not in env['CXX']:
            env["CC"] = "clang"
            env["CXX"] = "clang++"
            env["LD"] = "clang++"
        env.Append(CPPFLAGS=['-DTYPED_METHOD_BIND'])
        env.Append(CXXFLAGS=['-std=c++11'])
        env.extra_suffix = ".llvm"
    elif (os.system("gcc --version > /dev/null 2>&1") == 0): # GCC
        # Hack to prevent building this branch with GCC 6+, which trigger segfaults due to UB when dereferencing pointers in Object::cast_to
        # This is fixed in the master branch, for 2.1 we just prevent using too recent GCC versions.
        import subprocess
        gcc_major = subprocess.check_output(['gcc', '-dumpversion']).decode('ascii').split('.')[0]
        if (int(gcc_major) > 5):
            print("Your configured compiler appears to be GCC %s, which triggers issues in release builds for this version of Godot (fixed in Godot 3.0+)." % gcc_major)
            print("You can use the Clang compiler instead with the `use_llvm=yes` option, or configure another compiler such as GCC 5 using the CC, CXX and LD flags.")
            print("Aborting..")
            sys.exit(255)

    if (env["use_sanitizer"] == "yes"):
        env.Append(CCFLAGS=['-fsanitize=address', '-fno-omit-frame-pointer'])
        env.Append(LINKFLAGS=['-fsanitize=address'])
        env.extra_suffix += "s"

    if (env["use_leak_sanitizer"] == "yes"):
        env.Append(CCFLAGS=['-fsanitize=address', '-fno-omit-frame-pointer'])
        env.Append(LINKFLAGS=['-fsanitize=address'])
        env.extra_suffix += "s"

    # if (env["tools"]=="no"):
    #	#no tools suffix
    #	env['OBJSUFFIX'] = ".nt"+env['OBJSUFFIX']
    #	env['LIBSUFFIX'] = ".nt"+env['LIBSUFFIX']

    if (env["use_lto"] == "yes"):
        env.Append(CCFLAGS=['-flto'])
        env.Append(LINKFLAGS=['-flto'])


    env.Append(CCFLAGS=['-pipe'])
    env.Append(LINKFLAGS=['-pipe'])

    if (env["target"] == "release"):
        env.Prepend(CCFLAGS=['-Ofast'])
        if (env["debug_release"] == "yes"):
            env.Prepend(CCFLAGS=['-g2'])

    elif (env["target"] == "release_debug"):

        env.Prepend(CCFLAGS=['-O2', '-ffast-math', '-DDEBUG_ENABLED'])
        if (env["debug_release"] == "yes"):
            env.Prepend(CCFLAGS=['-g2'])

    elif (env["target"] == "debug"):

        env.Prepend(CCFLAGS=['-g2', '-DDEBUG_ENABLED', '-DDEBUG_MEMORY_ENABLED'])
        env.Append(LINKFLAGS=['-rdynamic'])

    env.ParseConfig('pkg-config x11 --cflags --libs')
    env.ParseConfig('pkg-config xinerama --cflags --libs')
    env.ParseConfig('pkg-config xcursor --cflags --libs')
    env.ParseConfig('pkg-config xrandr --cflags --libs')

    if (env['touch'] == 'yes'):
        x11_error = os.system("pkg-config xi --modversion > /dev/null ")
        if (x11_error):
            print("xi not found.. cannot build with touch. Aborting.")
            sys.exit(255)
        env.ParseConfig('pkg-config xi --cflags --libs')
        env.Append(CPPFLAGS=['-DTOUCH_ENABLED'])

    if (env['builtin_openssl'] == 'no'):
        env.ParseConfig('pkg-config openssl --cflags --libs')

    if (env['builtin_libwebp'] == 'no'):
        env.ParseConfig('pkg-config libwebp --cflags --libs')

    # freetype depends on libpng and zlib, so bundling one of them while keeping others
    # as shared libraries leads to weird issues
    if (env['builtin_freetype'] == 'yes' or env['builtin_libpng'] == 'yes' or env['builtin_zlib'] == 'yes'):
        env['builtin_freetype'] = 'yes'
        env['builtin_libpng'] = 'yes'
        env['builtin_zlib'] = 'yes'

    if (env['builtin_freetype'] == 'no'):
        env.ParseConfig('pkg-config freetype2 --cflags --libs')

    if (env['builtin_libpng'] == 'no'):
        env.ParseConfig('pkg-config libpng --cflags --libs')

    if (env['builtin_squish'] == 'no' and env["tools"] == "yes"):
        env.ParseConfig('pkg-config libsquish --cflags --libs')

    # Sound and video libraries
    # Keep the order as it triggers chained dependencies (ogg needed by others, etc.)

    if (env['builtin_libtheora'] == 'no'):
        env['builtin_libogg'] = 'no'  # Needed to link against system libtheora
        env['builtin_libvorbis'] = 'no'  # Needed to link against system libtheora
        env.ParseConfig('pkg-config theora theoradec --cflags --libs')

    if (env['builtin_libvorbis'] == 'no'):
        env['builtin_libogg'] = 'no'  # Needed to link against system libvorbis
        env.ParseConfig('pkg-config vorbis vorbisfile --cflags --libs')

    if (env['builtin_opus'] == 'no'):
        env['builtin_libogg'] = 'no'  # Needed to link against system opus
        env.ParseConfig('pkg-config opus opusfile --cflags --libs')

    if (env['builtin_libogg'] == 'no'):
        env.ParseConfig('pkg-config ogg --cflags --libs')

    env.Append(CPPFLAGS=['-DOPENGL_ENABLED'])

    if (env['builtin_glew'] == 'no'):
        env.ParseConfig('pkg-config glew --cflags --libs')

    if os.system("pkg-config --exists alsa") == 0:
        print("Enabling ALSA")
        env.Append(CPPFLAGS=["-DALSA_ENABLED"])
        env.ParseConfig('pkg-config alsa --cflags --libs')
    else:
        print("ALSA libraries not found, disabling driver")

    if (platform.system() == "Linux"):
        env.Append(CPPFLAGS=["-DJOYDEV_ENABLED"])
    if (env["udev"] == "yes"):
        # pkg-config returns 0 when the lib exists...
        found_udev = not os.system("pkg-config --exists libudev")

        if (found_udev):
            print("Enabling udev support")
            env.Append(CPPFLAGS=["-DUDEV_ENABLED"])
            env.ParseConfig('pkg-config libudev --cflags --libs')
        else:
            print("libudev development libraries not found, disabling udev support")

    if (env["pulseaudio"] == "yes"):
        if not os.system("pkg-config --exists libpulse-simple"):
            print("Enabling PulseAudio")
            env.Append(CPPFLAGS=["-DPULSEAUDIO_ENABLED"])
            env.ParseConfig('pkg-config --cflags --libs libpulse-simple')
        else:
            print("PulseAudio development libraries not found, disabling driver")

    if (env['builtin_zlib'] == 'no'):
        env.ParseConfig('pkg-config zlib --cflags --libs')

    env.Append(CPPFLAGS=['-DX11_ENABLED', '-DUNIX_ENABLED', '-DGLES2_ENABLED', '-DGLES_OVER_GL'])
    env.Append(LIBS=['SDL2','pthread'])

    if (platform.system() == "Linux"):
        env.Append(LIBS=['dl'])
    # env.Append(CPPFLAGS=['-DMPC_FIXED_POINT'])

    # host compiler is default..

    if (is64 and env["bits"] == "32"):
        env.Append(CPPFLAGS=['-m32'])
        env.Append(LINKFLAGS=['-m32', '-L/usr/lib/i386-linux-gnu'])
    elif (not is64 and env["bits"] == "64"):
        env.Append(CPPFLAGS=['-m64'])
        env.Append(LINKFLAGS=['-m64', '-L/usr/lib/i686-linux-gnu'])

    import methods

    env.Append(BUILDERS={'GLSL120': env.Builder(action=methods.build_legacygl_headers, suffix='glsl.gen.h', src_suffix='.glsl')})
    env.Append(BUILDERS={'GLSL': env.Builder(action=methods.build_glsl_headers, suffix='glsl.gen.h', src_suffix='.glsl')})
    env.Append(BUILDERS={'GLSL120GLES': env.Builder(action=methods.build_gles2_headers, suffix='glsl.gen.h', src_suffix='.glsl')})
    #env.Append( BUILDERS = { 'HLSL9' : env.Builder(action = methods.build_hlsl_dx9_headers, suffix = 'hlsl.gen.h',src_suffix = '.hlsl') } )

    # Link those statically for portability
    if (env["use_static_cpp"] == "yes"):
        env.Append(LINKFLAGS=['-static-libgcc', '-static-libstdc++'])

    list_of_x86 = ['x86_64', 'x86', 'i386', 'i586']
    if any(platform.machine() in s for s in list_of_x86):
        env["x86_libtheora_opt_gcc"] = True
