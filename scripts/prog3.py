fin = open("inputs/EIA_LiqPipProject_data.csv", "r")
fout = open("outputs/pipeline_dataset.txt", "w")
headers = fin.readline().strip().split(";")
headers[0] = headers[0].lstrip("\ufeff")
lst = []
for line in fin:
    d = dict()
    # print(line)
    vals = line.strip().split(";")
    for i in range(len(headers)):
        d[headers[i]] = vals[i]
    lst.append(d)

for i in range(len(lst)):
    print(lst[i])

# fout.write("State(s);Beg_State;End_State;Beg_Region;End_Region;Miles;Added Capacity;Interstate;Flow Direction\n")
fout.write("ID;State(s);Beg_Region;End_Region;Added Capacity\n")
for d in lst:
    # fout.write(d["State(s)"] + ";" + d["Beg_State"] + ";" + d["End_State"] + ";" + d["Beg_Region"] + ";" + d["End_Region"] + ";" +
    #            d["Miles"] + ";" + d["Added Capacity"] + ";" + d["Interstate"] + ";" + d["Flow Direction"] + "\n")
    fout.write(d["ID"] + ";" + d["State(s)"] + ";" + d["Beg_Region"] + ";" + d["End_Region"] + ";" + d["Added Capacity"] +";" + d["Flow Direction"] + "\n")

fin.close()
fout.close()