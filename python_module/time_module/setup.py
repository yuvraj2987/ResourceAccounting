from distutils.core import setup, Extension

module1 = Extension('timeModule',
                      sources = ['timeModule.c'],
                      libraries = ['rt']
                   )

setup (name = 'PackageName',
       version = '1.0',
       description = 'This is a timeModule package',
       ext_modules = [module1])
