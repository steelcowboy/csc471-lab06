def FlagsForFile( filename, **kwargs ):
  return {
    'flags': [ '-I', '/home/steelcowboy/programming/csc471/lab06/ext',
        '-I', '/home/steelcowboy/programming/csc471/lab06/ext/glad/include',
        '-std=c++0x', '-Wall',  '-pedantic' ],
  }
