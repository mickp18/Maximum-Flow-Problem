fin = open("inputs/dag_edges.txt", "r")
fout = open("outputs/dag_edges_positive.txt", "w")

line = fin.readline()
fout.write(line)
for line in fin:
    vals = line.split()
    vals[0] = int(vals[0]) + 1
    vals[1] = int(vals[1]) + 1
    fout.write(str(vals[0]) + " " + str(vals[1]) + " " + vals[2] + "\n")


fin.close()
fout.close()