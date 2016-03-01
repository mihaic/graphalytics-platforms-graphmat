#include "GraphMatRuntime.cpp"
#include "common.hpp"

#include <limits>
#include <omp.h>
#include <stdint.h>
#include <algorithm>
#include <iostream>

#ifdef GRANULA
#include "granula.hpp"
#endif

using namespace std;

typedef double depth_type;
typedef depth_type msg_type;
typedef depth_type reduce_type;
typedef depth_type edge_value_type;

struct vertex_value_type {
    public:
        depth_type curr;

        vertex_value_type() {
            curr = numeric_limits<depth_type>::max();
        }

        vertex_value_type(depth_type d) {
            curr = d;
        }

        bool operator!= (const vertex_value_type& other) const {
            return !(curr == other.curr);
        }

        friend ostream& operator<< (ostream& stream, const vertex_value_type &v) {
            if (v.curr != numeric_limits<depth_type>::max()) {
                stream << v.curr;
            } else {
                stream << "inf";
            }

            return stream;
        }

        depth_type get_output() {
            return curr;
        }
};

class SingleSourceShortestPath: public GraphProgram<msg_type, reduce_type, vertex_value_type, edge_value_type> {
    public:
        bool send_message(const vertex_value_type& vertex, msg_type& msg) const {
            msg = vertex.curr;
            return true;
        }

        void reduce_function(reduce_type& total, const reduce_type& partial) const {
            total = min(total, partial);
        }

        void process_message(const msg_type& msg, const edge_value_type edge_value, const vertex_value_type& vertex, reduce_type& result) const {
            result = msg + edge_value;
        }

        void apply(const reduce_type& msg, vertex_value_type& vertex) {
            vertex.curr = min(vertex.curr, msg);
        }

};


int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    GraphPad::GB_Init();
    if (argc < 3) {
        cerr << "usage: " << argv[0] << " <graph file> <source vertex> [output file]" << endl;
        return EXIT_FAILURE;
    }

    bool is_master = GraphPad::global_myrank == 0;
    char *filename = argv[1];
    int source_vertex = atoi(argv[2]) - 1;
    char *output = argc > 3 ? argv[3] : NULL;

    if (is_master) cout << "source vertex: " << source_vertex << endl;

    nthreads = omp_get_max_threads();
    if (is_master) cout << "num. threads: " << nthreads << endl;

#ifdef GRANULA
    granula::operation graphmatJob("GraphMat", "Id.Unique", "Job", "Id.Unique");
    if (is_master) cout<<graphmatJob.getOperationInfo("StartTime", graphmatJob.getEpoch())<<endl;

    granula::operation loadGraph("GraphMat", "Id.Unique", "LoadGraph", "Id.Unique");
    if (is_master) cout<<loadGraph.getOperationInfo("StartTime", loadGraph.getEpoch())<<endl;
#endif

    timer_start(is_master);

    timer_next("load graph");
    Graph<vertex_value_type, edge_value_type> graph;
    graph.ReadMTX(filename, nthreads * 4);

#ifdef GRANULA
    if (is_master) cout<<loadGraph.getOperationInfo("EndTime", loadGraph.getEpoch())<<endl;
#endif

    timer_next("initialize engine");
    if (source_vertex < 0 || source_vertex >= graph.nvertices) {
        cerr << "ERROR: invalid source vertex (not in range [0, " << graph.nvertices-1 << "]" << endl;
        return EXIT_FAILURE;
    }

    graph.setAllInactive();
    graph.setVertexproperty(source_vertex, vertex_value_type(0));
    graph.setActive(source_vertex);

    SingleSourceShortestPath prog;
    auto ctx = graph_program_init(prog, graph);

#ifdef GRANULA
    granula::operation processGraph("GraphMat", "Id.Unique", "ProcessGraph", "Id.Unique");
    if (is_master) cout<<processGraph.getOperationInfo("StartTime", processGraph.getEpoch())<<endl;
#endif

    timer_next("run algorithm");
    run_graph_program(&prog, graph, -1);

#ifdef GRANULA
    if (is_master) cout<<processGraph.getOperationInfo("EndTime", processGraph.getEpoch())<<endl;
#endif

#ifdef GRANULA
    granula::operation offloadGraph("GraphMat", "Id.Unique", "OffloadGraph", "Id.Unique");
    if (is_master) cout<<offloadGraph.getOperationInfo("StartTime", processGraph.getEpoch())<<endl;
#endif

    timer_next("print output");
    print_graph<vertex_value_type, edge_value_type, depth_type>(output, graph, MPI_DOUBLE);

#ifdef GRANULA
    if (is_master) cout<<offloadGraph.getOperationInfo("EndTime", processGraph.getEpoch())<<endl;
#endif


    timer_next("deinitialize engine");
    graph_program_clear(ctx);

    timer_end();

#ifdef GRANULA
    if (is_master) cout<<graphmatJob.getOperationInfo("EndTime", graphmatJob.getEpoch())<<endl;
#endif

    MPI_Finalize();
    return EXIT_SUCCESS;
}
