project('gtest-mpi-listener', ['c', 'cpp'],
  license: 'BSD-3-Clause',
)

null_dep = dependency('', required : false)
if get_option('MPI_WRAPPER')
  mpi_dep = null_dep
else
  mpi_dep = dependency('mpi', language: 'cpp', required: true)
endif

gtest_mpi_listener_dep = declare_dependency(
  sources: files([
    'include/gtest-mpi-listener.hpp',
    ]),
  include_directories: 'include',
  dependencies: [mpi_dep],
)
