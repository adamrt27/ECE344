project(
  'pht',
  'c',
  default_options : ['c_std=gnu17', 'warning_level=2'],
)
add_global_arguments('-D_DEFAULT_SOURCE', language : 'c')

subdir('src')

thread_dep = dependency('threads')
executable('pht-tester', pht_tester_sources, dependencies : thread_dep)
