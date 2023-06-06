#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#ifndef NATIVE_SYSTEMC
#include <cynw_p2p.h>
#endif
using namespace sc_dt;
#include <iostream>
#include <systemc>
#include <limits.h>

using namespace std;
//using namespace sc_dt;
using namespace sc_core;
const int WIDTH = 29;
#define V 9



class short_path : public sc_module {

public:

  int graph[V][V]={0};
//HLS_SEPARATE_ARRAY(graph);
//HLS_INVERT_DIMENSIONS(graph);
  int read_num=0;
  int src; 
  sc_in_clk i_clk;
  sc_in< bool > i_rst;
  sc_uint<16>dist_result_dij;


#ifndef NATIVE_SYSTEMC
   cynw_p2p<sc_dt::sc_uint<WIDTH> >::in i_a_port;
   cynw_p2p<sc_dt::sc_uint<WIDTH+1> >::out o_dis_port;
#else
  sc_fifo_in<sc_dt::sc_uint<WIDTH> > i_a_port;
  sc_fifo_out<sc_dt::sc_uint<WIDTH+1> > o_dis_port;
#endif

  SC_CTOR(short_path){ 

    SC_THREAD(addEdge);  
    sensitive << i_clk.pos();
    reset_signal_is(i_rst, false);
    SC_THREAD(dijkstra);  
    sensitive << i_clk.pos();
    dont_initialize();
    reset_signal_is(i_rst, false);
#ifndef NATIVE_SYSTEMC
 	i_a_port.clk_rst(i_clk, i_rst);
  o_dis_port.clk_rst(i_clk, i_rst);
#endif
  }

  void addEdge(){
  
{  
 #ifndef NATIVE_SYSTEMC
 		HLS_DEFINE_PROTOCOL("main_reset");
 		i_a_port.reset();
 #endif
    wait();
}  

//    for (int i = 0; i < V; i++){        
//      for (int j = 0; j < V; j++){            
//        graph[i][j] = 0;        
//      }    
//    } 


    while (true) {
      cout<<"-----------------------------------------"<<endl;
//HLS_PIPELINE_LOOP( HARD_STALL, 2,"pipeline" );

      
#ifndef NATIVE_SYSTEMC
{         HLS_DEFINE_PROTOCOL("input"); 
 	        _i_a = i_a_port.get();
 					wait();	
}
#else
          _i_a = i_a_port.read();  
#endif

    int dis = _i_a.range(7,0); 
    cout<<"dis :"<< dis <<endl;  
    int y = _i_a.range(15,8);
    cout<<"y :"<< y <<endl; 
    int x = _i_a.range(23,16);
    cout<<"x :"<< x <<endl; 
    src = _i_a.range(27,24);
    cout<<"src :"<< src <<endl; 
    cout<<"time is "<<sc_time_stamp()<<endl;
    graph[x][y] = dis; 
    graph[y][x] = dis;
    read_num = read_num + 1;
    cout<<"read_num "<< read_num << endl;;
    wait(1,SC_NS);
    }
  }


  int minDistance(int dist[], bool sptSet[]){
    int min = INT_MAX, min_index;
    for (int v = 0; v < V; v++){
HLS_PIPELINE_LOOP( HARD_STALL, 2,"pipeline" );
      if (sptSet[v] == false && dist[v] <= min){
        min = dist[v];
        min_index = v;
      }
    }
  return min_index;
}


  void dijkstra(){  

    wait(20,SC_NS);
    cout<<"dij triggered"<<endl;
    cout<<"time is "<<sc_time_stamp()<<endl;

#ifndef NATIVE_SYSTEMC
    o_dis_port.reset();
#endif
   wait();
    
  cout<<"read_num : "<<read_num<<endl;
  cout<<"src : "<<src<<endl;
  if(read_num==14){
    cout<<"read_num=14 triggered"<<endl; 
  int dist[V]; 
  bool sptSet[V]; 
  for (int i = 0; i < V; i++){
  
      dist[i] = INT_MAX, sptSet[i] = false;
  }
  dist[src] = 0;
  for (int count = 0; count < V - 1; count++) {
//HLS_UNROLL_LOOP(ALL,"loop1");
    int u = minDistance(dist, sptSet);
    sptSet[u] = true;
    for (int v = 0; v < V; v++){
//HLS_UNROLL_LOOP();
      if (!sptSet[v] && graph[u][v]&& dist[u] != INT_MAX&& dist[u] + graph[u][v] < dist[v])
        dist[v] = dist[u] + graph[u][v];
}
  }
  
  for(int i=0; i<V; i++){ 
      dist_result_dij.range(7,0)=i;
      dist_result_dij.range(15,8)=dist[i];
 #ifndef NATIVE_SYSTEMC
{ 
       HLS_DEFINE_PROTOCOL("output");
       o_dis_port.put(dist_result_dij);
       wait();
}       
 #else
       o_dis_port.write(dist_result_dij);
#endif  
 
  } 
 } 
}



private:
  sc_uint<WIDTH> _i_a;
};



