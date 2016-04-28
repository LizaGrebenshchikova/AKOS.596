#include <stdio.h>
#include <string.h>
#include <uniwbrk.h>
#include <unistr.h>
#include <unicase.h>
#include <vector>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <map>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <algorithm>
#include <pthread.h>



#define NUM_THREADS 4
long int* start_positions;
long int* end_positions; 
pthread_t* ourthreads;
pthread_t* ourthreads1;
std::vector<std::multimap<std::string, int>>OurTrees;


std::vector<std::string> tokenize(const char *input) {
  uint8_t *uinput = (uint8_t *)input;
  const uint8_t *pos = uinput, *beg;
  std::vector<std::string> result;
  ucs4_t uc;
  bool in_word = false;
  do {
    const uint8_t *old_pos = pos;
    pos = u8_next(&uc, pos);
    if (uc_wordbreak_property(uc) != WBP_ALETTER) {
      if (in_word) {
        size_t len = old_pos - beg;
        uint8_t word[len + 1];
        memset(word, 0, sizeof(word));
        u8_tolower(beg, len, "ru", UNINORM_NFC, word, &len);
        result.push_back(reinterpret_cast<char *>(word));
      }
      in_word = false;
    } else {
      if (!in_word) {
        beg = old_pos;
      }
      in_word = true;
    }
  } while (pos && uc);
  return result;
}

void threadwork (int * i) {
    std::ofstream out;
    char* filename = (char*) malloc (260*sizeof(char) );
    sprintf (filename, "%s%d%s", "curout", *i, ".txt");
    out.open (filename);
    FILE* ourfile = fopen ("input.txt", "r");
    char* cur_line = (char*) malloc (260*sizeof(char) );
    fseek(ourfile, start_positions[*i], SEEK_SET);
    while (ftell(ourfile) + 1 < end_positions[*i]  ) {
          fscanf (ourfile, "%s", cur_line); 
        
          std::vector<std::string> words = tokenize(cur_line);
          for(size_t j = 0; j < words.size();  j++) {
              out<<words[j]; out<<std::endl;
          }
          cur_line[0]='\0';
    }
    out.close();
    free(filename);
    free(cur_line);
    fclose(ourfile);

    char* filename1 = (char*) malloc (260*sizeof(char) );
    sprintf (filename1, "%s%d%s", "curout", *i, ".txt");

    char* filename2 = (char*) malloc (260*sizeof(char) );
    sprintf (filename2, "%s%d%s", "output", *i, ".txt");

    char* filename3 = (char*) malloc (260*sizeof(char) );
    sprintf (filename3, "%s%d", "./stemwords", *i);
    
    char** execargs = (char**)malloc(6*sizeof(char*) );
    execargs[0] = (char*)malloc (200*sizeof(char) );
    strcpy(execargs[0], filename3 );
    execargs[1] = (char*)malloc (200*sizeof(char) );
    strcpy(execargs[1], "-i" );
    execargs[2] = (char*)malloc (200*sizeof(char) );
    strcpy(execargs[2], filename1 );
    execargs[3] = (char*)malloc (200*sizeof(char) );
    strcpy(execargs[3], "-o" );
    execargs[4] = (char*)malloc (200*sizeof(char) );
    strcpy(execargs[4], filename2);
    execargs[5]=NULL;
    //execvp(execargs[0],execargs);
    pid_t child_pid;
    int child_status;
    child_pid = fork ();
    if (child_pid == 0) {
            execvp (execargs[0], execargs);
            printf("cucu\n");
    } else {
            if( (child_pid = wait(&child_status)) < 0){
            std::cout<<"wait";
            _exit(1);
          }
    }
 
        
    
   
    free(execargs[0]);
    free(execargs[1]);
    free(execargs[2]);
    free(execargs[3]);
    free(execargs[4]);
    free(execargs[5]);
    free(execargs);
    free(filename1);
    free(filename2);
    free(filename3);
}

void threadwork2 (int * i) {
  std::ifstream in;
  char* filename4 = (char*) malloc (260*sizeof(char) );
  sprintf (filename4, "%s%d%s", "output", *i,".txt");
  in.open(filename4);
  free(filename4);
  std::string tmp;
  while(!in.eof()){
    in>>tmp;
    if (OurTrees[*i].find(tmp) == OurTrees[*i].end()) {
     OurTrees[*i].insert(std::make_pair(tmp,1));
   } else {
     OurTrees[*i].find(tmp)->second++;

   }
 }
}

void threadwork3(int*u){
  int thread_number = *u;
  int*nums=(int*)malloc(thread_number*sizeof(int));
  for(int j = 0; j < thread_number; j++){
  nums[j] = j;
  }
  for (int j = 0; j < thread_number; j++) {
    pthread_create(ourthreads1 + j, NULL, (void * (*)(void *)) threadwork2, nums+j);
  }
  for (int j = 0; j < thread_number; j++) {
    pthread_join(ourthreads1[j], NULL);
  }
}
int main() {

start_positions = (long int*)malloc (sizeof(long int)*NUM_THREADS );
end_positions = (long int*)malloc (sizeof(long int)*NUM_THREADS );
ourthreads = (pthread_t*)malloc (sizeof(pthread_t)*(NUM_THREADS + 1));
ourthreads1 = (pthread_t*)malloc (sizeof(pthread_t)*NUM_THREADS );
OurTrees.resize(NUM_THREADS);

FILE * file_;
file_ = fopen ("input.txt","rw");
fseek(file_, 0, SEEK_END);
long int filelength = ftell(file_);
fseek(file_, 0, SEEK_SET);
start_positions[0] = 0;
size_t i = 0;
int c;
std::cout<<filelength<<"\n";

for (i = 0; i < NUM_THREADS; ++i) {
    if ( start_positions[i] >= filelength )
        break;
    long int cur_pos = std::min (start_positions[i] + filelength/NUM_THREADS, filelength - 1);
    std::cout<<cur_pos<<"\n";
    fseek (file_, cur_pos, SEEK_SET);
    //std::cout<<ftell(file_)<<"\n";
    while  ( (c = fgetc (file_)) != EOF ) {
          if ( char(c) == '\n') 
             break;
    }
    end_positions[i] = ftell (file_) - 1;
    if (i+1 < NUM_THREADS) {
        start_positions[i+1] = end_positions[i] + 1;
    }
    std::cout<<start_positions[i]<< " "<<end_positions[i]<<"\n";
}

int threadnumber = (int)i;
int thread_number = threadnumber;
std::cout<<threadnumber<<"\n";
//char* text = (char*)malloc(filelength + 10*sizeof(char)); //с запасом просто
//fgets (text, filelength, file_);
fclose(file_);
int* nums = (int*)malloc(sizeof(int)*threadnumber);
for(int j = 0; j < threadnumber; j++){
  nums[j] = j;
  std::cout<<nums[j]<<"\n";
}
//std::vector<std::string> words = tokenize(text);
for (int j = 0; j < threadnumber; j++) {
    pthread_create(ourthreads + j, NULL, (void * (*)(void *)) threadwork, nums+j);
}
pthread_create(ourthreads + threadnumber, NULL, (void * (*)(void *)) threadwork3, &thread_number);
for (int j = 0; j <= threadnumber; j++) {
    pthread_join(ourthreads[j], NULL);
}
std::cout<<"cucu";



std::multimap< std::string, int> Result_Tree;
for (int i = 1; i < threadnumber  ; i++){
  if (OurTrees[0].size()==0){
    OurTrees[0].swap(OurTrees[i]);
  }
  if (OurTrees[i].size() == 0){
    continue;
  }
  auto it3 = OurTrees[0].begin();
  auto it4 = OurTrees[i].begin();
  if (it3->first < 
  it4->first){
        Result_Tree.insert(*it3);
        ++it3;
    } else {
        Result_Tree.insert(*it4);
        ++it4;
    }
  auto it5 = Result_Tree.begin();
  while ((it3!=OurTrees[0].end()) && (it4!=OurTrees[i].end())){
    if (it3->first < it4->first){
        it5 = Result_Tree.insert(it5,*it3);
        ++it3;
    } else {
        it5 = Result_Tree.insert(it5,*it4);
        ++it4;
    }
  }
  while (it3!=OurTrees[0].end()){
     it5 = Result_Tree.insert(it5,*it3);
        ++it3;
  }
  while (it4!=OurTrees[i].end()){
     it5 =  Result_Tree.insert(it5,*it4);
        ++it4;
  }
  Result_Tree.swap(OurTrees[0]);
  Result_Tree.clear();
}

Result_Tree.swap(OurTrees[0]);
auto it1 = Result_Tree.begin();
auto it2 = it1;
++it2;
std::ofstream out;
out.open("result.txt");
while(it2!=Result_Tree.end()){
  if (it2->first == it1->first){
    it2->second += it1->second;
  } else {
    out<< it1->first<<" "<<it1->second<<"\n";
  }
  ++it1;
  ++it2;
}
out<< it1->first<<" "<<it1->second<<"\n";

out.close();

  
  return 0;
}
