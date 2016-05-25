import os
from glob import glob
#import ycm_core

base = os.path.dirname(os.path.abspath(__file__))

def FlagsForFile(filename, **kwargs):
	filedir = os.path.dirname(filename)
	flags = [
		'-x', 'c++',
		'-Wall',
		'-Wextra',
		'-Wpedantic',
		'-fPIC',
		'-std=c++14',
		'-isystem', os.path.join(os.environ['HOME'], '.vim/bundle/YouCompleteMe/third_party/ycmd/ycmd/../clang_includes'),
		'-isystem', '/usr/include/eigen3',
		'-I', '.',
	]

	for path in [os.path.join(base, 'include'), os.path.join(base, 'include', 'yaskawa-ethernet')]:
		flags += ['-I', path]

	return {
		'flags': flags,
		'do_cache': True
	}
