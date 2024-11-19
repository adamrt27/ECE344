project(
  'hello-ext2',
  'c',
  default_options : ['c_std=c17', 'warning_level=3'],
)
add_global_arguments('-D_DEFAULT_SOURCE', language : 'c')

ext2_create_exe = executable(
  'ext2-create',
  'src/ext2-create.c',
)
