#include <bits/stdc++.h>
#include "header.h"

using namespace std;


queue<int>ready;
queue<int>running;
queue<int>blocked;

vector<string>states={"Ready","Running","Blocked"};

struct memchunk{//structure for keeping track of the free chunks inside the memory.
int size;
memchunk* ptr;

memchunk(){
  this->size=0;
  ptr=NULL;
}

memchunk(int x){
  this->size=x;
  ptr=NULL;
}
};


//the global file table which actually points to each inode number.
map<int,int>global_file_table;





struct Node
{
int node;
int time;
Node* left;
Node* right;
Node()
{
  node=0;
  time=0;
  left=NULL;
  right=NULL;
}

Node(int x,int times)
{
  node=x;
  time=times;
  left=NULL;
  right=NULL;
}

};

Node* process_tree(vector<vector<int>>nums,int left,int right)
{
if(left<=right)
{
  int mid=left+(right-left)/2;
  Node* newnode=new Node(nums[mid][1],nums[mid][0]);
  newnode->left=process_tree(nums,left,mid-1);
  newnode->right=process_tree(nums,mid+1,right);
  return newnode; 
}
  return NULL;
}

Node* insert_process(Node* node,int id,int times)
{
  if(node==NULL)
  {
    node = new Node(id,times);
  }
  else if(times<=node->time)
  {
    node->left=insert_process(node->left,id,times);
  }
  else
  {
    node->right=insert_process(node->right,id,times);
  }
  return node;
};

void view_process_tree(Node* node)
{
  if(!node)
    return;

  view_process_tree(node->left);
  
  cout<<"The process id is : "<<node->node<<" and the time for which it is running is : "<<node->time<<endl;
  
  view_process_tree(node->right);
}

struct process{
//the process id,identifier
int id;
//the amount of time for which it wants to run.
int quanta;

//process State
string process_state;

//stacksize
int stacksize;

//heapsize
int heapsize;

//total memory consumed by the process
int process_size;

//the total free size this process has. generally, it is equal to the total size of this process minus the stack and heap size
int freesize;

//will tell you if a child of this process exists,otherwise its a parent process
bool child_exists;

//this will keep track of the free memory in chunk size of one each.
memchunk* nextchunk;


//will contain the list of the files being used by this particular process. this will further point to the global open file table.
map<int,string>local_file_table ;

//copy of the process,option.
//pointers to other related processes(Parent processes)
//scheduling of processes on basis of red black tree.
//add some space for the OS code as well
//add page table here as well.PCB
//per process open file table add here.

process(int x,int y)
{
  this->id=x;
  this->quanta=y;
  this->process_size=20 + ( rand() % ( 40 - 20 + 1 ) );
  this->stacksize=0+( rand() % ( (this->process_size)/5 - 0 + 1 ) );
  this->heapsize=0+( rand() % ( (this->process_size)/5 - 0 + 1 ) );
  this->freesize=this->process_size-( this->stacksize+this->heapsize);
  this->child_exists=false;//by default,we assume that the process doesnt have any child as such.
  this->process_state=states[rand()%3];

  //opening the basic three files.
  local_file_table[0]="STDIN";
  local_file_table[1]="STDOUT";
  local_file_table[2]="STDERR";

  
int number=this->freesize;
memchunk* original;
  nextchunk=new memchunk(1);
  original=nextchunk;
while(number--) //Free memory mapping using linked list.
  {
   nextchunk-> ptr= new memchunk(1);
    nextchunk=nextchunk->ptr;   
  }
  nextchunk = original;

}

void printdetails(){
  cout<<"The process id is : "<<this->id<<endl;
  cout<<"The total size of the process is : "<<this->process_size<<endl;
  cout<<"The total time elapse by this process is : "<<this->quanta<<endl;
  cout<<"The size of the stack is : "<<this->stacksize<<endl;
  cout<<"The size of the heap is : "<<this->heapsize<<endl;
  cout<<"The total free memory inside is : "<<this->freesize<<endl;
  cout<<"The state of the process is : "<<this->process_state<<endl;
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
      
      if(x.second->process_state=="Ready")
      ready.push(x.first);

      if(x.second->process_state=="Running")
      running.push(x.first);

      if(x.second->process_state=="Blocked")
      blocked.push(x.first);
        
      cout<<"First free chunk is : "<<x.second->nextchunk->size<<endl;
    }
  //cout<<memorysize<<endl;
    cout<<"\n"<<"\n";
  
  vector<vector<int>>nums;
  
  for(auto &x:mapper)
    {
      vector<int>arr;
      arr.push_back(x.second->quanta);
      arr.push_back(x.first);
      nums.push_back(arr);
    }
  sort(nums.begin(),nums.end());

  Node* ProcessTree=process_tree(nums, 0, nums.size()-1);
  //view_process_tree(ProcessTree);

int number;
  while(1)
    {
      cout<<"Press 1 if you want to add a new process"<<endl;
      cout<<"Press 2 if you want to see all the processes along with the time quanta for which it is running"<<endl;
      cout<<"Press 3 if you want to fork a process and create its child"<<endl;
      cout<<"Press 4 if you want to see the memory image of a running process"<<endl;
      cout<<"Press 5 if you want to see the list of open files by a process"<<endl;
      
      cin>>number;
printf("\n\n");
      if(number==1)
      {
        int ID;
        int TIME;
        cout<<"Please enter the id of the process which you want to create : "<<endl;
        cin>>ID;
        cout<<"Please enter the time quanta for which u want to run"<<endl;
        cin>>TIME;
        if(mapper.count(number))
        {
          cout<<"Sorry, this process already exists"<<endl;         
        }
        else
        {
          process *newproc=new process(ID,TIME);
          mapper[ID]=newproc;
          ProcessTree=insert_process(ProcessTree,ID,TIME);
        }
      }
      else if(number==2)
      {
        view_process_tree(ProcessTree);
      }
      else if(number==3)
      {
        int ID;
        cout<<"Please input the ID of the process of which you want to create fork"<<endl;
        cin>>ID;
        mapper[ID]->child_exists=true;
        memorysize=memorysize-mapper[ID]->process_size; 
        //since a child creates a separate memory image of its own.
      }
      else if(number==4)
      {
        int ids;
        cout<<"Please enter the ID of the process for which  u want to see the memory image"<<endl;
        cin>>ids;
        mapper[ids]->printdetails();
      }
      else
      {
        int ids;
        cout<<"Please enter the ID of the process for which  u want to see the memory image"<<endl;
        cin>>ids;
        for(auto x:mapper[ids]->local_file_table)
          {
            cout<<x.second<<endl;
          }
      }
    }
}
