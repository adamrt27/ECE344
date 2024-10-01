tests = [
  'copy-1',
  'copy-2',
  'copy-3',
  'copy-4',
  'copy-5',
  'cow-1',
  'cow-2',
  'cow-3',
  'cow-4',
  'cow-5',
  'cow-6',
  'cow-7',
  'cow-8',
  'cow-9',
]

foreach test : tests
  source = files(['main.c', '@0@.c'.format(test)])
  exe = executable(
    test, source,
    include_directories : inc,
    link_with : [vms_lib]
  )
  test('@0@'.format(test), exe)
endforeach
