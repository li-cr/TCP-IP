#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <iostream>
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[])
{
    if(argc <= 2) return 0;
    char buf[10];
    symlink(argv[1], argv[2]);
    buf[readlink(argv[2], buf, 10)] = '\0';
    cout<<buf<<endl;
    return 0;
}