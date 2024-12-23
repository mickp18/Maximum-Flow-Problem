import networkx as nx
import matplotlib.pyplot as plt

# Determine if input graph is cyclic

def graph_from_file(filename):
    G = nx.DiGraph()
    with open(filename) as f:
        for line in f:
            node1, node2, weight = line.split()
            G.add_edge(node1, node2, capacity=int(weight))
    return G

def remove_cycles(G):
    while True:
        try:
            # Find the first cycle in the graph
            cycle = next(nx.simple_cycles(G))
           # print(f"Cycle found: {cycle}")
            
            # Remove one edge from the cycle to break it
            edge_to_remove = (cycle[0], cycle[1])
            G.remove_edge(*edge_to_remove)
           # print(f"Removed edge: {edge_to_remove}")
        except StopIteration:
            # No more cycles found
            break
    return G

def main():
    G = graph_from_file("inputs/airports_500.txt")
    # print(nx.is_directed_acyclic_graph(G))
    #print(nx.find_cycle(G))
   
    # print(next(nx.simple_cycles(G)))
#    print(nx.is_directed_acyclic_graph(G))
   # G2 = remove_cycles(G)
    #print(nx.is_directed_acyclic_graph(G2))
    
 #   print(G.edges())
   # print(G.number_of_nodes())
    print(nx.maximum_flow(G,"1","500")[0])
    draw(G)

def draw(G):
    plt.figure(figsize=(150, 80))  # Set the figure size
    pos = nx.spring_layout(G)  # Use spring layout for node positioning
    nx.draw(
        G, pos, with_labels=True, node_color='skyblue', edge_color='black',
        node_size=2000, font_size=100, font_color='darkblue'
    )

    plt.title("Directed Graph", fontsize=16)
    plt.savefig("outputs/directed_graph.png", format="PNG")  # Save as PNG
    plt.show()

main()
