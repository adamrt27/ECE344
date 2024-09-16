sleep_ms = library('sleep_ms', 'sleep_ms.c')

tests = [
  'arg0-ownership',
  'dev-null',
  'echo-cat-cat',
  'send-signal',
  'sigpipe',
  'single-cat',
  'single-sleep',
  'ssp-wait',
  'subreaper-child',
  'subreaper-grandchild',
  'true-false',
]

foreach test : tests
  c_source = files(['@0@.c'.format(test)])
  py_source = files(['@0@.py'.format(test)])
  exe = executable(
    test, c_source,
    include_directories : inc,
    link_with : [sleep_ms, ssp]
  )
  test(
    'tests/@0@'.format(test),
    py_source,
    args : [exe],
  )
endforeach
