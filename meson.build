project('simple', 'cpp')
src = ['shader.cpp', 'main.cpp', 'initialization.cpp', 'glad.cpp']
inc_dir = include_directories('include')
glfw3_dep = dependency('glfw3')
dl_dep = dependency('dl')
thread_dep = dependency('threads')
#glib_dep = dependency('libc')
executable('gg2', src, include_directories : inc_dir, dependencies : [glfw3_dep, thread_dep, dl_dep])

