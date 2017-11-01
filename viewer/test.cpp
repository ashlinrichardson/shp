#include<fstream>
#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include<string>
#include"ansicolor.h"
using namespace std;

#include "rapidjson/document.h"
using namespace rapidjson;

long int getFileSize(std::string fn){
  ifstream i;
  i.open(fn.c_str(), ios::binary);
  if(!(i.is_open())){
    printf("Error: was unable to open file: %s\n", (fn.c_str()));
    return (-1);
  }
  i.seekg(0, ios::end);
  long int len = i.tellg();
  return(len);
}

int main(int argc, char ** argv){
  std::string fn("TA_PEP_SVW_polygon.json");
  long int fs = getFileSize(fn);
  char * fd = (char *)(void *)malloc(fs);
  memset(fd, '\0',fs);
  printf("load file\n");
  FILE * f = fopen(fn.c_str(), "rb");
  long int br = fread(fd, 1, fs, f);
  if(br != fs){
    printf("Error: br (%ld)!= fs (%ld)\n", (long int)br, (long int)fs); exit(1);
  }
  long int i;
  long int f2 = fs;
  for(i = 0; i < fs; i++){
    if(fd[i] == '\n'){
      f2 -= 1;
    }
  }
  cout << fs << endl;
  cout << f2 << endl;

  char * fd2 = (char *)(void *)malloc(f2);
  memset(fd2, '\0',f2);

  printf("remove newline\n");
  long int ci = 0;
  for(i = 0; i < fs; i++){
    if(fd[i] != '\n'){
      fd2[ci++] = fd[i];
    }
  }

  printf("parsing json...\n");
  Document document;
  document.Parse((const char *)(void *) fd2);
  assert(document.IsObject());

  static const char* kTypeNames[] = {
    "Null", "False", "True", "Object", "Array", "String", "Number"
  };

  for(Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr){
    printf("%sType of member %s%s%s is %s%s%s\n",
    KGRN,
    KRED,
    itr->name.GetString(),
    KGRN,
    KYEL,
    kTypeNames[itr->value.GetType()],
    KGRN
    );
    printf("here\n");
    if(!strncmp("Array\0", kTypeNames[itr->value.GetType()],5)){
      //assert(itr->IsArray());
      printf("\tisArray=true\n");
      const Value& a = itr->value;
      int c = 0;
      for (Value::ConstValueIterator itr2 = a.Begin(); itr2 != a.End(); ++itr2){
        printf("%ld %s\n", c++, itr2->IsObject()?"true":"false");
        itr2->MemberBegin();

        // for all the members in iter2
        for (Value::ConstMemberIterator itr3 = itr2->MemberBegin(); itr3 != itr2->MemberEnd(); ++itr3){
          printf("\t%sType of member %s%s%s is %s%s%s\n",
          KGRN,
          KRED,
          itr3->name.GetString(), //iter3 is reference to a member
          KGRN,
          KYEL,
          kTypeNames[itr3->value.GetType()],
          KGRN
          );
          if(true){  //!strncmp("Array\0", kTypeNames[itr->value.GetType()],5)){
          //printf("\t\t%s\n", (itr3)["type"].GetString());

        }
      }
    }
    else{
    }
  }

  printf("done\n");
  return 0;
}
