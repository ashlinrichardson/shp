#include<string>
#include<vector>
#include<stdio.h>
#include<fstream>
#include<iostream>
#include<stdlib.h>
#include"ansicolor.h"
#include"rapidjson/document.h"
using namespace std;
using namespace rapidjson;

long int getFileSize(std::string fn){
  ifstream i;
  i.open(fn.c_str(), ios::binary);
  if(!(i.is_open())){
    printf("Error: was unable to open file: %s\n", (fn.c_str()));
    return -1;
  }
  i.seekg(0, ios::end);
  long int len = i.tellg();
  return(len);
}

int main(int argc, char ** argv){
  vector<string> data_PORC; /* primary ORC */
  vector<string> data_PNAM; /* park name */
  vector<string> data_COOR; /* GIS coordinate string (WKT format) */


  std::string fn("TA_PEP_SVW_polygon.json");
  long int fs = getFileSize(fn);
  char * fd = (char *)(void *)malloc(fs);
  memset(fd, '\0', fs);
  printf("load file\n");
  FILE * f = fopen(fn.c_str(), "rb");
  long int br = fread(fd, 1, fs, f);
  if(br != fs){
    printf("Error: br (%ld)!= fs (%ld)\n", (long int)br, (long int)fs);
    exit(1);
  }
  long int i;
  long int f2 = fs;
  for(i = 0; i < fs; i++){
    if(fd[i] == '\n'){
      f2 -= 1;
    }
  }
  cout << fs << endl << f2 << endl;

  char * fd2 = (char *)(void *)malloc(f2);
  memset(fd2, '\0', f2);

  printf("remove newline\n");
  long int ci = 0;
  for(i = 0; i < fs; i++){
    if(fd[i] != '\n'){
      fd2[ci++] = fd[i];
    }
  }

  printf("parsing json...\n");
  Document document;
  document.Parse((const char *)(void *)fd2);
  assert(document.IsObject());

  static const char* kTypeNames[] = {
    "Null", "False", "True", "Object", "Array", "String", "Number"
  };

  /* for all doc members */
  for(Value::ConstMemberIterator itr = document.MemberBegin();
  itr != document.MemberEnd();
  ++itr){

    printf("%s%s%s%s (%s%s%s)\n", KGRN, KRED, itr->name.GetString(),
    KGRN, KYEL, kTypeNames[itr->value.GetType()], KGRN);

    /* assert(itr->IsArray()); */
    if(!strncmp("Array\0", kTypeNames[itr->value.GetType()], 5)){

      printf("\tisArray=true\n");
      const Value& a = itr->value;
      int c = 0;

      /* for all members in a */
      for(Value::ConstValueIterator itr2 = a.Begin();
      itr2 != a.End();
      ++itr2){

        printf("%ld %s\n", (long int)(c++), itr2->IsObject()?"true":"false");
        itr2->MemberBegin();

        /* for all the members in iter2 */
        for(Value::ConstMemberIterator itr3 = itr2->MemberBegin();
        itr3 != itr2->MemberEnd();
        ++itr3){

          printf("\t%s%s%s%s (%s%s%s)\n", KGRN, KRED, itr3->name.GetString(),
          KGRN, KYEL, kTypeNames[itr3->value.GetType()], KGRN);
          /* Type of member geometry: Object */

          if(!strncmp("String\0", kTypeNames[itr3->value.GetType()], 5)){
            printf("\t\t\t%s\n", itr3->value.GetString());
          }

          if(!strncmp("Object\0", kTypeNames[itr3->value.GetType()], 5)){
            if(!strncmp("properties\0", itr3->name.GetString(), 10)){
              for(Value::ConstMemberIterator itr4 = itr3->value.MemberBegin();
              itr4 != itr3->value.MemberEnd();
              ++itr4){

                printf("\t\t%s%s%s%s (%s%s%s)\n", KGRN, KRED, itr4->name.GetString(),
                KGRN, KYEL, kTypeNames[itr3->value.GetType()], KGRN);

                if(!strncmp("ORC_PRIMRY\0", itr4->name.GetString(), 10)){
                    printf("\t\t\tORC_PRIMRY=%s\n", itr4->value.GetString());
                }

                if(!strncmp("PROT_NAME\0", itr4->name.GetString(), 9)){
                    printf("\t\t\tPROT_NAME=%s\n", itr4->value.GetString());
                }
              }

            }
            if(!strncmp("geometry\0", itr3->name.GetString(), 8)){
              /* printf("\t\t%s\n", itr3->name.GetString()); */
              for(Value::ConstMemberIterator itr4 = itr3->value.MemberBegin();
              itr4 != itr3->value.MemberEnd();
              ++itr4){

                printf("\t\t%s%s%s%s (%s%s%s)\n", KGRN, KRED, itr4->name.GetString(),
                KGRN, KYEL, kTypeNames[itr3->value.GetType()], KGRN);

                /* Type of member "type": Object */
                /* Type of member "coordinates": Object */

                /* Next step: open these two objects */

                /* "features": [
                { "type": "Feature", "properties":
                { "SRV_GEN_PL": "3TU1901", "ADMIN_AREA": 626.0, "F_CODE": "FA02550271",
                "ORC_SCNDRY": "00", "ORC_PRIMRY": "0237", "PROT_CODE": "PP",
                "PARK_CLASS": "Class A", "OBJECTID": 389.0, "PROT_DESG": "PROVINCIAL PARK",
                "PROT_NAME": "DISCOVERY ISLAND MARINE PARK"
                */
              }
            }
          }
        }
      }
    }
  }
  printf("done\n");
  return 0;
}
