#include <bits/stdc++.h>
#include "header.h"

using namespace std;

struct process{
int id;
int quanta;
int process_size;
bool child_exists;

process(int x,int y)
{
  this->id=x;
  this->quanta=y;
  this->process_size=20 + ( rand() % ( 40 - 20 + 1 ) );
  this->child_exists=false;
}

void printdetails(){
  cout<<"The id of the process is :"<<id<<" quanta is :"<<quanta<<" and its size is : "<<process_size<<endl;
}

};

void initialize(map<int,process*>& mapper){
  ifstream inFile;
  inFile.open("processinfo.txt");
  int procid,time;
  while (true)
  {
    
    inFile >> procid>>time;
    
    if (inFile.eof())
      break;
    
    process *newprocess=new process(procid,time);
    memorysize=memorysize-newprocess->process_size;
    
    mapper[procid]=newprocess;
  }
  inFile.close();
  
}

int main() {
  map<int,process*>mapper;
  initialize(mapper);
  for(auto &x : mapper)
    {
      x.second->printdetails();
    }
  cout<<memorysize<<endl;
  
}
