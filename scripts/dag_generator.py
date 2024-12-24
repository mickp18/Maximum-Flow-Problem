import random
import networkx as nx

def generate_dag(file_name, num_nodes, num_edges):
    """
    Generate a Directed Acyclic Graph (DAG) with a source and sink,
    and write its edges to a file in the format: "node1 node2 capacity".

    Args:
        file_name (str): Name of the output file.
        num_nodes (int): Number of nodes in the graph.
        num_edges (int): Number of edges in the graph.
    """
    if num_edges > (num_nodes * (num_nodes - 1)) // 2:
        raise ValueError("Too many edges for the given number of nodes in a DAG.")
    
    # Create a list of all possible edges (u, v) where u < v to avoid cycles
    possible_edges = [(u, v) for u in range(num_nodes) for v in range(u + 1, num_nodes)]

    # Randomly select the desired number of edges
    edges = random.sample(possible_edges, num_edges)

    # Assign random capacities to edges
    edges_with_capacity = [(u, v, random.randint(1, 20)) for u, v in edges]

    # Add edges from source (0) and to sink (num_nodes-1) if needed
    source = 0
    sink = num_nodes - 1

    while not any(u == source for u, _, _ in edges_with_capacity):
        v = random.choice(range(1, num_nodes))
        edges_with_capacity.append((source, v, random.randint(1, 20)))

    while not any(v == sink for _, v, _ in edges_with_capacity):
        u = random.choice(range(0, num_nodes - 1))
        edges_with_capacity.append((u, sink, random.randint(1, 20)))

    # Validate DAG using NetworkX
    G = nx.DiGraph()
    G.add_weighted_edges_from(edges_with_capacity)
    
    # Remove edges causing cycles
    if not nx.is_directed_acyclic_graph(G):
        edges_with_capacity = [(u, v, w) for u, v, w in edges_with_capacity if u < v]
        G = nx.DiGraph()
        G.add_weighted_edges_from(edges_with_capacity)

    if not nx.is_directed_acyclic_graph(G):
        raise RuntimeError("Failed to generate a valid DAG. Try different parameters.")

    # Write the edges to the file
    with open(file_name, "w") as f:
        f.write(f"{num_nodes}\n")
        for u, v, cap in edges_with_capacity:
            f.write(f"{u} {v} {cap}\n")

    print(f"DAG with {num_nodes} nodes and {len(edges_with_capacity)} edges saved to '{file_name}'.")
    print(f"Is DAG: {nx.is_directed_acyclic_graph(G)}")

# Example usage
if __name__ == "__main__":
    num_nodes = 1000  # Adjust number of nodes
    num_edges = 15000  # Adjust number of edges
    generate_dag("dag_edges.txt", num_nodes, num_edges)