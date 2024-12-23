import networkx as nx
import matplotlib.pyplot as plt
from networkx.drawing.nx_agraph import to_agraph

# Determine if input graph is cyclic

def graph_from_file(filename):
    G = nx.DiGraph()
    with open(filename) as f:
        line = f.readline()
        for line in f:
            node1, node2, weight = line.split()
            G.add_edge(node1, node2, capacity=int(weight))
    return G

def remove_cycles(G):
    while True:
        try:
            # Find the first cycle in the graph
            cycle = next(nx.simple_cycles(G))
         #   print(f"Cycle found: {cycle}")
            
            # Remove one edge from the cycle to break it
            edge_to_remove = (cycle[0], cycle[1])
            G.remove_edge(*edge_to_remove)
           # print(f"Removed edge: {edge_to_remove}")
        except StopIteration:
            # No more cycles found
            break
    return G

def main():
    fout = open("inputs/airport_1500_dag.txt", "w")
 #   fout = open("dag_edges_out.txt", "w")
    # G = graph_from_file("inputs/dag_edges.txt")
    G = graph_from_file("inputs/airports_1500_directed.txt")
  
   
    is_dag = nx.is_directed_acyclic_graph(G)
    print(is_dag)
    if (not is_dag):
      #  print(nx.find_cycle(G))
        G2 = remove_cycles(G)
        # print(nx.is_directed_acyclic_graph(G2))
    
        # print("New graph:")
        #print(G2.edges())
        fout.write(f"{G2.number_of_nodes()}\n")
        for edge in G2.edges():
             fout.write(f"{edge[0]} {edge[1]} {G2[edge[0]][edge[1]]['capacity']}\n")

        fout.close()

   # print(G.number_of_nodes())
    # print("maximum flow " + str(nx.maximum_flow(G,"0","999")[0]))
   # draw(G)
    
def draw(G):
    # plt.figure(figsize=(100, 60))  # Set the figure size
    # pos = nx.spring_layout(G)  # Use spring layout for node positioning
    # pos = nx.shell_layout(G)  # Use spring layout for node positioning
    # pos = nx.circular_layout(G)  # Use spring layout for node positioning
    # nx.draw(
    #     G, pos, with_labels=True, node_color='skyblue', edge_color='black',
    #     node_size=2000, font_size=100, font_color='darkblue'
    # )

    # plt.title("Directed Graph", fontsize=16)
    # plt.savefig("outputs/directed_graph.png", format="PNG")  # Save as PNG
    # plt.show()

    
    
    layouts = []

  #  layouts.append(nx.arf_layout(G))
  #  layouts.append(nx.bipartite_layout(G, "1"))
    # layouts.append(nx.bfs_layout(G,"1"))
    # layouts.append(nx.circular_layout(G))
    # layouts.append(nx.forceatlas2_layout(G))
    # layouts.append(nx.kamada_kawai_layout(G))
    # layouts.append(nx.planar_layout(G))
    # layouts.append(nx.random_layout(G))
    # layouts.append(nx.rescale_layout(G))
    # layouts.append(nx.rescale_layout_dict(G))
    shells = [["0"], [1, 2], [3], [4, 5], ["499"]]  # Source (0), intermediate layers, sink (6)
    # layouts.append(nx.shell_layout(G, nlist=shells))
   # layouts.append(nx.spring_layout(G,  k=1.5, iterations=100))
    # layouts.append(nx.spectral_layout(G))
    # layouts.append(nx.spiral_layout(G))
    # layouts.append(nx.multipartite_layout(G))
    i = 0
    for layout in layouts:
        plt.figure(figsize=(100, 60))
        nx.draw(
            G, layout, with_labels=True, node_color='skyblue', edge_color='black',
            node_size=3500, font_size=50, font_color='darkblue'
        )
        # Add edge labels
        # edge_labels = nx.get_edge_attributes(G, 'capacity')  # Get edge capacities as labels
        # nx.draw_networkx_edge_labels(G, layout, edge_labels=edge_labels, font_color='red')

        plt.title(f"Directed Graph {i}", fontsize=16)
        plt.savefig(f"outputs/graph{i}.png", format="PNG")  # Save as PNG
        i+=1

    A = to_agraph(G)
    A.layout('dot')  # Hierarchical layout
    A.draw("flow_graph_dot_layout.png")
    
main()


'''
arf_layout(G[, pos, scaling, a, etol, dt, ...]) - Arf layout for networkx
bipartite_layout(G, nodes[, align, scale, ...]) - Position nodes in two straight lines.
bfs_layout(G, start, *[, align, scale, center]) - Position nodes according to breadth-first search algorithm.
circular_layout(G[, scale, center, dim]) - Position nodes on a circle.
forceatlas2_layout(G[, pos, max_iter, ...]) - Position nodes using the ForceAtlas2 force-directed layout algorithm.
kamada_kawai_layout(G[, dist, pos, weight, ...]) - position nodes using Kamada-Kawai path-length cost-function.
planar_layout(G[, scale, center, dim]) - Position nodes without edge intersections.
random_layout(G[, center, dim, seed]) - Position nodes uniformly at random in the unit square.
rescale_layout(pos[, scale]) - Returns scaled position array to (-scale, scale) in all axes.
rescale_layout_dict(pos[, scale]) - Return a dictionary of scaled positions keyed by node
shell_layout(G[, nlist, rotate, scale, ...]) - Position nodes in concentric circles.
spring_layout(G[, k, pos, fixed, ...]) - Position nodes using Fruchterman-Reingold force-directed algorithm.
spectral_layout(G[, weight, scale, center, dim]) - Position nodes using the eigenvectors of the graph Laplacian.
spiral_layout(G[, scale, center, dim, ...]) - Position nodes in a spiral layout.
multipartite_layout(G[, subset_key, align, ...]) - Position nodes in layers of straight lines.
'''