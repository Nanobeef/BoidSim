Color_Off='\033[0m'       # Text Reset

# Regular Colors
Black='\033[0;30m'        # Black
Red='\033[0;31m'          # Red
Green='\033[0;32m'        # Green
Yellow='\033[0;33m'       # Yellow
Blue='\033[0;34m'         # Blue
Purple='\033[0;35m'       # Purple
Cyan='\033[0;36m'         # Cyan
White='\033[0;37m'        # White

# Bold
BBlack='\033[1;30m'       # Black
BRed='\033[1;31m'         # Red
BGreen='\033[1;32m'       # Green
BYellow='\033[1;33m'      # Yellow
BBlue='\033[1;34m'        # Blue
BPurple='\033[1;35m'      # Purple
BCyan='\033[1;36m'        # Cyan
BWhite='\033[1;37m'       # White

# Underline
UBlack='\033[4;30m'       # Black
URed='\033[4;31m'         # Red
UGreen='\033[4;32m'       # Green
UYellow='\033[4;33m'      # Yellow
UBlue='\033[4;34m'        # Blue
UPurple='\033[4;35m'      # Purple
UCyan='\033[4;36m'        # Cyan
UWhite='\033[4;37m'       # White

# Background
On_Black='\033[40m'       # Black
On_Red='\033[41m'         # Red
On_Green='\033[42m'       # Green
On_Yellow='\033[43m'      # Yellow
On_Blue='\033[44m'        # Blue
On_Purple='\033[45m'      # Purple
On_Cyan='\033[46m'        # Cyan
On_White='\033[47m'       # White

# High Intensity
IBlack='\033[0;90m'       # Black
IRed='\033[0;91m'         # Red
IGreen='\033[0;92m'       # Green
IYellow='\033[0;93m'      # Yellow
IBlue='\033[0;94m'        # Blue
IPurple='\033[0;95m'      # Purple
ICyan='\033[0;96m'        # Cyan
IWhite='\033[0;97m'       # White

# Bold High Intensity
BIBlack='\033[1;90m'      # Black
BIRed='\033[1;91m'        # Red
BIGreen='\033[1;92m'      # Green
BIYellow='\033[1;93m'     # Yellow
BIBlue='\033[1;94m'       # Blue
BIPurple='\033[1;95m'     # Purple
BICyan='\033[1;96m'       # Cyan
BIWhite='\033[1;97m'      # White

# High Intensity backgrounds
On_IBlack='\033[0;100m'   # Black
On_IRed='\033[0;101m'     # Red
On_IGreen='\033[0;102m'   # Green
On_IYellow='\033[0;103m'  # Yellow
On_IBlue='\033[0;104m'    # Blue
On_IPurple='\033[0;105m'  # Purple
On_ICyan='\033[0;106m'    # Cyan
On_IWhite='\033[0;107m'   # White

build_mode=$1
output_mode=$2

if [[ -z "$2" ]]; then
	output_mode="single"
fi


bin_dir="bin/"
build_dir="build/"
mkdir -p $build_dir
source_dir="src/"
target_name="BoidSim"
target_binary="$target_name"


clean_stage(){
	rm -r "build/"
	mkdir -p $build_dir
}

if 	 [ "$build_mode" = "release" ]; then
	printf "${BIRed}Release${Color_Off}\n"

elif [ "$build_mode" = "clean" ]; then
	printf "${BIBlue}Clean${Color_Off}\n"
	clean_stage
	exit
else
	printf "${BIGreen}Debug${Color_Off}\n"
fi


verbose_mode=1
debug_print(){
	if [ $verbose_mode -eq 1 ]; then
		printf "$1\n"
	fi
}

up_to_date_print(){
	if [ $verbose_mode -eq 1 ]; then
		printf "${IBlack} Up to date: $1${Color_Off}\n"
	fi
}

not_to_date_print(){
	if [ $verbose_mode -eq 1 ]; then
		printf "${IPurple} $1${Color_Off}\n"
	fi
}

section_print(){
	printf "${BICyan}$1${Color_Off}\n"
}

time_print(){
	printf "$1\n"
}

get_time_us(){
	local sec nano
	read -r sec nano <<< "$(date +'%s %N')"
	echo $(( (10#$sec * 1000000) + (10#$nano / 1000) ))
}



# Build Variables

needs_to_link=0

glsl_compiler="glslc"

compiler="gcc"
cflags_common="-std=gnu99 -D_GNU_SOURCE"
cflags_debug="-D DEBUG -O0 -gdwarf -g"
cflags_release="-D RELEASE -O4 -g -gdwarf"
cflags_profile=""
#clfags_profile="-p -pg"
cflags_include="-I/usr/include/freetype2"
cflags_warnings="-Wall -Wno-unused-function -Wno-builtin-declaration-mismatch -Wno-strict-aliasing -Wno-unused-variable -Wno-nonnull -Wno-address -Wno-psabi"
cflags_machine="-march=native"

linker="gcc -fuse-ld=lld"
lflags_debug="-O0 -gdwarf -g"
lflags_release="-O4 -flto"
#lflags_profile="-p -pg"
lflags_profile=""
#lflags_machine="-march=native"
#lflags_machine="-march=x86-64"
lflags_libs="-lm -lc -lpthread -latomic -lX11 -lXext -lXpresent -lvulkan -levdev -lasound -lfreetype"

# Build Stages

shader_stage(){
	declare -a GLSL_FILES=()
	declare -a SPV_FILES=()
	GLSL_FILES=$(find $source_dir -name "*.glsl" -type f)

	local glsl_flags="-O0"

	for source_file in $GLSL_FILES; do
		file_base=$(basename "$source_file" .glsl)
		spv_file="$build_dir$file_base.spv"
		binary_file="$bin_dir$file_base.spv"

		message="$source_file -> $spv_file"
		
		if [ "$source_file" -nt "$spv_file" ]; then
			not_to_date_print "$message"
			(($glsl_compiler $glsl_flags $source_file -o $spv_file) && (cp $spv_file $binary_file))  & else up_to_date_print "$message"
		fi
	done
	wait
}

embed_stage(){

	declare -a EMBED_FILES=()
	declare -a EMBED_OFILES=()

	EMBED_FILES="$(find "$bin_dir" -name "*" -type f) "

	for binary_file in $EMBED_FILES; do
		
		file_base=$(basename "$bin_dir$binary_file")
		object_file="$build_dir${file_base%.*}.o"

		OFILES+="$object_file "

		message="$binary_file -> $object_file "

		if [ "$binary_file" -nt "$object_file" ]; then
			not_to_date_print "$message"
			objcopy \
				--input-target binary \
				--output-target elf64-x86-64 \
				--binary-architecture i386:x86-64 \
				--rename-section .data=.rodata,alloc,load,data,contents,readonly \
				"$binary_file" \
				"$object_file" &
			needs_to_link=1
		else
			up_to_date_print "$message"
		fi
	done
	wait
}
compile_stage(){

	local cflags="$cflags_common $cflags_include $cflags_warnings $cflags_machine $cflags_profile"

	if [ "$build_mode" = "release" ]; then
		cflags+=" $cflags_release"	
	else
		cflags+=" $clfags_debug"
	fi

	declare -a CFILES=()

	CFILES=$(find $src_dir -name "*.c" -type f)
	CHEADERS=$(find $src_dir -name "*.h" -type f)

	header_has_changed=0

	for header_file in $CHEADERS; do
		if [ "$header_file"  -nt "$target_binary" ]; then
			header_has_changed=1
			echo "$header_file"
		fi
	done


	for source_file in $CFILES; do
		file_base=$(basename "$source_file" .c)
		object_file="$build_dir$file_base.o"

		OFILES+="$object_file "

		message="$source_file -> $object_file"

		if [ "$source_file" -nt "$object_file" ] || [ $header_has_changed -eq 1 ]; then
			not_to_date_print "$message"
			$compiler -c $cflags "$source_file" -o "$object_file" &
			needs_to_link=1
		else
			up_to_date_print "$message"
		fi
	done
	wait
}
link_stage(){


	local lflags="$lflags_machine $lflags_libs $lflags_profile"

	if [ "$build_mode" = "release" ]; then
		lflags+=" $lflags_release"	
	else
		lflags+=" $lflags_debug"
	fi


	if [ $needs_to_link -eq 1 ] || [ ! -f "$target_binary" ]; 
	then
		not_to_date_print "${IGreen}"$target_binary
		$linker $lflags $OFILES -o $target_binary
	else
		 up_to_date_print "$target_binary"
	fi
	wait
}

time_next(){
	TIME=$(($(get_time_us)-TIME))
	time_print "$(((TIME) / 1000)) ms"
	TIME=$(get_time_us)
	section_print "$1"
}
time_first(){
	TIME=$(get_time_us)
	START_TIME=$TIME
}

time_last(){
	TIME=$(($(get_time_us)-$START_TIME))
	section_print "$1"
	time_print "$((TIME / 1000)) ms"
}

run_single_build(){

	
	time_first 
	section_print "Compiling Shaders:"
		shader_stage
	time_next "Embedding Files:"
		embed_stage
	time_next "Compiling C:"
		compile_stage
	time_next "Linking:"
		link_stage
	time_last "Done"
}

reset_build(){
	clean_stage
	OFILES=""
}

if [ "$output_mode" = "all" ]; then
	reset_build
	target_binary="$target_name-x86_64"
	cflags_machine="-march=x86-64"
	run_single_build

	reset_build
	target_binary="$target_name-x86_64-avx2"
	cflags_machine="-march=core-avx2"
	run_single_build

	reset_build
	target_binary="$target_name-x86_64-zen4-avx512"
	cflags_machine="-march=znver4"
	run_single_build
else
	run_single_build
fi







