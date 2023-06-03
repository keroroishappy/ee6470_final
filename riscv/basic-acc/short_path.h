//#ifndef SOBEL_FILTER_H_
//#define SOBEL_FILTER_H_
#include <systemc>
#include <cmath>
#include <iomanip>
#include <limits>
#define V 9
const int WIDTH = 32;
using namespace sc_dt;
using namespace sc_core;

#include <tlm>
#include <tlm_utils/simple_target_socket.h>

#include "filter_def.h"

struct short_path : public sc_module {
  tlm_utils::simple_target_socket<short_path> tsock;

  sc_fifo<sc_uint<WIDTH> > i_a;
  sc_fifo<sc_uint<WIDTH> > i_b;
  sc_fifo<sc_uint<WIDTH> > i_c;
  sc_fifo<sc_uint<WIDTH> > i_src;
  sc_fifo<sc_uint<WIDTH> > o_dis;
  sc_fifo<sc_uint<WIDTH> > o_point;

  SC_HAS_PROCESS(short_path);

  short_path(sc_module_name n): 
    sc_module(n), 
    tsock("t_skt"), 
    base_offset(0)   
  {
    tsock.register_b_transport(this, &short_path::blocking_transport);
    SC_THREAD(addEdge);
    SC_THREAD(dijkstra);
  }
  ~short_path() {
	}

  int read_num=0;
  int graph[V][V];
  int src; 
  sc_uint<WIDTH> _i_a;
  sc_uint<WIDTH> _i_b;
  sc_uint<WIDTH> _i_c;
  sc_uint<WIDTH> _i_src;

  unsigned int base_offset;

  void addEdge(){
    for (int i = 0; i < V; i++){        
      for (int j = 0; j < V; j++){            
        graph[i][j] = 0;        
      }    
    } 

    while (true) {
      cout<<"-----------------------------------------"<<endl;

      _i_a = i_a.read();  
      _i_b = i_b.read();  
      _i_c = i_c.read();  
      _i_src = i_src.read();  

      int dis = _i_a; 
      cout<<"dis :"<< dis <<endl;  
      int y = _i_b;
      cout<<"y :"<< y <<endl; 
      int x = _i_c;
      cout<<"x :"<< x <<endl; 
      src = _i_src;
      graph[x][y] = dis; 
      graph[y][x] = dis;
      read_num = read_num + 1;
      cout<<"read_num = "<<read_num<<endl;
      cout<<"add time is "<<sc_time_stamp()<<endl;
      wait(1,SC_NS);
    }
  }
  
  int minDistance(int dist[], bool sptSet[]){
    int min = INT_MAX, min_index;
    for (int v = 0; v < V; v++){
      if (sptSet[v] == false && dist[v] <= min){
        min = dist[v];
        min_index = v;
      }
    }
    return min_index;
  }


void dijkstra(){  

    wait(2100000,SC_NS);
    cout<<"dij triggered"<<endl;
    cout<<"dij time is "<<sc_time_stamp()<<endl;
    cout<<"read_num : "<< read_num <<endl;
    cout<<"src : "<<src<<endl;

    if(read_num==14){
      cout<<"read_num=14 triggered"<<endl; 
      int dist[V]; 
      bool sptSet[V]; 
      for (int i = 0; i < V; i++)
          dist[i] = INT_MAX, sptSet[i] = false;
      dist[src] = 0;
      for (int count = 0; count < V - 1; count++) {
        int u = minDistance(dist, sptSet);
        sptSet[u] = true;
        for (int v = 0; v < V; v++)
        
          if (!sptSet[v] && graph[u][v]&& dist[u] != INT_MAX&& dist[u] + graph[u][v] < dist[v])
            dist[v] = dist[u] + graph[u][v];
      }
      
      for(int i=0; i<V; i++){ 
          o_point.write(i);
          o_dis.write(dist[i]);
          cout<<"i is "<<i;
          cout<<"     dist[i] is "<<dist[i]<<endl;
      } 
    } 
  }


  void blocking_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay){
    wait(delay);

    tlm::tlm_command cmd = payload.get_command();
    sc_dt::uint64 addr = payload.get_address();
    //printf("addr of addr is %p \n", &addr);
    unsigned char *data_ptr = payload.get_data_ptr();
    //printf("addr of data_ptr is %p \n", data_ptr);

    addr -= base_offset;

    // cout << (int)data_ptr[0] << endl;
     //cout << (int)data_ptr[1] << endl;
     //cout << (int)data_ptr[2] << endl;
     //cout << (int)data_ptr[3] << endl;

    union word buffer;
    int o_dis_t;
    int o_point_t;
    unsigned char o_dis_cast;
    unsigned char o_point_cast;

    switch (cmd) {
      case tlm::TLM_READ_COMMAND:
        // cout << "READ" << endl;
        switch (addr) {
          case SOBEL_FILTER_RESULT_ADDR:
            o_dis_t = o_dis.read();
            //cout<< "o_dis_t = " <<o_dis_t<<endl;
            o_point_t = o_point.read();
            //cout<< "o_point_t = " <<o_point_t<<endl;
            //cout<<endl;
            o_dis_cast = (unsigned char) o_dis_t;
            //cout<<"o_dis_cast = "<<(int)o_dis_cast<<endl;
            o_point_cast = (unsigned char) o_point_t;
            //cout<<"o_point_cast = "<<(int)o_point_cast<<endl;
            data_ptr[0] = o_dis_cast;
            data_ptr[1] = o_point_cast;
            //cout<<"data_ptr[0] = "<<(int)data_ptr[0]<<endl;
            //cout<<"data_ptr[1] = "<<(int)data_ptr[1]<<endl;
            cout<<"+++++++++++++++++++++++++++++++++++++++++"<<endl;
            break;

          default:
            std::cerr << "READ Error! SobelFilter::blocking_transport: address 0x"
                      << std::setfill('0') << std::setw(8) << std::hex << addr
                      << std::dec << " is not valid" << std::endl;
          }
        // data_ptr[0] = buffer.uc[0];
        // data_ptr[1] = buffer.uc[1];
         data_ptr[2] = buffer.uc[2];
         data_ptr[3] = buffer.uc[3];
        break;



      case tlm::TLM_WRITE_COMMAND:
        // cout << "WRITE" << endl;
        switch (addr) {
          case SOBEL_FILTER_R_ADDR:
            i_a.write(data_ptr[0]);
            i_b.write(data_ptr[1]);
            i_c.write(data_ptr[2]);
            i_src.write(data_ptr[3]);
            break;
          default:
            std::cerr << "WRITE Error! SobelFilter::blocking_transport: address 0x"
                      << std::setfill('0') << std::setw(8) << std::hex << addr
                      << std::dec << " is not valid" << std::endl;
        }
        break;
      case tlm::TLM_IGNORE_COMMAND:
        payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return;
      default:
        payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return;
      }
      payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
  }
};
//#endif
