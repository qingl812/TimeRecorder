#!/bin/bash

# 项目所在文件夹和生成的项目名称
project_dir=(
    "TimeRecorder-Core"
    "TimeRecorder-Qt"
)
project_exe=(
    "TimeRecorderCore.exe"
    "TimeRecorderQt.exe"
)

# 打印致命错误并退出脚本 红色
info_fatal() {
    printf "\033[1;31m""%s""\033[0m\n" "$*"
    exit 0
}
# 打印提示信息 紫色
info_normal() {
    printf "\033[1;35m""%s""\033[0m\n" "$*"
}
# 打印成功信息 绿色
info_success() {
    printf "\033[1;32m""%s""\033[0m\n" "$*"
}
# 打印失败信息 黄色
info_failed() {
    printf "\033[1;33m""%s""\033[0m\n" "$*"
}

# 如果参数为空则不执行 如果有一个参数 n 则执行第 n 个项目
if [ $# -eq 0 ];then
    run_num=2
elif [ $# -eq 1 ];then
    if ! [[ $1 =~ ^[0-9]*$ ]] ; then
        info_fatal "Only accepts positive integer as parameter"
    elif [ $1 -lt 1 ] || [ $1 -gt ${#project_dir[@]} ];then
        info_fatal "The parameter is too large or too small"
    else
        run_num=$1
    fi
else
    info_fatal "takes only one or zero parameter"
fi

# 向数组 error 中添加一个警告信息
add_error() {
    let index=${#error[@]}+1
    error[index]="$1"
}
# 打印所有警告信息
printf_error() {
    for i in $(seq 0 ${#error[@]})
    do
        info_failed "${error[${i}]}"
    done
}
# 第一个参数项目文件夹名，第二个参数构建出的程序名
build_project() {
    info_normal "building $1..."
    if [ ! -d "build" ];then
      mkdir build
    fi
    if [ ! -d "build/$1" ];then
      mkdir build/$1
    fi
    cd build/$1
    cmake ../../$1 -G "MinGW Makefiles"
    if [[ "$?" -ne 0 ]];then
        add_error "$1: cmake failed"
        cd ../../
        return 0
    fi
    make
    if [[ "$?" -ne 0 ]];then
        add_error "$1: make failed"
    fi
    cd ../../
    # 复制项目文件到 bin 目录
    if [ ! -d "bin" ];then
    mkdir bin
    fi
    cp "build/$1/$2" "bin"
}

# 构建所有项目
build_project ${project_dir[0]} ${project_exe[0]}
build_project ${project_dir[1]} ${project_exe[1]}
if [[ ${#error[@]} -eq 0 ]];then
    info_success "build all success"
else
    printf_error
    exit 0
fi

# 执行要运行的 project
let temp=${run_num}-1
./bin/${project_exe[${temp}]}
