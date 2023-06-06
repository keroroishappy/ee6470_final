#include <cstdio>
#include <cstdlib>
using namespace std;
#include <iostream>
#include "stim.h"

#include <queue>
static std::queue<sc_time> time_queue;

  sc_in_clk i_clk;










  stim::stim(sc_module_name n):sc_module(n) {
  
    SC_THREAD(stim_gen);
    sensitive << i_clk.pos();
    dont_initialize();

    SC_THREAD(sink);
    sensitive << i_clk.pos();
    dont_initialize();
  
  }

  stim::~stim() {

	  cout << "Total run time = " << total_run_time << endl;
  }









  void stim::stim_gen() {
#ifndef NATIVE_SYSTEMC
	o_a_port.reset();
#endif
    cout << setw(12) << "time" << setw(12) << "a" << setw(12) << "b" <<
    setw(12) << "dis"<<endl;
    for (int i = 0; i < 14; i++) {
        temp_a.range(7,0) =   c_arr[i];

        temp_a.range(15,8) =  b_arr[i];

        temp_a.range(23,16) = a_arr[i];

        temp_a.range(27,24) = src;
        
#ifndef NATIVE_SYSTEMC
					o_a_port.put(temp_a);
#else
					o_a_port.write(temp_a);
#endif
        time_queue.push( sc_time_stamp() );
        cout << setw(12) << sc_time_stamp();
        cout << setw(12) << temp_a << endl;
    }
  }





  void stim::sink() {
    wait(100,SC_NS);
    unsigned long total_latency = 0;
    
    
    #ifndef NATIVE_SYSTEMC
	i_dis_port.reset();
#endif
    while(true){


#ifndef NATIVE_SYSTEMC      
      dist_result_stim=i_dis_port.get(); 
#else 
      dist_result_stim=i_dis_port.read();
#endif

      point=dist_result_stim.range(7,0);
      dis=dist_result_stim.range(15,8);
      cout << setw(12) <<" point" << point;
      cout << setw(12) <<" dis "<< dis << endl;
      sc_time sent_time( time_queue.front() );
      time_queue.pop();
        unsigned long latency = clock_cycle( sc_time_stamp() - sent_time );
        total_latency += latency;
        cout << "Latency for dut " <<  latency << endl;
      wait(); 
    }
    
    sc_stop();
unsigned long average_latency = total_latency / 262144;
    esc_log_latency( "dut", average_latency, "average_latency" );
    cout << "Average latency " << average_latency << "." << endl;
	total_run_time = sc_time_stamp() - total_start_time;

}




int clock_cycle( sc_time time )
{
    sc_clock * clk_p = dynamic_cast< sc_clock * >( i_clk.get_interface() );
    sc_time clock_period = clk_p->period();
    return ( int )( time / clock_period );

}
