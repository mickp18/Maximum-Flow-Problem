fin = open("inputs/airports.txt", "r")
fout = open("inputs/airports_500_dag.txt", "w")
# n1 n2
# 1 47 esistente
# 47 1 eliminare
# 50 1 tenere
''' { 1 : [47, 59, 68],
    3 : [4, 2, 467]}
'''
d = dict()
for line in fin:
    val = line.strip().split()
    # check if key in dictionary and if val in list of key 
    if not (val[1] in d and val[0] in d[val[1]]):
        fout.write(f"{int(val[0]) - 1} {int(val[1]) - 1} {int(val[2])}\n")        
        if (val[0] not in d):   
            d[val[0]] = []
        d[val[0]].append(val[1])


fin.close()
fout.close()