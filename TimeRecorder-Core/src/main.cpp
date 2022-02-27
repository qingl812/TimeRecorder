#include "tr_core.h"

#include <iostream>
#include <sstream>
#include <string>

int main(int, char**) {
    system("chcp 65001");
    tr::Core core;
    return core.exec();
}
