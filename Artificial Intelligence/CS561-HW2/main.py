from collections import defaultdict
import heapq, sys

""" GRAPH """ 
class Node:
    graph = {}
    def __init__(self, name, cost, parent):
        self.name = name
        self.cost = cost
        self.utility = cost
        self.ancestor_names = parent.ancestor_names[:] + [parent.name] if parent else []

    def children(self):
        for name, cost in Node.graph[self.name].iteritems():
            if name not in self.ancestor_names:
                yield Node(name, self.cost + cost, self)

    def __lt__(self, other):
        return self.utility < other.utility

    def path(self):
        return '-'.join(self.ancestor_names + [self.name])

""" SEARCH STRATEGIES """
def greedy(graph, start, goal, heuristics):
    node = Node(start, 0, None)
    node.utility = heuristics[node.name]

    Node.graph = graph
    h = [node]
    while h:
        node = heapq.heappop(h)
        if node.name == goal:
            return node
        for child in node.children():
            child.utility = heuristics[child.name]
            heapq.heappush(h, child)

def astar(graph, start, goal, heuristics):
    node = Node(start, 0, None)
    node.utility += heuristics[node.name]

    Node.graph = graph
    h = [node]
    while h:
        node = heapq.heappop(h)
        if node.name == goal:
            return node
        for child in node.children():
            child.utility += heuristics[child.name]
            heapq.heappush(h, child)

""" MAIN PROGRAM """
try:
    _, graph_file, heuristics_file, start, goal = sys.argv
except:
    print 'python main.py <graph> <heuristics> <start> <goal>'
    sys.exit(0)

time_graph = defaultdict(lambda: defaultdict(float))
risk_graph = defaultdict(lambda: defaultdict(float))
for line in open(graph_file):
    x, y, time, risk = line.split()
    time_graph[x][y] = time_graph[y][x] = float(time)
    risk_graph[x][y] = risk_graph[y][x] = float(risk)

time_heuristics = defaultdict(float)
risk_heuristics = defaultdict(float)
for line in open(heuristics_file):
    x, time, risk = line.split()
    time_heuristics[x] = float(time)
    risk_heuristics[x] = float(risk)

outputs = [
    ('A-star.risk.result.txt', astar(risk_graph,  start, goal, risk_heuristics)),
    ('A-star.time.result.txt', astar(time_graph,  start, goal, time_heuristics)),
    ('Greedy.risk.result.txt', greedy(risk_graph, start, goal, risk_heuristics)),
    ('Greedy.time.result.txt', greedy(time_graph, start, goal, time_heuristics))
]

for filename, result in outputs:
    fp = open(filename, 'w')
    print >> fp, result.path()
    fp.close()
