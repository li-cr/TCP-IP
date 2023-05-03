#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <iostream>
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[])
{
    int fd = open("b.cpp", O_WRONLY);
    write(fd, "\0", 1);
    return 0;
}