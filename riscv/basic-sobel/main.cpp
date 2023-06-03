#include "string"
#include "string.h"
#include "cassert"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include <iostream>
#include "math.h"

using namespace std;

union word {
  int sint;
  unsigned int uint;
  unsigned char uc[4];
};

  int mstatus=0;
  int point;
  int dis;
  unsigned char a_arr[15]={0,0,1,1,2,2,2,3,3,4,5,6,6,7,8};
  unsigned char b_arr[15]={1,7,2,7,3,8,5,4,5,5,6,7,8,8,1};
  unsigned char c_arr[15]={4,8,8,11,7,2,4,9,14,10,2,1,6,7,1};

  //unsigned char a_arr[14] = {9,9,10,10,11,11,11,12,12,13,14,15,15,16};
  //unsigned char a_arr[14] = {10,16,11,16,12,17,14,13,14,14,15,16,17,17};
  //unsigned char a_arr[14] = {4,8,8,11,7,2,4,9,14,10,2,1,6,7};

  unsigned char src = 4;


// Sobel Filter ACC
static char* const SOBELFILTER_START_ADDR = reinterpret_cast<char* const>(0x73000000);
static char* const SOBELFILTER_READ_ADDR  = reinterpret_cast<char* const>(0x73000004);

// DMA 
static volatile uint32_t * const DMA_SRC_ADDR  = (uint32_t * const)0x70000000;
static volatile uint32_t * const DMA_DST_ADDR  = (uint32_t * const)0x70000004;
static volatile uint32_t * const DMA_LEN_ADDR  = (uint32_t * const)0x70000008;
static volatile uint32_t * const DMA_OP_ADDR   = (uint32_t * const)0x7000000C;
static volatile uint32_t * const DMA_STAT_ADDR = (uint32_t * const)0x70000010;
static const uint32_t DMA_OP_MEMCPY = 1;

bool _is_using_dma = true;



///////////////multi thread define//////////////////////

int sem_init (uint32_t *__sem, uint32_t count) __THROW
{
  *__sem=count;
  return 0;
}

int sem_wait (uint32_t *__sem) __THROW
{
  uint32_t value, success; //RV32A
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     beqz %[value],L%=                   # if zero, try again\n\t\
     addi %[value],%[value],-1           # value --\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  return 0;
}

int sem_post (uint32_t *__sem) __THROW
{
  uint32_t value, success; //RV32A
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     addi %[value],%[value], 1           # value ++\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  return 0;
}
bool _is_using_dma = false;
int barrier(uint32_t *__sem, uint32_t *__lock, uint32_t *counter, uint32_t thread_count) {
	sem_wait(__lock);
	if (*counter == thread_count - 1) { //all finished
		*counter = 0;
		sem_post(__lock);
		for (int j = 0; j < thread_count - 1; ++j) sem_post(__sem);
	} else {
		(*counter)++;
		sem_post(__lock);
		sem_wait(__sem);
	}
	return 0;
}

//////////////////////////////////////////////////////////////







void write_data_to_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){  
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(buffer);
    *DMA_DST_ADDR = (uint32_t)(ADDR);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Send
    //cout<<"trigger w"<<endl;
    memcpy(ADDR, buffer, sizeof(unsigned char)*len);
  }
}
void read_data_from_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(ADDR);
    *DMA_DST_ADDR = (uint32_t)(buffer);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Read
    //cout<<"trigger r"<<endl;
    memcpy(buffer, ADDR, sizeof(unsigned char)*len);
  }
}

#define PROCESSORS 2
//the barrier synchronization objects
uint32_t barrier_counter=0; 
uint32_t barrier_lock; 
uint32_t barrier_sem; 
//the mutex object to control global summation
uint32_t lock;  
//print synchronication semaphore (print in core order)
uint32_t print_sem[PROCESSORS]; 



int main(){

 unsigned char buffer[4] = {0};
  word data;
	printf("start giving data to dij \n ");
  printf("======================================\n");


	if (hart_id == 0) {
		// create a barrier object with a count of PROCESSORS
		sem_init(&barrier_lock, 1);
		sem_init(&barrier_sem, 0); //lock all cores initially
		for(int i=0; i< PROCESSORS; ++i){
			sem_init(&print_sem[i], 0); //lock printing initially
		}
		// Create mutex lock
		sem_init(&lock, 1);
	}

    int transaction_data = 0;

    // if(hart_id==0){
    //   sem_wait(&lock);
      for (int i = 0; i < 14; i++) {

	      //o_a_port.write(temp_a);
        
              buffer[0] = c_arr[i];
              //cout<<buffer[0]<<endl;
              buffer[1] = b_arr[i];
              //cout<<buffer[1]<<endl;
              buffer[2] = a_arr[i];
              //cout<<buffer[2]<<endl;
              buffer[3] = src;
              //cout<<buffer[3]<<endl;

            //printf("buffer %d",buffer[2]);
            write_data_to_ACC(SOBELFILTER_START_ADDR, buffer, sizeof(buffer));

            transaction_data = (i+1)*4;
          
      }  
      //sem_post(&lock);
    //}
    // else{
    //   for (int i = 0; i < 14; i++) {
    //     sem_wait(&lock);
	  //     //o_a_port.write(temp_a);
        
    //           buffer[0] = c_arr[i];
    //           //cout<<buffer[0]<<endl;
    //           buffer[1] = b_arr[i];
    //           //cout<<buffer[1]<<endl;
    //           buffer[2] = a_arr[i];
    //           //cout<<buffer[2]<<endl;
    //           buffer[3] = src;
    //           //cout<<buffer[3]<<endl;

    //         //printf("buffer %d",buffer[2]);
    //         write_data_to_ACC(SOBELFILTER_START_ADDR, buffer, 4);   


    //   }  
    //   sem_post(&lock);
    // }
  //   if (hart_id == 0) {  
	// 	printf("core%d, transfer data done \n", hart_id);
	// 	sem_post(&print_sem[1]);  // Allow Core 1 to print
	// } else {
	// 	for (int i = 1; i < PROCESSORS; ++i) {
	// 		sem_wait(&print_sem[i]); 
	// 		printf("core%d, receive data done \n", hart_id);
	// 		sem_post(&print_sem[i + 1]);  // Allow next Core to print
	// 	}
	// }

    int DMA_cyc = 0;
    DMA_cyc = 2 + transaction_data/4;


    for (int x = 0; x <9; ++x){
  
      read_data_from_ACC(SOBELFILTER_READ_ADDR, buffer, sizeof(buffer));
      cout <<" dis :  " << (int)buffer[0] <<endl;
      cout <<" point :  " << (int)buffer[1] <<endl;
      if(x==8){
        mstatus=1;
      }
    }
    cout<<"mstatus = "<<mstatus<<" work is done"<<endl;
    cout<<"DMA cycle is "<< DMA_cyc <<endl;
}