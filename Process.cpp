#include <iostream>
#include<vector>
#include<set>
#include <random>
#include<map>

using namespace std;

int RAM_size=4000;

struct process{
int id;
int quanta;
int process_size;
bool child_exists;

process(int x,int y)
{
  id=x;
  quanta=y;
  process_size=20 + ( rand() % ( 40 - 20 + 1 ) );
  child_exists=false;
}

void printdetails(){
  cout<<"The id of the process is :"<<id<<" quanta is :"<<quanta<<" and its size is : "<<process_size<<endl;
}

};
map<int,process>mapper;

void initialize(){

  
}




int main() {
  
 // process example(1,20);
  //example.printdetails();

  
}