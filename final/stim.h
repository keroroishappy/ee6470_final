#ifndef _STIM_H_
#define _STIM_H_
#include <iomanip>
#include "short_path.h"

using std::setw;
using namespace sc_core;
using namespace sc_dt;
using namespace std;
#include <systemc>
#include <cynw_p2p.h>







SC_MODULE(stim) {

public:  
  

 	unsigned int n_txn;
	sc_time max_txn_time;
	sc_time min_txn_time;
	sc_time total_txn_time;
	sc_time total_start_time;
	sc_time total_run_time;
 
 
  sc_in_clk i_clk;
  sc_uint<28> temp_a;
  sc_out<bool> o_rst;
  sc_uint<16>dist_result_stim;
  
#ifndef NATIVE_SYSTEMC  
   cynw_p2p<sc_dt::sc_uint<WIDTH> >::base_out o_a_port;
   cynw_p2p<sc_dt::sc_uint<WIDTH> >::base_in i_dis_port;
#else
  sc_fifo_out<sc_dt::sc_uint<WIDTH> > o_a_port;
  sc_fifo_in<sc_dt::sc_uint<WIDTH> > i_dis_port;
#endif




  int point;
  int dis;
  char a_arr[14]={0,0,1,1,2,2,2,3,3,4,5,6,6,7};
  char b_arr[14]={1,7,2,7,3,8,5,4,5,5,6,7,8,8};
  char c_arr[14]={4,8,8,11,7,2,4,9,14,10,2,1,6,7};
  char src = 4;
  
  
  SC_HAS_PROCESS(stim);

  stim(sc_module_name n);
  
  ~stim();
  
  int clock_cycle( sc_time time );
  
  void stim_gen();

  void sink();

};
#endif