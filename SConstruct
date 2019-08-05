#!/usr/bin/env python3

#Local
from thirdparty import methods


env = Environment(parse_flags='-I# -I#protocol')
#env=Environment(parse_flags='-I#thirdparty -I#protocol',CXXFLAGS=['-std=c++17','-Wall','-Wextra','-Werror'],LIBS=[''])

env.__class__.add_source_files = methods.add_source_files
env.__class__.add_library = methods.add_library

Export('env')

SConscript('protocol/SConscript')

SConscript('source/SConscript')
SConscript('util/SConscript')
