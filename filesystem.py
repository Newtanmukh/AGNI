import sys
import os
import random

no_of_files=random.randrange(50,100)
file_inode_mappings={}

#the main thing in this folder, will show list of all the files whcih are inside each of the folder.
folder_contents={}

list=['bin','usr','pbs','opt','apt','init.d','dpkg','amazon','shell','hosts','perl','fonts','group','passwd','networks','resolv.conf','gitconfig']

folder_paths=[]

for i in range(no_of_files):
  depth=random.randrange(1,len(list))
  name="/"
  for j in range(depth):
    name=name+random.choice(list)+"/"
    
  if name in folder_paths:
    if i>0:
      i=i-1
  else:
    folder_paths.append(name)

folders_list=[]

for x in folder_paths:
  folders=x.split("/")
  for y in folders:
    if y not in folders_list:
      folders_list.append(y)


names=['test','faltu','check','ok','fun','pbs','google','software','semester','assignment','work','coding','leetcode','softcopies','hardcopies']
extensions=['.pdf','.txt','.cpp','.c','.go','.h','.txt','.sh','.conf','.p','.mat']

for x in folders_list:
  files_inside=[]
  number_of_files_inside=random.randrange(6,30)
  for i in range(number_of_files_inside):
    file_name=random.choice(names)+random.choice(extensions)
    if file_name not in files_inside:
      files_inside.append(file_name)
  folder_contents[x]=files_inside

#print(folder_contents)

check_if_inode_used=[]
AllFilesAndFolders=[]
for x in folder_contents.keys():
  if x not in AllFilesAndFolders:
    AllFilesAndFolders.append(x)
  for y in folder_contents[x]:
    if y not in AllFilesAndFolders:
      AllFilesAndFolders.append(y)

for x in AllFilesAndFolders:
  inode=random.randrange(1,10000)
  if inode not in check_if_inode_used:
    file_inode_mappings[x]=inode
    check_if_inode_used.append(inode)
  


for x in folder_paths:
  print(x)
  print("\n")
  
print("The files, folders and their corresponding inode mappings are  :  \n")

for x in file_inode_mappings.keys():
  print("File/Folder Name is : " + str(x) + "and their corresponding inode number is : "+ str(file_inode_mappings[x])+"\n")
  

while(1):
  folder_path=str(input("Please enter the folder path whose contents which you want to see : \n"))
  folders=folder_path.split("/")
  folder_name=folders[len(folders)-2]
  print("Folder name is : "+folder_name)
  if folder_name in folder_contents.keys():
    for x in folder_contents[folder_name]:
      print(x)
      print("\n")
  
