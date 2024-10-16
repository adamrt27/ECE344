project(
  'wut',
  'c',
  default_options : ['c_std=c17', 'warning_level=3'],
)
add_global_arguments('-D_DEFAULT_SOURCE', language : 'c')

inc = include_directories('include')

subdir('include')
subdir('src')

wut = shared_library(
  'wut',
  wut_sources,
  include_directories : inc,
)

subdir('test')
subdir('tests')
