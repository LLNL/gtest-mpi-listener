project('gtest-mpi-listener', ['c', 'cpp'],
  license: 'BSD-3-Clause',
)

mpi_dep = dependency('mpi', language: 'cpp', required: true)

gtest_mpi_listener_dep = declare_dependency(
  sources: files([
    'include/gtest-mpi-listener.hpp',
    ]),
  include_directories: 'include',
  dependencies: [mpi_dep],
)
