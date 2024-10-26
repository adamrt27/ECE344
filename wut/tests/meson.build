tests = [
  'main-thread-is-0',
  'first-thread-is-1',
  'main-thread-yields',
  'first-thread-exits-explicitly',
  'first-thread-exits-implicitly',
  'first-thread-runs',
  'main-thread-joins',
  'first-thread-cancelled',
  'thread-in-thread',
  'two-threads',
  'reuse-thread-0',
  'error-thread-join-self',
  'error-thread-yield-none',
  'rishabh2050',
  'rosiepie-1',
  'rosiepie-2',
  'lots-of-threads',
  'even-more-threads',
  'fifo-order',
  'student-a',
  'join-cancelled-thread',
]

foreach test : tests
  source = files(['main.c', '@0@.c'.format(test)])
  exe = executable(
    test, source,
    include_directories : inc,
    link_with : [wut]
  )
  test('@0@'.format(test), exe)
endforeach
