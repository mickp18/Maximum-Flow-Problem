#include <graphviz/gvc.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

struct Edge
{
    string from;
    string to;
    int weight;
};

// Function to read graph edges from file
vector<Edge> readGraphFromFile(const string &filename)
{
    vector<Edge> edges;
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Unable to open file!" << endl;
        exit(1);
    }

    int from, to, weight;
    while (file >> from >> to >> weight)
    {
        edges.push_back({to_string(from), to_string(to), weight});
    }

    file.close();
    return edges;
}

// Function to create and draw the graph using Graphviz
void drawGraph(const vector<Edge> &edges, const string &outputFile)
{
    // Initialize Graphviz context
    GVC_t *gvc = gvContext();
    Agraph_t *graph = agopen(const_cast<char *>("G"), Agdirected, nullptr);

    // Set graph attribute for horizontal layout
    agsafeset(graph, const_cast<char*>("rankdir"), const_cast<char*>("LR"), "");

    // Add nodes and edges
    for (const auto &edge : edges)
    {
        Agnode_t *node1 = agnode(graph, const_cast<char *>(edge.from.c_str()), 1);
        Agnode_t *node2 = agnode(graph, const_cast<char *>(edge.to.c_str()), 1);

        Agedge_t *graphEdge = agedge(graph, node1, node2, nullptr, 1);
        agsafeset(graphEdge, const_cast<char *>("label"), const_cast<char *>(to_string(edge.weight).c_str()), "");
    }

    // Render graph
    gvLayout(gvc, graph, "dot"); // Use "dot" layout
    gvRenderFilename(gvc, graph, "png", outputFile.c_str());

    // Free resources
    gvFreeLayout(gvc, graph);
    agclose(graph);
    gvFreeContext(gvc);
}

int main()
{
    string inputFile = "result.txt";  // Input graph data
    string outputFile = "graph.png"; // Output image file

    // Read graph edges from file
    vector<Edge> edges = readGraphFromFile(inputFile);

    // Draw graph and save as PNG
    drawGraph(edges, outputFile);

    cout << "Graph saved to " << outputFile << endl;
    return 0;
}
