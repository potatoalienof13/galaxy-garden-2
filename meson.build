project('simple', 'cpp')
src = ['shader.cpp', 'main.cpp', 'initialization.cpp', 'glad.cpp']
inc_dir = include_directories('include')

cppc = meson.get_compiler('cpp')
dl_dep = cppc.find_library('dl', required : true)

executable('gg2', src, include_directories : inc_dir, dependencies : [
dependency('glfw3'),
dl_dep,
])

