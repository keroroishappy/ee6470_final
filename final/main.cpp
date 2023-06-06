#include "stim.h"
//#define CLOCK_PERIOD 1.0
#include <sys/time.h>








struct timeval start_time, end_time;








int sc_main(int argc, char *argv[]) {
  
  //Create modules and signals
  stim testbench("testbench");
  short_path dut("dut");
  sc_clock clk("clk", CLOCK_PERIOD, SC_NS);
  sc_signal<bool> rst("rst");


  //Create FIFO channels
// #ifndef NATIVE_SYSTEMC
//   cynw_p2p<sc_uint> src;
//   cynw_p2p<sc_uint<WIDTH> > fifo_i_a;
//   cynw_p2p<sc_uint<WIDTH+1> > fifo_o_dis;
//   cynw_p2p<sc_uint<WIDTH+1> > fifo_o_point;
// #else

  sc_fifo<sc_uint<WIDTH> > fifo_i_a;
  sc_fifo<sc_uint<WIDTH+1> > fifo_o_dis;

// #endif
  //Connect FIFO channels with modules
  testbench.i_clk(clk);
  testbench.o_rst(rst);
  
  dut.i_clk(clk);
  dut.i_rst(rst);

  testbench.o_a_port(fifo_i_a);
  testbench.i_dis_port(fifo_o_dis);

  dut.i_a_port(fifo_i_a);
  dut.o_dis_port(fifo_o_dis);


  sc_start(100,SC_MS);
  return 0;
}
