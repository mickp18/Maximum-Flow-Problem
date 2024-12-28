# Maximum-Flow-Problem
Sequential version
use adjacency list and BFS


command to run in the Terminal:
./scripts/run.sh airports_500_dag.txt result.txt

If the previous command gives `/bin/bash^M: bad interpreter: No such file or directory`
run the command in this [link](https://stackoverflow.com/a/29747593)


(Do the following from VS code running in WSL)
To install graphviz you need to run the following commands:
```bash
sudo apt install grapviz
```
In case of error related to the library `graphviz/gvc.h`, run the following commands:
```bash
sudo apt-get update
```
```bash
sudo apt-get install graphviz-dev
```

TO install the Python library
- networkx:
```bash
python3 -m pip install -U networkx
```
- matplotlib:
<!-- `python3 -m pip install -U pip` -->

```bash
python3 -m pip install -U matplotlib
```