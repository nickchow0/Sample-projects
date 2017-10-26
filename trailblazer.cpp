// Nick Chow
// CS106B HW7 - Trailblazer
// This file contains four algorithms to find a path between 2 given vertexes
// and a Kruskal's algorithm to find a minimum spanning tree
// The 4 algorithms are depth first search, breath first search, Dijkstra, and A Star

#include "trailblazer.h"
#include "basicgraph.h"
#include "queue.h"
#include "pqueue.h"
#include <algorithm>
#include "vector.h"
#include "set.h"

using namespace std;

bool depthFirstSearchHelper(BasicGraph& graph, Vertex* start, Vertex* end, Vector<Vertex *> &path);
bool sameCluster (Vector<Set<Vertex *> > clusterHolder, Edge* edge);

// this is a Depth First Search method
Vector<Vertex*> depthFirstSearch(BasicGraph& graph, Vertex* start, Vertex* end) {
    graph.resetData(); // reset graph data
    Vector<Vertex*> path;
    depthFirstSearchHelper(graph, start, end, path); // call recursive helper method
    return path;
}

// this is a recursive helper method for DFS
bool depthFirstSearchHelper(BasicGraph& graph, Vertex* current, Vertex* end, Vector<Vertex*>& path) {
    path.add(current); // add current vertex to path
    current->visited = true;
    current->setColor(GREEN);
    if (current == end) {
        return true; // base case - if path is found
    }
    for (Edge* edge: current->edges) { // for each edge of the current vertex
        // visit each unvisited vertex and path is found, return true
        if (!edge->finish->visited
                && depthFirstSearchHelper(graph, edge->finish, end, path)) {
            return true;
        }
    }
    // path not found
    path.remove(path.size()-1); // remove current vertex
    current->setColor(GRAY); // mark incorrect path gray
    return false;
}

// this is a Breadth First Search method
Vector<Vertex*> breadthFirstSearch(BasicGraph& graph, Vertex* start, Vertex* end) {
    graph.resetData(); // reset graph data
    Vector<Vertex*> path;
    Queue<Vertex*> queue;
    queue.enqueue(start); // enqueue start vertex
    start->visited = true;
    start->setColor(YELLOW);
    while (!queue.isEmpty()) { // while the queue is not empty
        Vertex* current = queue.dequeue();
        current->visited = true;
        current->setColor(GREEN);
        if (current == end) {
            // path is found, reconstruct path back to start
            while (current != start) {
                path.add(current); // add each vertex to path
                if (current->name == start->name) { // reached beginning of path
                    break;
                }
                current = current->previous; // set current vertex to the previous vertex
            }
            path.add(current); // add the first vertex
            reverse(path.begin(), path.end()); // reverse the order of the vector
            return path;
        }
        for (Edge* edge: current->edges) { // for each edge of the current vertex
            Vertex* neighbors = edge->finish;
            // visit each unvisited vertex
            if (!neighbors->visited) {
                neighbors->setColor(YELLOW);
                neighbors->visited = true;
                neighbors->previous = current; // set previous vertex as current
                queue.enqueue(neighbors); // enqueue each neighbor vertexes
            }
        }
    }
    // path not found
    while (!path.isEmpty()) {
        path[path.size()-1]->setColor(GRAY); // mark incorrect path gray
        path.remove(path.size()-1); // remove current vertex
    }
    return path;
}

// this is Dijkstra's Algorithm to find the cost optimal path
Vector<Vertex*> dijkstrasAlgorithm(BasicGraph& graph, Vertex* start, Vertex* end) {
    graph.resetData(); // reset graph data
    Vector<Vertex*> path;
    for (Vertex* vertex: graph) {
        vertex->cost = POSITIVE_INFINITY; // initialize every node to have cost infinity
        vertex->previous = NULL; // initialize previous of each vertex to NULL
    }
    start->cost = 0.0; // initialize start cost as 0.0
    PriorityQueue<Vertex*> pqueue;
    pqueue.enqueue(start, 0.0); // enqueue start vertex with priority 0.0
    Vertex* current;
    while (!pqueue.isEmpty()) { // while pqueue is not empty
        current = pqueue.dequeue(); // dequeue and set as current vertex
        current->visited = true;
        current->setColor(GREEN);
        if (current == end) { // path is found
            break;
        }
        for (Edge* edge: current->edges) { // for each edge for the current vertex
            Vertex* next = edge->finish;
            if (!edge->finish->visited) { // visit all unvisited neighbors
                double cost = edge->cost + current->cost; // initialize cost to edge cost + cost of current vertex
                if (cost < next->cost) {  // if new cost is less than next vertex's cost, else do nothing
                    next->cost = cost; // set new cost
                    next->previous = current; // set current vertex as previous
                    if (next->getColor() == YELLOW) { // if next is already in pqueue
                        pqueue.changePriority(next, next->cost); // change prioirty to new cost
                    } else { // if next is not already in pqueue
                        pqueue.enqueue(next, next->cost); // enqueue with cost as priority
                        next->setColor(YELLOW);
                    }
                }
            }
        }
    }
    if (!end->visited) { // if did not reach end vertex
        return path; // return empty path
    }
    // path is found, reconstruct path back to start
    while (end != start) {
        path.add(end); // add each vertex to path
        if (end->name == start->name) { // reached beginning of path
            break;
        }
        end = end->previous;
    }
    path.add(end);
    reverse(path.begin(), path.end()); // reverse the order of the vector
    return path;
}

// this is Dijkstra's Algorithm to find the cost optimal path with heuristic cost accounted for
Vector<Vertex*> aStar(BasicGraph& graph, Vertex* start, Vertex* end) {
    graph.resetData(); // reset graph data
    Vector<Vertex*> path;
    for (Vertex* vertex: graph) {
        vertex->cost = POSITIVE_INFINITY; // initialize every node to have cost infinity
        vertex->previous = NULL; // initialize previous of each vertex to NULL
    }
    start->cost = 0.0; // initialize start cost as 0.0
    PriorityQueue<Vertex*> pqueue;
    pqueue.enqueue(start, 0.0); // enqueue start vertex with priority 0.0
    Vertex* current;
    while (!pqueue.isEmpty()) { // while pqueue is not empty
        current = pqueue.dequeue(); // dequeue and set as current vertex
        current->visited = true;
        current->setColor(GREEN);
        if (current == end) { // path is found
            break;
        }
        for (Edge* edge: current->edges) { // for each edge for the current vertex
            Vertex* next = edge->finish;
            if (!edge->finish->visited) { // visit all unvisited neighbors
                double cost = edge->cost + current->cost; // initialize cost to edge cost + cost of current vertex
                if (cost < next->cost) {  // if new cost is less than next vertex's cost, else do nothing
                    next->cost = cost; // set new cost
                    next->previous = current; // set current vertex as previous
                    if (next->getColor() == YELLOW) { // if next is already in pqueue
                        pqueue.changePriority(next, next->cost + heuristicFunction(next, end)); // change prioirty to new cost + heuristic cost
                    } else { // if next is not already in pqueue
                        pqueue.enqueue(next, next->cost + heuristicFunction(next, end)); // enqueue with cost as priority + heuristic cost
                        next->setColor(YELLOW);
                    }
                }
            }
        }
    }
    if (!end->visited) { // if did not reach end vertex
        return path; // return empty path
    }
    // path is found, reconstruct path back to start
    while (end != start) {
        path.add(end); // add each vertex to path
        if (end->name == start->name) { // reached beginning of path
            break;
        }
        end = end->previous;
    }
    path.add(end);
    reverse(path.begin(), path.end()); // reverse the order of the vector
    return path;
}

// This method creates a minimum spanning tree from a given graph
Set<Edge*> kruskal(BasicGraph& graph) {
    Set<Edge*> mst;
    PriorityQueue<Edge*> kruskalPQ;
    Vector<Set<Vertex*>> clusterHolder; // initialize a Vector to hold all the clusters
    for (Vertex* vertex: graph.getVertexSet()) { // for each vertex in the graph
        Set<Vertex*> cluster;
        cluster.add(vertex); // add vertex to the cluster
        clusterHolder.add(cluster); // add cluster to the clusterHolder
    }
    for (Edge* edge: graph.getEdgeSet()) { // for each edge in the graph
        kruskalPQ.enqueue(edge, edge->cost); // enqueue into the PQ with the edge cost as priority
    }
    while (clusterHolder.size() > 1) { // while there are more than one clusters
        Edge* edge = kruskalPQ.dequeue(); // dequeue an edge from the PQ
        if (!sameCluster(clusterHolder, edge)) { // if not in the same cluster
            int startSetIndex = 0;
            int endSetIndex = 0;
            Vertex* start = edge->start;
            Vertex* end = edge->end;
            // find which clusters the vertexes belong to
            for (int i = 0; i < clusterHolder.size(); ++i) { // loop through all clusters
                if (clusterHolder[i].contains(start)) { // find the start vertex
                    startSetIndex = i;
                }
                else if (clusterHolder[i].contains(end)) { // find the end vertex
                    endSetIndex = i;
                }
                if (startSetIndex != 0 && endSetIndex != 0) { // stop the loop if both vertexes were found
                    break;
                }
            }
            clusterHolder[startSetIndex] += clusterHolder[endSetIndex]; // merge clusters
            clusterHolder.remove(endSetIndex); // remove cluster for the end vertex
            mst.add(edge); // add edge to mst set
        }
        // do nothing if vertexes are in the same cluster
    }
    return mst;
}

// This method checks if the start and end vertexes for an edge are in the same cluster
bool sameCluster(Vector<Set<Vertex*>> clusterHolder, Edge* edge) {
    Vertex* start = edge->start;
    Vertex* end = edge->end;
    for (int i = 0; i < clusterHolder.size(); ++i) { // for each cluster in the clusterHolder
        if (clusterHolder[i].contains(start)
                && clusterHolder[i].contains(end)) {
            return true; // both vertexes are in the the same cluster
        }
    }
    return false; // not in the same cluster
}
