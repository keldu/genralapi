#!/usr/bin/env python3

#Local
from thirdparty import methods


def PhonyTargets(env = None, **kw):
    if not env: env = DefaultEnvironment()
    for target, action in kw.items():
        env.AlwaysBuild(env.Alias(target, [], action))

env = Environment(parse_flags='-I#')
#env=Environment(parse_flags='-I#thirdparty -I#protocol',CXXFLAGS=['-std=c++17','-Wall','-Wextra','-Werror'],LIBS=[''])

env.__class__.add_source_files = methods.add_source_files
env.__class__.add_library = methods.add_library

Export('env')

SConscript('protocol/SConscript')

SConscript('source/SConscript')
SConscript('util/SConscript')

env.Install('/usr/local/lib', [env.core_static_lib, env.core_shared_lib, env.util_static_lib, env.util_shared_lib])
env.Install('/usr/local/include/genral', [env.core_header, env.util_header])
env.Alias('install','/usr/local')

Default(env.core_static_lib, env.core_shared_lib)
