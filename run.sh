

target_name="BoidSim"


if [ "$1" = "all" ]; then
	./"$target_name" &
	./"$target_name-x86_64" &
	./"$target_name-x86_64-avx2" &
	./"$target_name-x86_64-zen4-avx512" &
elif [ "$1" = "debug" ]; then
	gdb "$target_name" 
elif [ "$1" = "detached" ]; then
	./"$target_name" &
else
	./"$target_name"
fi





