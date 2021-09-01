#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

#define OPEN

FILE *fp;

double total_in = 0;
double total_out = 0;

//Structure to hold a node information
struct Node
{
  int starting;
  int no_of_edges;
};

void BFSGraph(int argc, char** argv);

void Usage(int argc, char**argv){
  fprintf(stderr,"Usage: %s <input_file> <output_file>\n", argv[0]);
}
////////////////////////////////////////////////////////////////////////////////
// Main Program
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char** argv) 
{
  BFSGraph( argc, argv);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
//Apply BFS on a Graph using CUDA
////////////////////////////////////////////////////////////////////////////////
void BFSGraph( int argc, char** argv) 
{
  int no_of_nodes = 0;

  int edge_list_size = 0;
  char *input_f;
  char *output_f;

  if(argc!=3) {
    Usage(argc, argv);
    exit(0);
  }

  double runtime;
  struct timeval t1, t2;
  //gettimeofday(&t1, NULL);

  input_f = argv[1];
  output_f = argv[2];

  printf("Reading File\n");
  //Read in Graph from a file
  fp = fopen(input_f,"r");
  if(!fp) {
    printf("Error Reading graph file\n");
    return;
  }

  FILE *fp_out = fopen(output_f, "w");
  if(!fp_out) {
    printf("Error creating output file\n");
    return;
  }
  int source = 0;

  fscanf(fp,"%d", &no_of_nodes);
  printf("Total number of nodes = %d\n", no_of_nodes);

  // allocate host memory
  Node h_graph_nodes[no_of_nodes];
  bool h_graph_mask[no_of_nodes];
  bool h_updating_graph_mask[no_of_nodes];
  bool h_graph_visited[no_of_nodes];

  int start, edgeno;   
  // initalize the memory
  int max_edge = 0;
  for( unsigned int i = 0; i < no_of_nodes; i++) {
    fscanf(fp,"%d %d", &start, &edgeno);

    h_graph_nodes[i].starting = start;
    h_graph_nodes[i].no_of_edges = edgeno;
    if(edgeno > max_edge) max_edge = edgeno;
    h_graph_mask[i] = false;
    h_updating_graph_mask[i] = false;
    h_graph_visited[i] = false;
  }

  //read the source node from the file
  fscanf(fp,"%d",&source);

  //set the source node as true in the mask
  h_graph_mask[source] = true;
  h_graph_visited[source] = true;

  fscanf(fp,"%d",&edge_list_size);

  int id,cost;
  int h_graph_edges[edge_list_size];
  for(int i=0; i < edge_list_size ; i++) {
    fscanf(fp,"%d",&id);
    fscanf(fp,"%d",&cost);
    h_graph_edges[i] = id;
  }

  if(fp) fclose(fp);    

  // allocate mem for the result on host side
  int h_cost[no_of_nodes];
  for(int i=0;i<no_of_nodes;i++)
    h_cost[i]=-1;
  h_cost[source]=0;

  int num_dev = omp_get_num_devices(); 

  printf("Start traversing the tree on %d devices\n", num_dev);
  printf("kernel,Outer,Inner,Iter,VarDecl,refExpr,intLiteral,floatLiteral,mem_to,mem_from,add_sub_int,add_sub_double,mul_int,mul_double,div_double,assign_int,assign_double,runtime1,runtime\n");
  int dev = 0;
//#pragma omp parallel for private(runtime, t1, t2, total_out, total_in)
//  for(int dev = 0; dev < num_dev; dev++)
  {
    total_in += sizeof(int) + sizeof(bool)*no_of_nodes + sizeof(Node)*no_of_nodes + sizeof(int)*edge_list_size + sizeof(bool)*no_of_nodes + sizeof(bool)*no_of_nodes + sizeof(int)*no_of_nodes;
    total_out += sizeof(int)*no_of_nodes;
  printf("Total data transfered = %.3lf\n", (total_in + total_out) / 1024 / 1024);
//#pragma omp target enter data map(to: no_of_nodes, h_graph_mask[0:no_of_nodes], h_graph_nodes[0:no_of_nodes], h_graph_edges[0:edge_list_size], h_graph_visited[0:no_of_nodes], h_updating_graph_mask[0:no_of_nodes], h_cost[0:no_of_nodes]) device(dev)
    {
      bool stop;
      do
      {
        //if no thread changes this value then the loop stops
        stop=false;
        int in = sizeof(bool)*no_of_nodes + sizeof(Node)*no_of_nodes +  sizeof(int) * edge_list_size + sizeof(bool) * no_of_nodes + sizeof(bool) * no_of_nodes + sizeof(int)*no_of_nodes;
        int out = sizeof(bool)*no_of_nodes + sizeof(Node)*no_of_nodes +  sizeof(int) * edge_list_size + sizeof(bool) * no_of_nodes + sizeof(bool) * no_of_nodes + sizeof(int)*no_of_nodes;
        gettimeofday(&t1, NULL);
#pragma omp target teams distribute parallel for device(dev) map(h_graph_mask[0:no_of_nodes], h_graph_nodes[0:no_of_nodes], h_graph_edges[0:edge_list_size], h_graph_visited[0:no_of_nodes], h_updating_graph_mask[0:no_of_nodes], h_cost[0:no_of_nodes])
        for(int tid = 0; tid < no_of_nodes; tid++ ) {
          if (h_graph_mask[tid] == true) { 
            h_graph_mask[tid]=false;
            for(int i = h_graph_nodes[tid].starting; i < (h_graph_nodes[tid].no_of_edges + h_graph_nodes[tid].starting); i++) {
              int id = h_graph_edges[i];
              if(!h_graph_visited[id]) {
                h_cost[id] = h_cost[tid] + 1;
                h_updating_graph_mask[id] = true;
              }
            }
          }
        }
        gettimeofday(&t2, NULL);
        runtime = (t2.tv_sec - t1.tv_sec) * 1000000;
        runtime += (t2.tv_usec - t1.tv_usec);
#pragma omp critical
        fprintf(fp_out, "Kernel1_%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.0lf,%.6lf\n", dev, no_of_nodes, 0, 0, max_edge+1, 10*max_edge+6, max_edge+2, 0,in,out, max_edge+2, 0,0,0,0,0, max_edge, 0, runtime, runtime / 1000000.0);

        gettimeofday(&t1, NULL);
        in = sizeof(bool)*no_of_nodes + sizeof(bool)*no_of_nodes + sizeof(bool)*no_of_nodes + sizeof(bool);
        out = sizeof(bool)*no_of_nodes + sizeof(bool)*no_of_nodes + sizeof(bool)*no_of_nodes + sizeof(bool);
#pragma omp target teams distribute parallel for device(dev) map(h_updating_graph_mask[0:no_of_nodes],h_graph_mask[0:no_of_nodes], h_graph_visited[0:no_of_nodes], stop)
        for(int tid=0; tid< no_of_nodes ; tid++ ) {
          if (h_updating_graph_mask[tid] == true){
            h_graph_mask[tid]=true;
            h_graph_visited[tid]=true;
            stop=true;
            h_updating_graph_mask[tid]=false;
          }
        }
        gettimeofday(&t2, NULL);
        runtime = (t2.tv_sec - t1.tv_sec) * 1000000;
        runtime += (t2.tv_usec - t1.tv_usec);
#pragma omp critical
        fprintf(fp_out, "Kernel2_%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.0lf,%.6lf\n", dev, no_of_nodes, 0, 0,0,9,5,0,in,out,0,0,0,0,0,0,4,0, runtime, runtime / 1000000.0);

      } while(stop);

    }
#pragma omp target exit data map(from: h_cost[0:no_of_nodes]) device(dev)
  }
  fclose(fp_out);

}

