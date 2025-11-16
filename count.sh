


clear

printf " lines\t chars\n"

find . -name '*.c' | xargs wc -l -m | sort -n
find . -name '*.h' | xargs wc -l -m | sort -n
find . -name '*.glsl' | xargs wc -l -m | sort -n

find . -name '*.c' -or -name '*.h' -or -name '*.glsl' | xargs wc -l -m | sort -n
