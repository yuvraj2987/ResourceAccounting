from distutils.core import setup, Extension

module1 = Extension('deviceModule',
                        sources = ['deviceModule.c'])

setup (name = 'PackageName',
           version = '1.0',
                  description = 'This is a deviceModule package',
                         ext_modules = [module1])
