project(
  'vms',
  'c',
  default_options : ['c_std=c17', 'warning_level=3'],
)
add_global_arguments('-D_DEFAULT_SOURCE', language : 'c')

inc = include_directories('include')

subdir('include')
subdir('src')

vms_lib = shared_library(
  'vms',
  vms_sources,
  include_directories : inc,
)

vms_exe = executable(
  'vms',
  'src/main.c',
  include_directories : inc,
  link_with : [vms_lib]
)

# subdir('test')
subdir('tests')
