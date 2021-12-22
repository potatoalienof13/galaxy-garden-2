# galaxy-garden-2

An improved version of galaxy-garden that uses opengl instead of opencv.
This has a dependency on glfw3, tclap, and opencv.

See `gg2 --help` for a basic idea of how to use it.

## Examples
gg2 -e 'angle' -I pics/angles.png -w 1920x1080 -r 121
!(pics/angles.png)

gg2 -e '(cos((gl_FragCoord.xy+point)/200)+vec2(1,1)).x * (sin((gl_FragCoord.xy+point)/200)+vec2(1,1)).y + psin(dist/20+time)*0.05' -s 0.001 -a 2 -r 0.1 -R 1 -m 1x1 -n 5 -I long_command.png -w 1920x1080
!(pics/long_command.png)

gg2 -v 'sin(dist/10)' -e dist -I pics/sort_not_rank.png -w 1920x1080
!(pics/sort_not_rank.png)

gg2 -e 'sin(dist/10*cos(dist/102))' -I pics/testing.png -w 1920x1080
!(pics/testing.png)

gg2 -e '20*(sin(dist/10)+1)+angle*dist' -w 1920x1080 -I pics/thing.png
!(pics/thing.png)

gg2 -p 'fg = fg + 100*sin(fg);' -s 0.001 -a 3 -n 20 -e 'dist' -y none -g 9x9 -I pics/prerun_example.png -w 1920x1080
!(pics/prerun_example.png)

gg2 -p 'fg = fg - 2*mod(fg,20);' -s 0.001 -a 3 -n 20 -e 'dist' -y none -I pics/prerun_2.png -w 1920x1080
!(pics/prerun_2.png)

