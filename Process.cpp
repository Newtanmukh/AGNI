#include <bits/stdc++.h>
#include "header.h"
#include <chrono>
#include <mutex>
#include <thread>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include<stdio.h>

using namespace std;

int pages=0;//keeps counter of the number of the physical pages in the real memory.

int privilege_bit=0; //It means that by default, the CPU is currently now at the User mode(less privileged mode). in case of making a system call, it will go to a higher system call and this bit will be set to 1.

//defining the kernel stack for system call simulation.
class kernel_stack
{
	//this will store the old PC, registers and other contexts
	public:
	int PC;
	int reg1;
	int reg2;
	int stack_ptr;
	kernel_stack(int x,int sp,int y,int z)
	{		
		this->PC=x;
		this->stack_ptr=sp;
		this->reg1=y;
		this->reg2=z;
	}
};

queue<int>ready;
queue<int>running;
queue<int>blocked;

map<string,int>interrupt_descriptor_table;

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
map<string,int>global_file_table;


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

//Thread structure. each thread has different programcounter,stackpointer as well as stacksize.
struct threads{
public:
	int pc;
	int SP;
	int stacksize;
	
	//registers for this thread,apart from PC and SP.
	int reg1;
	int reg2;
	
	threads(int x,int y,int z)
	{
	this->pc=x;
	this->SP=y;
	this->stacksize=z;
	}
	
	void printdetails()
	{
	
	cout<<"The program counter for this thread is : "<<this->pc<<endl;
	cout<<"The stack pointer for this thread is : "<<this->SP<<endl;
	cout<<"The stacksize for this thread is : "<<this->stacksize<<endl;
	
	}
};

//this function is used to see the list of blocked/running/ready processes.
void showq(queue<int> gq)
{
    queue<int> g = gq;
    while (!g.empty()) {
        cout << '\t' << g.front();
        g.pop();
    }
    cout << '\n';
}

//Create the balanced binary search tree.
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

//defining the MLFQ.
vector<vector<int>>mlfq;


void simulate_caching()
{
	//Simulate it for some time.
	return;
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

//Program Counter
int program_Counter;

//Stack Pointer;
int stack_pointer;

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
//scheduling of processes on basis of BST.
//add some space for the OS code as well.
//add page table here as well.PCB.
//per process open file table add here.

//this page table will store the VA-PA mappings.
map<int,int>page_table;

//to store the context and other info related to the thread.
map<int,threads*>threading_info;

//Every process thinks that OS code (logic for system calls etc. is like a part of its own. and it will itself execute it.)
vector<string>os_code;

int os_mem;

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
  
  //the Program Counter(PC) will be pointing to somewhere in between the memory where the code has been allocated. since we know the structure of the address space, it is easy to calculate the the memory that has been allocated for the code and static/global variables.
  this->program_Counter=0+rand()%(this->process_size - this->freesize - this->stacksize - this->heapsize+1);
  
  //stack grows upwards. heap grows downwards. stack pointer (SP) will be pointing to somewhere in between the memory where the stacksize is allocated.
  this->stack_pointer=this->freesize-this->stacksize+rand()%(this->stacksize+1);
  
  this->os_mem=this->freesize/3;
  
  this->os_code.push_back("Open() system call for opening a file.");
  this->os_code.push_back("brk() system call for allocating more memory to the heap.");
  this->os_code.push_back("Open() system call for opening a file.");
  this->os_code.push_back("fsync() system call will push all writes to the disk.");
  this->os_code.push_back("lseek() system call for going to a random location in a file.");
  
  //opening the basic three files.
  this->local_file_table[0]="STDIN";
  this->local_file_table[1]="STDOUT";
  this->local_file_table[2]="STDERR";
  
  int num_threads=rand()%4+1;
  
  for(int i=0;i<num_threads;i++)
  {
  	threads *new_thread=new threads(i*(this->process_size)/num_threads,i*(this->stacksize)/num_threads,this->stacksize/num_threads);
	threading_info[i]=new_thread;
  }

  
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

  //Filling up the page table of this process. Assuming that it will start from 0 and that each bpage in the physical memory will of one unit is good.

  for(int i=0;i<this->process_size;i++)
    {
      page_table[i]=pages;
      pages=pages+1;
    }

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
  
  interrupt_descriptor_table["Illegal Memory Address"]=200+(rand()%2000);
  interrupt_descriptor_table["Network packet has arrived"]=200+(rand()%2000);
  interrupt_descriptor_table["Open a file"]=200+(rand()%2000);
  interrupt_descriptor_table["Write to a file"]=200+(rand()%2000);
  interrupt_descriptor_table["Close a file"]=200+(rand()%2000);
  interrupt_descriptor_table["Go to random location in an opened file."]=200+(rand()%2000);

  //Putting the IDS of the processes into respective 
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
  //create the process tree using BST.
  Node* ProcessTree=process_tree(nums, 0, nums.size()-1);
  
  //opening the global file table entries. the local file table entries point to the global file table, which in turn point to the in-memory inode of the actual file.
global_file_table["STDIN"]=1;
global_file_table["STDOUT"]=2;
global_file_table["STDERR"]=3;  
  

  for(auto p:mapper)
    {
      if(p.second->process_state=="Ready")
        ready.push(p.first);
      else if(p.second->process_state=="Running")
        running.push(p.first);
      else
        blocked.push(p.first);
    }


int number;
  while(1)
    {
      cout<<"Press 1 if you want to add a new process"<<endl;
      cout<<"Press 2 if you want to see all the processes along with the time quanta for which it is running"<<endl;
      cout<<"Press 3 if you want to fork a process and create its child"<<endl;
      cout<<"Press 4 if you want to see the memory image of a running process"<<endl;
      cout<<"Press 5 if you want to see the list of open files by a process"<<endl;
      cout<<"Press 6 if you want to see the ID's of the running,ready and blocked processes respectively"<<endl;    
      cout<<"Press 7 if you want to QUIT."<<endl;
      cout<<"Press 8 if you want to see the inode number of all files that are open globally."<<endl;
      cout<<"Press 9 if you want to open a new file in a process"<<endl;
      cout<<"Press 10 if you want to exit a process"<<endl;
      cout<<"Press 11 if you want to implement MLFQ as well as see thr ID's of the processes inside."<<endl;
      cout<<"Press 12 if you want to see the recent VA-PA mappings"<<endl;
      cout<<"Press 13 if you want to see the thread block related info in a process."<<endl;
      cout<<"Press 14 if you want to see the total available free memory."<<endl;
      cout<<"Press 15 if you want to see the current program counter(PC) or stack Pointer(SP) of a process."<<endl;
      cout<<"Press 16 if you want to call a function(Function call) in a process."<<endl;
      cout<<"press 17 if you want to call a system call for a process "<<endl;
      cout<<"press 18 if you want to see the location of all the kernel functions. "<<endl;
      cout<<"Press 19 if you want to do context switch for a process."<<endl;
      cout<<"Press 21 if you wish to allocate memory for OS using buddy allocation scheme"<<endl;
      cout<<"Press 22 if you wish to see simulate Caching with Translational Lookaside Buffer."<<endl;
      
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
          insert_process(ProcessTree,ID,TIME);
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
        int allocate=mapper[ID]->process_size;
        //NOTE
        //Though fork creates another copy independently, but still here we for the sake for simplifying the calculation assume that the memory of this process itself gets doubled.
        
        for(int i=allocate;i<2*allocate;i++)
          {
          mapper[ID]->page_table[i]=pages;
          pages=pages+1;
          }
        
        mapper[ID]->process_size=2*mapper[ID]->process_size;
        
        memorysize=memorysize-2*mapper[ID]->process_size; 
        //since a child creates a separate memory image of its own.
      }
      else if(number==4)
      {
        int ids;
        cout<<"Please enter the ID of the process for which  u want to see the memory image"<<endl;
        cin>>ids;
        mapper[ids]->printdetails();
      }
      else if(number==5)
      {
        int ids;
        cout<<"Please enter the ID of the process for which  u want to see the memory image"<<endl;
        cin>>ids;
        for(auto x:mapper[ids]->local_file_table)
          {
            cout<<x.second<<endl;
          }
      }
      else if(number==6)
      {
        cout<<"The ID's of the Ready,Running and blocked processes are as follows : "<<endl;
        
      showq(ready);
      showq(running);
      showq(blocked);      
        
      }
      else if(number==7)
      {
        return 0;
      }
      else if(number==8)
      {
        cout<<"The inode number of the files that are open flobally are "<<endl;
        for(auto x:global_file_table)
          {
            cout<<x.second<<endl;
          }
      }
      else if(number==9)
      {
        int id;
        cout<<"Please enter the ID of the process which wants to open a new file "<<endl;
        cin>>id;

        if(mapper.count(id)==0)
        {
          cout<<"Please enter an ID of process that exists. enter it again"<<endl;
          int i;
        cin>>i;
          id=i;
        }

        string file_name;
        cout<<"Please enter the filename which you want to open"<<endl;
        cin>>file_name;
        
        mapper[id]->local_file_table[mapper[id]->local_file_table.size()]=file_name;

        if(global_file_table.count(file_name)==0)
        {
          //basically it means that this file is not opened globally by any other process yet. this process is opening the file globally for the first time
        global_file_table[file_name]=global_file_table.size()+1;
        }
        else
        {
          cout<<"This file has already been opened by another process globally in the global file table. the local file table has been updated"<<endl;
        }

        cout<<"The list of the files opened by the local file table is :"<<endl;
        for(auto x : mapper[id]->local_file_table)
          {
            cout<<x.second<<endl;
          }
        cout<<"The list of the files that are opened globally and their respective inode number are :"<<endl;

        for(auto x:global_file_table)
          {
            cout<<x.first<<" "<<x.second<<endl;
          }
        
      }
      else if(number==10)
      {
        int ids;
        cout<<"Please enter the ID of the process which you want to delete"<<endl;
        cin>>ids;

        while(mapper.count(ids)==0)
        {
          cout<<"Please enter a valid ID number of the process. Please enter it again."<<endl;
          cout<<"Please enter the valid ID number again"<<endl;
          int i;
          cin>>i;
          ids=i;
        }

        mapper.erase(ids);//basically remove the process again.

        //clearing the array so that we will use it again.
        nums.clear();
  
         for(auto &x:mapper)
         {
          vector<int>arr;
            arr.push_back(x.second->quanta);
          arr.push_back(x.first);
           nums.push_back(arr);
          }
        
  sort(nums.begin(),nums.end());
  //create the process tree again from scracth using BST. I could have just added the deletion function separately but instead i doing it the lazy way, i.e, creating the process tree from scracth.
   ProcessTree=process_tree(nums, 0, nums.size()-1);
        
      }
      else if(number==11)
      {
        //Implement MLFQ.
        vector<int>first_priority;
        vector<int>second_priority;
        vector<int>third_priority;
        mlfq.push_back(first_priority);
        mlfq.push_back(second_priority);
        mlfq.push_back(third_priority);

        for(auto p:mapper)
        {
          //within MLFQ,we can have different algorithms running at each level of the queue. at one level,maybe round robin,other maybe FIFO, etc. here, the running processes have highest priority, followed by others.
      if(p.second->process_state=="Running")
        mlfq[0].push_back(p.first);        
      else if(p.second->process_state=="Ready")
        mlfq[1].push_back(p.first);
      else
        mlfq[2].push_back(p.first);
       }


        // For seeing the processes at each level.

        for(int i=0;i<3;i++)
          {
            if(i==0)
              cout<<"The first priority queue in MLFQ is the running processes. they should finish first."<<endl;
            else if(i==1)
                cout<<"the second priority queue in MLFQ is the ready processes."<<endl;
            else
              cout<<"The last priority queue in the MLFQ is the IDs of the blocked processes"<<endl;
            
            for(auto x:mlfq[i])
              cout<<x<<" ";
            printf("\n");
          }

        mlfq.clear();//clearing the mlfq 2-D vector.

     
      }
      else if(number==12)
      {
      
      cout<<"Please enter the ID of the process for which you want to see the VA-PA mappings."<<endl;
        int ids;
        cin>>ids;

        while(mapper.count(ids)==0)
          {
            cout<<"This process id does not exist. Please enter the correct process ID once again"<<endl;
            int i;
            cin>>i;
            ids=i;
          }

         cout<<"The VA-PA mappings are as follows: "<<endl;
        for(auto x:mapper[ids]->page_table)
          {
            cout<<"The Virtual Page number "<<x.first<<" is allocated at physical frame number "<<x.second<<endl;
          }
          printf("\n");
          
     }
      else if(number==13)
      {
        cout<<"Please enter the ID of the process for which you want to see Thread related info inside it."<<endl;
        int ids;
        cin>>ids;

        while(mapper.count(ids)==0)
          {
            cout<<"This process id does not exist. Please enter the correct process ID once again"<<endl;
            int i;
            cin>>i;
            ids=i;
          }
          cout<<"The thread info associated with this process are as follows : "<<endl;
          cout<<"The total number of threads associated with this process are "<<mapper[ids]->threading_info.size()<<endl;
          
          for(auto thrd:mapper[ids]->threading_info)
          {
          	thrd.second->printdetails();
          }
      
      }
      else if(number==14)
      {
      	cout<<"The total available free memory is : "<<endl;
      	cout<<memorysize<<endl;
      }
      else if(number==15)
      {
      	cout<<"Please enter the ID of the process for which you want to see Thread related info inside it."<<endl;
        int ids;
        cin>>ids;

        while(mapper.count(ids)==0)
          {
            cout<<"This process id does not exist. Please enter the correct process ID once again"<<endl;
            int i;
            cin>>i;
            ids=i;
          }
          
          cout<<"The program counter (PC) of this process is : "<<mapper[ids]->program_Counter<<endl;
          cout<<"The stack Pointer(SP) of this process is : "<<mapper[ids]->stack_pointer<<endl;
          
      }
      else if(number==16)
      {
      cout<<"Please enter the ID of the process for which you want to call a function call"<<endl;
        int ids;
        cin>>ids;

        while(mapper.count(ids)==0)
          {
            cout<<"This process id does not exist. Please enter the correct process ID once again"<<endl;
            int i;
            cin>>i;
            ids=i;
          }
         cout<<"Doing a function call now for the process id : "<<ids<<endl;
         sleep(5);
         
         cout<<"Updating the program counter to the part where it is defined on the program."<<endl;
         int old_pc=mapper[ids]->program_Counter;
         mapper[ids]->program_Counter=500+rand()%1000;
         cout<<"Updating the stack pointer now. "<<endl;
         cout<<"Pushing a new stack frame to the stack and updating the Stack Pointer(SP). this new frame will store the old Program counter, the arguments to the function as well as the return value."<<endl;
         sleep(5);
         
         int old_sp=mapper[ids]->stack_pointer;
         mapper[ids]->stack_pointer=mapper[ids]->stack_pointer-5;//since stack grows  upwards so thats why the stack pointer is getting lesser. whereas the program counter will get bigger as it goes down.
         vector<int>arguments;
         int num_arg=1+rand()%5;
         
         while(num_arg--)
         {
         	arguments.push_back(5+rand()%30);
         }
         
         sleep(5);
         
         cout<<"The arguments to this function,the new stack pointer(SP) and the new program counter(PC) respectively are :"<<endl;
         cout<<"Total number of arguments is : "<<arguments.size()<<" and they are given below : "<<endl;
         for(auto arg:arguments)
         cout<<arg<<" ";
         cout<<"New stack frame is located at  : "<<mapper[ids]->stack_pointer<<endl;
         cout<<"The function is being called at the program counter : "<<mapper[ids]->program_Counter<<endl;
         
         sleep(5);
         cout<<"The function call has been completed. now popping the stack frame and reverting the values of program counter,stack pointer and registers back."<<endl;
         mapper[ids]->program_Counter=old_pc;
         mapper[ids]->stack_pointer=old_sp;
         arguments.clear();
  
      }
      else if(number==17)
      {
      	//code for system call simulation.
      	//generate trap function, etc.
      	cout<<"Please enter the ID of the process for which you want to call a System Call"<<endl;
        int ids;
        cin>>ids;

        while(mapper.count(ids)==0)
          {
            cout<<"This process id does not exist. Please enter the correct process ID once again"<<endl;
            int i;
            cin>>i;
            ids=i;
          }
        cout<<"System Call has been Invoked. Generating the Trap Instruction Now."<<endl;
      	cout<<"Setting the privilege bit to 1 and switching from User to Kernel Mode. Taking the CPU to higher privilege level."<<endl;
      	sleep(2);
      	privilege_bit=1;//privilege bit set.
      	
      	cout<<"Updating the stack Pointer to the Kernel stack now."<<endl;
      	int old_sp=mapper[ids]->stack_pointer;
      	sleep(2);
      	cout<<"Saving the Old register,the context  as well as the registers on the kernel stack now."<<endl;
      	int old_pc=mapper[ids]->program_Counter;
      	kernel_stack k_s(old_pc,rand()%5000,rand()%3000,rand()%4000);//saving the old PC,and registers(registers randomly generated) in the kernel stack.
      	mapper[ids]->stack_pointer=k_s.stack_ptr;//stack pointer now pointing to the kernel stack.
      	sleep(3);
      	cout<<"Now looking up the IDT table to look at which address to jump to. The Program Counter will then be updated to that address."<<endl;

      	vector<string>vec={"Illegal Memory Address","Network packet has arrived","Open a file","Write to a file","Close a file","Go to random location in an opened file."};
      	string res=vec[rand()%(vec.size())];
	mapper[ids]->program_Counter=interrupt_descriptor_table[res]; 
	sleep(3);
	
	cout<<"The system call which is being executed is : "<<endl;
	cout<<res<<endl;
	cout<<"And it is located at the memory Address : "<<endl;
	cout<<mapper[ids]->program_Counter<<endl;
	sleep(2);
	
	cout<<"Now calling the Return from trap instruction"<<endl;
	cout<<"Restoring the context,CPU Registers and the Program Counter and stack pointer back"<<endl;
	mapper[ids]->program_Counter=old_pc;
      	mapper[ids]->stack_pointer=old_sp;
      	sleep(1);
      	cout<<"System call successfully executed. Changing back the privilege bit to 0 and now coming to User Mode from Kernel mode."<<endl;
      	sleep(1);

      	privilege_bit=0;//privilege bit is 0. that means that we are in User Mode again(lesser privilege mode.)
      	
      }
      else if(number==18)
      {
      	cout<<"Showing the contents of the Interruptor Descriptor Table(IDT) and the address where they are stored, now :"<<endl;
      	sleep(2);
      	for(auto instruction:interrupt_descriptor_table)
      	{
      		cout<<"The address where the instruction for ' "<<instruction.first<<" ' is stored at"<<instruction.second<<endl;
      	}    	
      	sleep(10);
      }
      else if(number==19)
      {
      //Making a scheduler.
      	cout<<"Please enter the ID of the process where you are currently in"<<endl;
        int ids;
        cin>>ids;

        while(mapper.count(ids)==0)
          {
            cout<<"This process id does not exist. Please enter the correct process ID once again"<<endl;
            int i;
            cin>>i;
            ids=i;
          }
        
       //OS Scheduler will now pick up the process which had the least running time till now. it will have the most privilege and will get context switched to.
         
        Node* nodes=ProcessTree;
        
        while(nodes->left!=NULL)
        {
        	nodes=nodes->left;
        }
        
        int proc_id=nodes->node;
        int quanta=nodes->time;
        
        cout<<"OS Scheduler will now pickup the process which has run for the least time till now.\n"<<endl;
        cout<<"The process which was running previously had the Process ID of "<<ids<<" and had ran for a time period of : "<<mapper[ids]->quanta<<endl;
        sleep(3);
        cout<<"Going to higher-privileged kernel mode from User mode. setting the privilege bit to 1 now.\n"<<endl;
        privilege_bit=1;
        sleep(3);
	 cout<<"saving the context,Program Counters(PC's) and registers on the kernel stack now."<<endl;
	 int first_register=rand()%10000;          //assuming that it had access to two registers and generated them randomly.
	 int second_register=rand()%11000;
	 cout<<"The process which we are going to context switch to has process ID : "<<proc_id<<" and had ran for time period of : "<<quanta<<endl; 
	 sleep(3);
	 kernel_stack k_s(mapper[ids]->program_Counter,mapper[ids]->stack_pointer,first_register,second_register);        
         cout<<"Now Switching the stack pointer of the old process to the Kernel stack of B. From there the Return from trap will be invoked.\n"<<endl;
         cout<<"We will now restore the context and registers from this kernel stack"<<endl;
         mapper[ids]->stack_pointer=mapper[proc_id]->stack_pointer;
         mapper[ids]->program_Counter=mapper[proc_id]->program_Counter;
         //It is assumed that the new process to which we are going to context switch to, had also been saved sometime back in the memory while it had been context switched out.
         sleep(3);
         cout<<"Context switch done. We will now change the CPU Privilege level from kernel to user mode again. again setting the privilege_bit to 0."<<endl;
         privilege_bit=0;
         cout<<"Now Jumping back to the User Code once again. The new process will now resume from here.\n"<<endl;
         sleep(3);

          
      }
      else if(number==21)
      {
      }
      else if(number==22){
      
      	simulate_caching();
      
      }
    }
}
