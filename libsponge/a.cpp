#include <iostream>
#include "byte_stream.hh"
using namespace std;

ByteStream tmp(10);

void show()
{
    cout<<"input_ended: "<<tmp.input_ended()<<endl;    
    cout<<"buffer_empty: "<<tmp.buffer_empty()<<endl;
    cout<<"eof: "<<tmp.eof()<<endl;
    cout<<"bytes_read: "<<tmp.bytes_read()<<endl;
    cout<<"buffer_size: "<<tmp.buffer_size()<<endl;
    cout<<"bytes_write: "<<tmp.bytes_written()<<endl;
    cout<<"remainint_capacity: "<<tmp.remaining_capacity()<<endl;
    cout<<endl;
}
int main()
{
    tmp.write("abc");
    show();
    cout<<tmp.read(1)<<endl;
    show();
    cout<<tmp.read(3)<<endl;
    return 0;
}