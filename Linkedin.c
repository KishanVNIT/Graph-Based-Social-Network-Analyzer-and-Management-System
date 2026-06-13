#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_USERS      100
#define MAX_HOBBIES    10
#define MAX_NAME_LEN   64
#define MAX_STR_LEN    128
#define INF            1000000/2   
#define BASE_WEIGHT    5
#define MAX 100
typedef struct EdgeNode {
    int friendId;
    int weight;
    int seq;
    struct EdgeNode *next;
} EdgeNode;

typedef struct User {
    int    id;
    int    active;
    char   name[MAX_NAME_LEN];
    char   school[MAX_STR_LEN];
    char   college[MAX_STR_LEN];
    char   company[MAX_STR_LEN];
    char   hobbies[MAX_HOBBIES][MAX_NAME_LEN];
    int    hobbyCount;
    int    careerNext[MAX_USERS];   
    int    careerNextCount;
    EdgeNode *adj;
} User;

typedef struct Graph {
    User  users[MAX_USERS];
    int   userCount;     
    int   activeCount;
    int   nextId;         
} Graph;

typedef struct WEdge{
    int u;
    int v;
    int w;
} WEdge;

int globalConnectionCounter = 0;

typedef struct DSU {
    int parent[MAX_USERS];
    int rnk[MAX_USERS];
} DSU;

void dsuInit(DSU *d, int n) {
    for (int i = 0; i < n; i++) {
       d->parent[i] = i; 
       d->rnk[i] = 0;
   }
}
int dsuFind(DSU *d, int x) {
    if (d->parent[x] != x) d->parent[x] = dsuFind(d, d->parent[x]);
    return d->parent[x];
}
int dsuUnion(DSU *d, int x, int y) {
    int px = dsuFind(d, x);
    int py = dsuFind(d, y);
    if (px == py) return 0;
    if (d->rnk[px] < d->rnk[py]) { 
      int t = px; 
      px = py; 
      py = t;
     }
    d->parent[py] = px;
    if (d->rnk[px] == d->rnk[py]) d->rnk[px]++;
    return 1;
}


int q[MAX];
int front = -1, rear = -1;

void push(int x) {
    if (rear == MAX - 1) {
        printf("Queue Overflow\n");
        return;
    }

    if (front == -1)
        front = 0;

    q[++rear] = x;
}

void pop() {
    if (front == -1 || front > rear) {
        printf("Queue Underflow\n");
        return;
    }

    front++;

    if (front > rear)
        front = rear = -1;
}

int Front() {
    if (front == -1 || front > rear) {
        printf("Queue is empty\n");
        return -1;
    }
    return q[front];
}

int empty() {
    return (front == -1 || front > rear);
}

int size() {
    if (empty())
        return 0;
    return rear - front + 1;
}

typedef struct {
    int wt;
    int node;
    int parent;
} PQItem;

typedef struct {
    PQItem data[MAX_USERS * MAX_USERS];
    int size;
} PriorityQueue;

void pqInit(PriorityQueue *pq) {
    pq->size = 0;
}

int pqEmpty(PriorityQueue *pq) {
    return pq->size == 0;
}

static void pqSwap(PQItem *a, PQItem *b) {
    PQItem tmp = *a;
    *a = *b;
    *b = tmp;
}

void pqPush(PriorityQueue *pq, int wt, int node, int parent) {
    int i = pq->size++;
    pq->data[i].wt     = wt;
    pq->data[i].node   = node;
    pq->data[i].parent = parent;

    while (i > 0) {
        int p = (i - 1) / 2;
        if (pq->data[p].wt <= pq->data[i].wt) break;
        pqSwap(&pq->data[p], &pq->data[i]);
        i = p;
    }
}

PQItem pqPop(PriorityQueue *pq) {
    PQItem top = pq->data[0];
    pq->data[0] = pq->data[--pq->size];

    int i = 0;
    while (1) {
        int smallest = i, l = 2*i + 1, r = 2*i + 2;
        if (l < pq->size && pq->data[l].wt < pq->data[smallest].wt) smallest = l;
        if (r < pq->size && pq->data[r].wt < pq->data[smallest].wt) smallest = r;
        if (smallest == i) break;
        pqSwap(&pq->data[i], &pq->data[smallest]);
        i = smallest;
    }
    return top;
}

void printLine(char c, int n) {
    for (int i = 0; i < n; i++){
      printf("%c", c);
    }
    printf("\n");
}
void printBanner(const char *title) {
    printLine('=', 60);
    printf("  %s\n", title);
    printLine('=', 60);
}

int findUserIndex(Graph *g, int id) {
    for (int i = 0; i < g->userCount; i++){
        if (g->users[i].active && g->users[i].id == id){
            return i;
          }
    }
    return -1;
}

int computeWeight(User *a, User *b) {
    int w = BASE_WEIGHT;
    if (strlen(a->college) > 0 && strcmp(a->college, b->college) == 0){
       w--;
      }
    if (strlen(a->company) > 0 && strcmp(a->company, b->company) == 0){
      w--;
      }
    if (strlen(a->school)  > 0 && strcmp(a->school,  b->school)  == 0){
       w--;
      }
    for (int i = 0; i < a->hobbyCount; i++){
        for (int j = 0; j < b->hobbyCount; j++){
            if (strcmp(a->hobbies[i], b->hobbies[j]) == 0) {
               w--; 
               goto done; 
              }
            
        }
      }
done:
    return (w < 1) ? 1 : w;
}

void removeEdge(User *u, int fid) {
    EdgeNode *prev = NULL, *cur = u->adj;
    while (cur) {
        if (cur->friendId == fid) {
            if (prev) prev->next = cur->next;
            else      u->adj    = cur->next;
            free(cur);
            return;
        }
        prev = cur; cur = cur->next;
    }
}
int addUser(Graph *g, const char *name,
            const char *school, const char *college, const char *company,
            const char **hobbies, int hobbyCount) {

    int slot = -1;
    for(int i=0;i <g->userCount;i++){
        if (!g->users[i].active) { 
          slot = i; 
          break;
         }
        }
    if (slot == -1) {
        if (g->userCount >= MAX_USERS) {
           printf("[ERROR] Network full.\n"); 
           return -1;
           }
        slot = g->userCount++;
    }
   
    User *u      = &g->users[slot];
    memset(u, 0, sizeof(User));
    u->id        = ++g->nextId;
    u->active    = 1;
    u->adj       = NULL;
    u->hobbyCount = 0;
    u->careerNextCount = 0;

 
    strncpy(u->name,    name,    MAX_NAME_LEN - 1);
     u->name[MAX_NAME_LEN - 1]   = '\0';
    strncpy(u->school,  school,  MAX_STR_LEN  - 1);
     u->school[MAX_STR_LEN - 1]  = '\0';
    strncpy(u->college, college, MAX_STR_LEN  - 1);
     u->college[MAX_STR_LEN - 1] = '\0';
    strncpy(u->company, company, MAX_STR_LEN  - 1);
     u->company[MAX_STR_LEN - 1] = '\0';

    for (int i = 0; i < hobbyCount && i < MAX_HOBBIES; i++) {
        strncpy(u->hobbies[i], hobbies[i], MAX_NAME_LEN - 1);
        u->hobbies[i][MAX_NAME_LEN - 1] = '\0';
        u->hobbyCount++;
    }
    g->activeCount++;
    printf(" User '%s' registered  (id=%d, slot=%d)\n", u->name, u->id, slot);
    return u->id;
}

void deleteUser(Graph *g, int id) {
    int idx = findUserIndex(g, id);
    if (idx == -1){
       printf("[ERROR] User id=%d not found.\n", id);
        return; 
      }

   
    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].active || i == idx) continue;
        removeEdge(&g->users[i], id);
    }
   
    EdgeNode *cur = g->users[idx].adj;
    while (cur){ 
      EdgeNode *tmp = cur; 
      cur = cur->next; 
      free(tmp); 
    }

    printf(" User '%s' (id=%d) deleted.\n", g->users[idx].name, id);
    
    g->users[idx].active = 0;
    g->users[idx].adj= NULL;
    g->activeCount--;
}

 void addDirectedEdge(User *u, int friendId, int weight, int seq) {
    for (EdgeNode *e = u->adj; e; e = e->next){
        if(e->friendId==friendId){
              return;
         }
      }
    EdgeNode *node= (EdgeNode *)malloc(sizeof(EdgeNode));
    node->friendId = friendId;
    node->weight= weight;
    node->seq= seq;  
    node->next = u->adj;
    u->adj = node;
}

void addConnection(Graph *g, int idA, int idB) {
    int ia = findUserIndex(g, idA);
    int ib = findUserIndex(g, idB);
    if (ia == -1 || ib == -1) {
       printf("[ERROR] User(s) not found.\n");
       return;
       }

    int seq = ++globalConnectionCounter;  
    int w = computeWeight(&g->users[ia], &g->users[ib]);
    addDirectedEdge(&g->users[ia], idB, w, seq);
    addDirectedEdge(&g->users[ib], idA, w, seq);
    printf("Connected: %-4s <-> %-8s  (weight=%d)\n",g->users[ia].name, g->users[ib].name, w);
}

void addCareerEdge(Graph *g, int fromId, int toId) {
    int fi = findUserIndex(g, fromId);
    int ti = findUserIndex(g, toId);
    if(fi == -1 || ti == -1){ 
      printf("[ERROR] User(s) not found.\n"); 
      return; 
    }
    User *u = &g->users[fi];
    for (int k=0; k<u->careerNextCount;k++){
        if(u->careerNext[k]==ti){ 
          return;
        } 
      } 
    if (u->careerNextCount < MAX_USERS){
        u->careerNext[u->careerNextCount++] = ti;
    }
    printf(" Career edge: %s -> %s\n", g->users[fi].name, g->users[ti].name);
}

void removeConnection(Graph *g, int idA, int idB) {
    int ia = findUserIndex(g, idA);
    int ib = findUserIndex(g, idB);
    if (ia == -1 || ib == -1){
       printf("[ERROR] User(s) not found.\n");
        return;
       }
    removeEdge(&g->users[ia], idB);
    removeEdge(&g->users[ib], idA);
    printf(" Connection removed: %s <-> %s\n", g->users[ia].name, g->users[ib].name);
}

void bfsLevels(Graph *g, int srcId, int maxLevel) {
    int src = findUserIndex(g, srcId);
    if (src == -1) {
       printf("[ERROR] Source not found.\n");
        return; 
      }

    printBanner("BFS : Friends by Level of Separation");
    printf("Source: %s\n\n", g->users[src].name);

    int visited[MAX_USERS] = {0};
    int dist[MAX_USERS];
    
    for (int i = 0; i < MAX_USERS; i++) dist[i] = -1;
    visited[src] = 1;
    dist[src] = 0; 
    push(src);

    while (!empty()) {
        int u = Front();
        pop();
        if (dist[u] >= maxLevel) continue;
        for (EdgeNode *e = g->users[u].adj; e; e = e->next) {
            int v = findUserIndex(g, e->friendId);
            if (v == -1 || !g->users[v].active || visited[v]) continue;
            visited[v] = 1; 
            dist[v] = dist[u] + 1;
            push(v);
        }
    }
    for (int level = 1; level <= maxLevel; level++) {
        printf("  Level %d:\n", level);
        int found = 0;
        for (int i = 0; i < g->userCount; i++)
            if (g->users[i].active && dist[i] == level) {
                printf("    -> %s (id=%d)\n", g->users[i].name, g->users[i].id);
                found = 1;
            }
        if (!found) printf("    (none)\n");
    }
}

 void dfsUtil(Graph *g, int u, int visited[]) {
    visited[u] = 1;
    printf("  -> %s (id=%d)\n", g->users[u].name, g->users[u].id);
    for (EdgeNode *e = g->users[u].adj; e; e = e->next) {
        int v = findUserIndex(g, e->friendId);
        if (v != -1 && g->users[v].active && !visited[v])
            dfsUtil(g, v, visited);
    }
}

void dfsTraversal(Graph *g, int srcId) {
    int src = findUserIndex(g, srcId);
    if (src == -1) { 
      printf("[ERROR] Source not found.\n"); 
      return; 
    }
    printBanner("DFS : Complete Network Exploration");
    printf("Starting from: %s\n\n", g->users[src].name);
    int visited[MAX_USERS] = {0};
    dfsUtil(g, src, visited);
    for (int i = 0; i < g->userCount; i++)
        if (g->users[i].active && !visited[i])
            dfsUtil(g, i, visited);
}

 int cycleHelperinTopo(Graph *g, int u, int visited[], int pathv[]) {
       visited[u] = 1;
       pathv[u]=1;
      for(int k = 0; k < g->users[u].careerNextCount; k++) {
            int v = g->users[u].careerNext[k];
            if (!g->users[v].active) continue;
            if (!visited[v]) {
             if (cycleHelperinTopo(g, v, visited,pathv)) return 1; 
            }
            else if (pathv[v]) return 1;
        }
        pathv[u]=0;
    return 0;
}
int detectCycleinTopo(Graph *g) {
    int visited[MAX_USERS] = {0};
    int pathv[MAX_USERS]={0};
    int hasCycle = 0;
    for (int i = 0; i < g->userCount; i++) {
        if (g->users[i].active && !visited[i])
            if (cycleHelperinTopo(g, i, visited,pathv)){ 
              hasCycle = 1;
               break;
               }
    }
   return hasCycle;
}
void topologicalSort(Graph *g) {
  printBanner("Topological Sort : Career/Mentorship Order (Kahn's BFS)");
  if(detectCycleinTopo(g)) {printf("  [!] Cycle detected in career edges — not all nodes sorted.\n");
    return;
    }
    int indegree[MAX_USERS] = {0};
    int n = g->userCount;
    for (int i = 0; i < n; i++) {
        if (!g->users[i].active) {
          continue;
        }
        for (int k = 0; k < g->users[i].careerNextCount; k++) {
            int v = g->users[i].careerNext[k];
            if (v >= 0 && v < n && g->users[v].active){
                indegree[v]++;
            }
        }
    }
    for (int i = 0; i < n; i++){
        if (g->users[i].active && indegree[i] == 0){
           push(i);
           }
        }

    if (empty()){ 
      printf("  No source nodes (or no career edges defined).\n");
       return; 
      }

    int step = 1;
    while (!empty()){
        int u = Front();
        pop();
        printf("  Step %d: %s\n", step++, g->users[u].name);
        for (int k = 0; k < g->users[u].careerNextCount; k++) {
            int v = g->users[u].careerNext[k];
            if (v < 0 || v >= n || !g->users[v].active) continue;
            if (--indegree[v] == 0) push(v);
        }
    }   
}

void primMST(Graph *g) {
     printBanner("Prim's MST : Minimum Cost Network ");
    int n = g->userCount;
    if (g->activeCount == 0) {
        printf("No active users.\n");
        return;
    }
    int vis[MAX_USERS] = {0};
    PriorityQueue pq;
    int totalWeight = 0;
    for (int start = 0; start < n; start++) {
        if (!g->users[start].active || vis[start]) continue;

        pqInit(&pq);
        pqPush(&pq, 0, start, -1);   

        while (!pqEmpty(&pq)) {
            PQItem it   = pqPop(&pq);
            int node    = it.node;
            int wt      = it.wt;
            int parent  = it.parent;

            if (vis[node] == 1) continue;   

            vis[node] = 1;
            totalWeight += wt;

            if (parent != -1) {
                printf("  Edge: %-15s <-> %-15s  w=%d\n",
                       g->users[parent].name, g->users[node].name, wt);
            }

            for (EdgeNode *e = g->users[node].adj; e; e = e->next) {
                int adjNode = findUserIndex(g, e->friendId);
                int edW     = e->weight;
                if (adjNode == -1 || !g->users[adjNode].active) continue;

                if (!vis[adjNode]) {
                    pqPush(&pq, edW, adjNode, node);
                }
            }
        }
    }

    printf("\n  Total MST weight: %d\n", totalWeight);
}

void sortEdges(WEdge edges[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (edges[j].w > edges[j + 1].w) {
                WEdge temp = edges[j];
                edges[j] = edges[j + 1];
                edges[j + 1] = temp;
            }
        }
    }
}
void kruskalMST(Graph *g) {
    WEdge edges[MAX_USERS * MAX_USERS];
     int eCnt = 0;
    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].active) continue;
        for (EdgeNode *e = g->users[i].adj; e; e = e->next) {
            int j = findUserIndex(g, e->friendId);
            if (j == -1 || j <= i) continue;
            edges[eCnt].u = i; 
            edges[eCnt].v = j;
            edges[eCnt].w = e->weight; 
            eCnt++;
        }
    }
    sortEdges(edges, eCnt);
    DSU dsu; 
    dsuInit(&dsu, g->userCount);
    printBanner("Kruskal's MST : Minimum Cost Network");
    int totalWeight = 0;
    for (int i = 0; i < eCnt; i++){
        int u = edges[i].u;
        int v = edges[i].v;
        int w = edges[i].w;
        if (dsuUnion(&dsu, u, v)) {
            printf("  Edge: %-15s <-> %-15s  w=%d\n",
                   g->users[u].name, g->users[v].name, w);
            totalWeight += w;
        }
    }
    printf("\n  Total MST weight: %d\n", totalWeight);
}

void dijkstra(Graph *g, int srcId) {
    int src = findUserIndex(g, srcId);
    if (src == -1) {
       printf("[ERROR] Source not found.\n");
        return;
       }
    int n = g->userCount;
    int dist[MAX_USERS];
    int visited[MAX_USERS];
    int  prev[MAX_USERS];
    for (int i = 0; i < n; i++) { 
       dist[i] = INF;
       visited[i] = 0; 
       prev[i] = -1;
      }
    dist[src] = 0;

    for (int iter = 0; iter < n; iter++) {
        int u = -1;
        for (int i = 0; i < n; i++)
            if (!visited[i] && g->users[i].active && (u == -1 || dist[i] < dist[u])) u = i;
        if (u == -1 || dist[u] == INF) break;
        visited[u] = 1;
        for (EdgeNode *e = g->users[u].adj; e; e = e->next) {
            int v = findUserIndex(g, e->friendId);
            if (v == -1 || !g->users[v].active) continue;
            if (dist[u] + e->weight < dist[v]) {
               dist[v] = dist[u] + e->weight; 
               prev[v] = u; 
              }
        }
    }

    printBanner("Dijkstra : Single Source Shortest Path");
    printf("Source: %s\n\n", g->users[src].name);
    for (int i = 0; i < n; i++) {
        if (!g->users[i].active || i == src) continue;
        if (dist[i] == INF) {
             printf("  %-15s : unreachable\n", g->users[i].name);
              continue;
             }
        printf("  %-15s : dist=%-3d  path: ", g->users[i].name, dist[i]);
        int path[MAX_USERS];
        int pLen = 0;
        int cur = i;
        while (cur != -1) { 
          path[pLen++] = cur;
           cur = prev[cur];
           }
        for (int p = pLen - 1; p >= 0; p--) {
            printf("%s", g->users[path[p]].name);
            if (p > 0) printf(" -> ");
        }
        printf("\n");
    }
}

void floydWarshall(Graph *g) {
    int revMap[MAX_USERS];
    int  k = 0;
    int idxMap[MAX_USERS];
    for (int i = 0; i < g->userCount; i++) {
        idxMap[i] = g->users[i].active ? k : -1;
        if (g->users[i].active) revMap[k++] = i;
    }

    int dist[MAX_USERS][MAX_USERS];
    int nxt[MAX_USERS][MAX_USERS];
    for (int i = 0; i < k; i++){
        for (int j = 0; j < k; j++) {
            dist[i][j] = (i == j) ? 0 : INF;
            nxt[i][j]  = (i == j) ? i : -1;
        }
    }
    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].active) continue;
        int u = idxMap[i];
        for (EdgeNode *e = g->users[i].adj; e; e = e->next) {
            int j = findUserIndex(g, e->friendId);
            if (j == -1) continue;
            int v = idxMap[j];
            if (e->weight < dist[u][v]){ 
              dist[u][v] = dist[v][u] = e->weight; 
              nxt[u][v] = v; 
              nxt[v][u] = u;
             }
        }
    }
    for (int m = 0; m < k; m++)
        for (int i = 0; i < k; i++)
            for (int j = 0; j < k; j++)
                if (dist[i][m] != INF && dist[m][j] != INF &&
                    dist[i][m] + dist[m][j] < dist[i][j]) {
                    dist[i][j] = dist[i][m] + dist[m][j];
                    nxt[i][j]  = nxt[i][m];
                }

    printBanner("Floyd-Warshall : All-Pair Shortest Paths");

    printf("\n  [Distance Matrix]\n  %-15s", "");
    for (int j = 0; j < k; j++) printf("%-10s", g->users[revMap[j]].name);
    printf("\n"); printLine('-', 60);
    for (int i = 0; i < k; i++) {
        printf("  %-15s", g->users[revMap[i]].name);
        for (int j = 0; j < k; j++)
            if (dist[i][j] == INF) printf("%-10s", "INF");
             else printf("%-10d", dist[i][j]);
        printf("\n");
    }

   
    printf("\n  [Path Reconstruction]\n");
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            if (i == j) continue;
            if (nxt[i][j] == -1) {
                printf("  %-10s -> %-10s : no path\n",g->users[revMap[i]].name, g->users[revMap[j]].name);
                continue;
            }
            printf("  %-10s -> %-10s : ", g->users[revMap[i]].name, g->users[revMap[j]].name);
            int cur = i;
            while (cur != j) {
                printf("%s -> ", g->users[revMap[cur]].name);
                cur = nxt[cur][j];
                if (cur == -1) { 
                  printf("(error)");
                   break; 
                  }
            }
            printf("%s  (dist=%d)\n", g->users[revMap[j]].name, dist[i][j]);
        }
    }
}



 int matchAttr(User *u, const char *attr, const char *value) {
    if (strcmp(attr, "name")    == 0 && strstr(u->name,    value)) return 1;
    if (strcmp(attr, "school")  == 0 && strcmp(u->school,  value) == 0) return 1;
    if (strcmp(attr, "college") == 0 && strcmp(u->college, value) == 0) return 1;
    if (strcmp(attr, "company") == 0 && strcmp(u->company, value) == 0) return 1;
    if (strcmp(attr, "hobby")   == 0)
        for (int h = 0; h < u->hobbyCount; h++)
            if (strcmp(u->hobbies[h], value) == 0) return 1;
    return 0;
}

void searchByBFS(Graph *g, int srcId, const char *attr, const char *value) {
    int src = findUserIndex(g, srcId);
    if (src == -1){ 
      printf("[ERROR] Source not found.\n");
      return;
       }
    printBanner("BFS Search");
    printf("  Attribute: %s = \"%s\"  (BFS from %s)\n\n", attr, value, g->users[src].name);

    int visited[MAX_USERS] = {0};
    push(src);
    visited[src] = 1;
    int found = 0;
    while (!empty()) {
        int u = Front();
        pop();
        if (matchAttr(&g->users[u], attr, value)) {
            printf("  [FOUND via BFS] %-15s  college=%-15s  company=%s\n",
                   g->users[u].name, g->users[u].college, g->users[u].company);
            found++;
        }
        for (EdgeNode *e = g->users[u].adj; e; e = e->next) {
            int v = findUserIndex(g, e->friendId);
            if (v != -1 && g->users[v].active && !visited[v]) {
                visited[v] = 1; 
                push(v);
            }
        }
    }
  
    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].active || visited[i]) continue;
        visited[i] = 1;
         push(i);
        while (!empty()) {
            int u = Front();
            pop();
            if (matchAttr(&g->users[u], attr, value)) {
                printf("  [FOUND via BFS] %-15s  college=%-15s  company=%s\n",
                       g->users[u].name, g->users[u].college, g->users[u].company);
                found++;
            }
            for (EdgeNode *e = g->users[u].adj; e; e = e->next) {
                int v = findUserIndex(g, e->friendId);
                if (v != -1 && g->users[v].active && !visited[v]) {
                    visited[v] = 1;
                     push(v);
                }
            }
        }
    }
    printf("\n  Total found: %d\n", found);
}

 void dfsSearchUtil(Graph *g, int u, int visited[],
                           const char *attr, const char *value, int *found) {
    visited[u] = 1;
    if (matchAttr(&g->users[u], attr, value)) {
        printf("  [FOUND via DFS] %-15s  college=%-15s  company=%s\n",
               g->users[u].name, g->users[u].college, g->users[u].company);
        (*found)++;
    }
    for (EdgeNode *e = g->users[u].adj; e; e = e->next) {
        int v = findUserIndex(g, e->friendId);
        if (v != -1 && g->users[v].active && !visited[v])
            dfsSearchUtil(g, v, visited, attr, value, found);
    }
}

void searchByDFS(Graph *g, int srcId, const char *attr, const char *value) {
    int src = findUserIndex(g, srcId);
    if (src == -1){ 
      printf("[ERROR] Source not found.\n");
       return; 
      }
    printBanner("DFS Search");
    printf("  Attribute: %s = \"%s\"  (DFS from %s)\n\n", attr, value, g->users[src].name);
    int visited[MAX_USERS] = {0};
    int found = 0;
    dfsSearchUtil(g, src, visited, attr, value, &found);
    for (int i = 0; i < g->userCount; i++)
        if (g->users[i].active && !visited[i])
            dfsSearchUtil(g, i, visited, attr, value, &found);
    printf("\n  Total found: %d\n", found);
}

int isConnected(Graph *g, int idA, int idB) {
    int src = findUserIndex(g, idA);
    int dst = findUserIndex(g, idB);
    if (src == -1 || dst == -1) return -1;
    if (src == dst) return 1;
    int visited[MAX_USERS] = {0};
    visited[src] = 1;
     push(src);
    while (!empty()) {
        int u = Front();
        pop();
        for (EdgeNode *e = g->users[u].adj; e; e = e->next) {
            int v = findUserIndex(g, e->friendId);
            if (v == -1 || !g->users[v].active) continue;
            if (v == dst) return 1;
            if (!visited[v]) { 
              visited[v] = 1; 
              push(v);
             }
        }
    }
    return 0;
}

void checkConnection(Graph *g, int idA, int idB) {
    int ia = findUserIndex(g, idA);
    int ib = findUserIndex(g, idB);
    if (ia == -1 || ib == -1){ 
      printf("[ERROR] User(s) not found.\n"); 
      return;
     }
    int res = isConnected(g, idA, idB);
    printf("\n  isConnected(%s, %s) -> %s\n",
           g->users[ia].name, g->users[ib].name,
           res == 1 ? "YES " : "NO ");
}

int degreeOfSeparation(Graph *g, int idA, int idB) {
    int src = findUserIndex(g, idA);
    int dst = findUserIndex(g, idB);
    if (src == -1 || dst == -1) return -1;
    if (src == dst) return 0;
    int dist[MAX_USERS];
    int visited[MAX_USERS] = {0};
    for (int i = 0; i < MAX_USERS; i++) dist[i] = -1;
    dist[src] = 0;
     visited[src] = 1; 
     push(src);
    while (!empty()) {
        int u = Front();
        pop();
        for (EdgeNode *e = g->users[u].adj; e; e = e->next) {
            int v = findUserIndex(g, e->friendId);
            if (v == -1 || !g->users[v].active || visited[v]) continue;
            visited[v] = 1;
            dist[v] = dist[u] + 1;
            if (v == dst) return dist[v];
            push(v);
        }
    }
    return -1;   
}

void printDegreeOfSeparation(Graph *g, int idA, int idB) {
    int ia = findUserIndex(g, idA);
    int ib = findUserIndex(g, idB);
    if (ia == -1 || ib == -1) { 
      printf("[ERROR] User(s) not found.\n");
       return;
       }
    int d = degreeOfSeparation(g, idA, idB);
    if (d == -1) printf("\n  %s and %s are NOT connected.\n",
                        g->users[ia].name, g->users[ib].name);
    else printf("\n  Degree of separation between %s and %s: %d\n",
                g->users[ia].name, g->users[ib].name, d);
}

 int cycleHelper(Graph *g, int u, int visited[], int parent) {
    visited[u] = 1;
    for (EdgeNode *e = g->users[u].adj; e; e = e->next) {
        int v = findUserIndex(g, e->friendId);
        if (v == -1 || !g->users[v].active) continue;
        if (!visited[v]) { if (cycleHelper(g, v, visited, u)) return 1; }
        else if (v != parent) return 1;
    }
    return 0;
}
void detectCycle(Graph *g) {
    printBanner("Cycle Detection");
    int visited[MAX_USERS] = {0};
    int hasCycle = 0;
    for (int i = 0; i < g->userCount; i++) {
        if (g->users[i].active && !visited[i])
            if (cycleHelper(g, i, visited, -1)){ 
              hasCycle = 1;
               break;
               }
    }
    printf("  DFS result   : %s\n", hasCycle ? "Cycle DETECTED!" : "No cycle.");

    DSU dsu; 
    dsuInit(&dsu, g->userCount);
    int cycleUF = 0;
    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].active) continue;
        for (EdgeNode *e = g->users[i].adj; e; e = e->next) {
            int j = findUserIndex(g, e->friendId);
            if (j == -1 || j <= i) continue;
            if (!dsuUnion(&dsu, i, j)) { 
                cycleUF = 1;
                 break; 
                }
        }
        if (cycleUF) break;
    }
    printf("  Union-Find   : %s\n", cycleUF ? "Cycle DETECTED!" : "No cycle.");
}

 int pathStack[MAX_USERS], pathLen = 0, pathCount = 0;

void allPathsUtil(Graph *g, int u, int dest, int visited[]) {
    visited[u] = 1;
    pathStack[pathLen++] = u;
    if (u == dest) {
        printf("  Path %d: ", ++pathCount);
        for (int i = 0; i < pathLen; i++) {
            printf("%s", g->users[pathStack[i]].name);
            if (i < pathLen - 1) printf(" -> ");
        }
        printf("\n");
    } else {
        for (EdgeNode *e = g->users[u].adj; e; e = e->next) {
            int v = findUserIndex(g, e->friendId);
            if (v != -1 && g->users[v].active && !visited[v])
                allPathsUtil(g, v, dest, visited);
        }
    }
    pathLen--;
   visited[u] = 0;
}

void allPaths(Graph *g, int srcId, int dstId) {
    int src = findUserIndex(g, srcId);
    int dst = findUserIndex(g, dstId);
    if (src == -1 || dst == -1){
       printf("[ERROR] User(s) not found.\n"); 
       return; 
      }
    printBanner("All Paths : Backtracking DFS");
    printf("From: %s  ->  To: %s\n\n", g->users[src].name, g->users[dst].name);
    int visited[MAX_USERS] = {0};
    pathLen = 0;
    pathCount = 0;
    allPathsUtil(g, src, dst, visited);
    if (pathCount == 0) printf("  No paths found.\n");
    else printf("\n  Total paths: %d\n", pathCount);
}

void graphDiameter(Graph *g) {
    printBanner("Graph Diameter : Maximum Degree of Separation");
    int maxDist = 0;
    int uMax = -1;
    int vMax = -1;
    for (int s = 0; s < g->userCount; s++) {
        if (!g->users[s].active) continue;
        int dist[MAX_USERS];
        int visited[MAX_USERS] = {0};
        for (int i = 0; i < MAX_USERS; i++) dist[i] = -1;
        dist[s] = 0; 
        visited[s] = 1;
         push(s);
        while (!empty()) {
            int u = Front();
            pop();
            for (EdgeNode *e = g->users[u].adj; e; e = e->next) {
                int v = findUserIndex(g, e->friendId);
                if (v == -1 || !g->users[v].active || visited[v]) continue;
                visited[v] = 1;
                 dist[v] = dist[u] + 1; 
                 push(v);
                if (dist[v] > maxDist) { 
                  maxDist = dist[v];
                   uMax = s;
                    vMax = v; 
                  }
            }
        }
    }
    if (uMax == -1) printf("  Single node or no edges.\n");
    else printf("  Max separation: %d hops  (%s <-> %s)\n",
                maxDist, g->users[uMax].name, g->users[vMax].name);
}

void displayContactList(Graph *g) {
    printBanner("Contact List (Adjacency List)");
    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].active) continue;
        printf("  %-15s ->", g->users[i].name);
        int cnt = 0;
        for (EdgeNode *e = g->users[i].adj; e; e = e->next) {
            int j = findUserIndex(g, e->friendId);
            if (j != -1) { printf("  %s(w=%d)", g->users[j].name, e->weight); cnt++; }
        }
        if (!cnt) printf("(none)");
        printf("\n");
    }
}

void displayUserNetwork(Graph *g, int userId) {
    int idx = findUserIndex(g, userId);
    if (idx == -1){
       printf("[ERROR] User not found.\n");
        return; 
      }
    printBanner("User Network");
    printf("  User: %s\n\n", g->users[idx].name);

   
    printf("  [Direct Connections]\n");
    int directCnt = 0;
    for (EdgeNode *e = g->users[idx].adj; e; e = e->next) {
        int j = findUserIndex(g, e->friendId);
        if (j != -1){ 
          printf("    -> %-15s  (weight=%d)\n", g->users[j].name, e->weight); 
          directCnt++; 
        }
    }
    if (!directCnt) printf("    (none)\n");

    
    printf("\n  [Complete Reachable Network : BFS]\n");
    int visited[MAX_USERS] = {0};
    int dist[MAX_USERS];
    for (int i = 0; i < MAX_USERS; i++) dist[i] = -1;
    visited[idx] = 1; 
    dist[idx] = 0;
    push(idx);
    while (!empty()) {
        int u = Front();
        pop();
        for (EdgeNode *e = g->users[u].adj; e; e = e->next) {
            int v = findUserIndex(g, e->friendId);
            if (v == -1 || !g->users[v].active || visited[v]) continue;
            visited[v] = 1;
             dist[v] = dist[u] + 1; 
             push(v);
            printf("    Level %d: %-15s\n", dist[v], g->users[v].name);
        }
    }
}

void findMates(Graph *g, int userId, const char *field) {
    int idx = findUserIndex(g, userId);
    if (idx == -1) {
       printf("[ERROR] User not found.\n"); 
       return;
       }
    User *u = &g->users[idx];
    const char *target = strcmp(field,"school")  == 0 ? u->school  :
                         strcmp(field,"college") == 0 ? u->college :
                         strcmp(field,"company") == 0 ? u->company : NULL;
    if (!target){ 
      printf("[ERROR] Invalid field.\n");
       return;
       }
    printf("  %s's %s-mates (at '%s'):\n", u->name, field, target);
    int found = 0;
    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].active || i == idx) continue;
        const char *val = strcmp(field,"school")  == 0 ? g->users[i].school  :
                          strcmp(field,"college") == 0 ? g->users[i].college : g->users[i].company;
        if (strcmp(val, target) == 0) {
           printf("    -> %s\n", g->users[i].name);
            found++; 
          }
    }
    if (!found) printf("    (none)\n");
}

void hobbyGroups(Graph *g) {
    printBanner("Potential Hobby Groups");
    char unique[MAX_USERS * MAX_HOBBIES][MAX_NAME_LEN];
     int uCnt = 0;
    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].active) continue;
        for (int h = 0; h < g->users[i].hobbyCount; h++) {
            int dup = 0;
            for (int k = 0; k < uCnt; k++)
                if (strcmp(unique[k], g->users[i].hobbies[h]) == 0) { 
                  dup = 1; 
                  break;
                 }
            if (!dup) { 
               strncpy(unique[uCnt], g->users[i].hobbies[h], MAX_NAME_LEN - 1);
               unique[uCnt][MAX_NAME_LEN-1]='\0'; 
               uCnt++;
               }
        }
    }
    for (int k = 0; k < uCnt; k++) {
        printf("  [%s Group]\n", unique[k]);
        for (int i = 0; i < g->userCount; i++) {
            if (!g->users[i].active) continue;
            for (int h = 0; h < g->users[i].hobbyCount; h++)
                if (strcmp(g->users[i].hobbies[h], unique[k]) == 0)
                    printf("    -> %s\n", g->users[i].name);
        }
        printf("\n");
    }
}

void latestConnections(Graph *g, int userId) {
    int idx = findUserIndex(g, userId);
    if (idx == -1) {
        printf("[ERROR] User not found.\n");
        return;
    }
    printBanner("Latest Connections");
    printf("  User: %s\n\n", g->users[idx].name);

    typedef struct {
        char name[MAX_NAME_LEN];
        int  seq;    
        int  fid;
    } Conn;

    Conn conns[MAX_USERS];
    int cnt = 0;

    for (EdgeNode *e = g->users[idx].adj; e; e = e->next) {
        int j = findUserIndex(g, e->friendId);
        if (j == -1) continue;
        strncpy(conns[cnt].name, g->users[j].name, MAX_NAME_LEN - 1);
        conns[cnt].name[MAX_NAME_LEN - 1] = '\0';
        conns[cnt].seq = e->seq;          
        conns[cnt].fid = e->friendId;
        cnt++;
    }

    for (int i = 0; i < cnt - 1; i++)
        for (int j = i + 1; j < cnt; j++)
            if (conns[j].seq > conns[i].seq) {
                Conn tmp = conns[i];
                conns[i] = conns[j];
                conns[j] = tmp;
            }

    printf("  [%s's connections : newest first]\n", g->users[idx].name);
    for (int i = 0; i < cnt; i++) {
        printf("    -> %-15s  (connection #%d)\n", conns[i].name, conns[i].seq);
    }
    if (!cnt) printf("    (none)\n");

    printf("\n  [Each Friend's Latest Connections]\n");
    for (int i = 0; i < cnt; i++) {
        int fi = findUserIndex(g, conns[i].fid);
        if (fi == -1) continue;
        printf("\n    %-15s ->", conns[i].name);

        Conn fc[MAX_USERS];
        int fcnt = 0;
        for (EdgeNode *e = g->users[fi].adj; e; e = e->next) {
            int j = findUserIndex(g, e->friendId);
            if (j == -1) continue;
            strncpy(fc[fcnt].name, g->users[j].name, MAX_NAME_LEN - 1);
            fc[fcnt].name[MAX_NAME_LEN - 1] = '\0';
            fc[fcnt].seq = e->seq;
            fcnt++;
        }

        for (int a = 0; a < fcnt - 1; a++)
            for (int b = a + 1; b < fcnt; b++)
                if (fc[b].seq > fc[a].seq) {
                    Conn tmp = fc[a];
                    fc[a] = fc[b];
                    fc[b] = tmp;
                }

        for (int a = 0; a < fcnt; a++) {
            printf("  %s(#%d)", fc[a].name, fc[a].seq);
        }
        if (!fcnt) printf("  (none)");
    }
    printf("\n");
}
void displayProfile(Graph *g, int userId) {
    int idx =findUserIndex(g, userId);
    if (idx == -1){ 
      printf("[ERROR] User not found.\n");
       return;
       }
    User *u = &g->users[idx];
    printLine('-', 50);
    printf("  PROFILE: %-20s (id=%d)\n", u->name, u->id);
    printLine('-', 50);
    printf("  School  : %s\n  College : %s\n  Company : %s\n",u->school, u->college, u->company);
    printf("  Hobbies : ");
    for (int h = 0; h < u->hobbyCount; h++){
        printf("%s%s", u->hobbies[h], h < u->hobbyCount - 1 ? ", " : "");
    }
    int cnt = 0;
     for (EdgeNode *e = u->adj; e; e = e->next) cnt++;
    printf("\n  Friends : %d\n", cnt);
    printLine('-', 50);
}

void displayProfilesByName(Graph *g,   char *name) {
    printBanner("Search Profiles by Name");
    printf("  Query: \"%s\"\n\n", name);

    char lq[MAX_NAME_LEN];
    int k;
    for (k = 0; name[k] && k < MAX_NAME_LEN - 1; k++){
        lq[k] = (name[k] >= 'A' && name[k] <= 'Z') ? name[k] + 32 : name[k];
    }
    lq[k] = '\0';

    int found = 0;

    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].active) continue;

        
        char ln[MAX_NAME_LEN];
        for (k = 0; g->users[i].name[k] && k < MAX_NAME_LEN - 1; k++){
            ln[k] = (g->users[i].name[k] >= 'A' && g->users[i].name[k] <= 'Z')
                     ? g->users[i].name[k] + 32 : g->users[i].name[k];
        }
        ln[k] = '\0';

        
        if (strstr(ln, lq)) {
            displayProfile(g, g->users[i].id);
            found++;
        }
    }

    if (!found) {
        printf("  No profiles found matching \"%s\".\n", name);
    } else {
        printf("\n  Total profiles found: %d\n", found);
    }
}
int main(void) {
    Graph g; 
    memset(&g, 0, sizeof(g));

    printBanner("PROFESSIONAL SOCIAL NETWORK ANALYZER ");
    printf("  Graph-based LinkedIn-like Platform in C\n\n");

    const char *h1[] = {"Cricket","Coding","Reading"};
    const char *h2[] = {"Cricket","Photography"};
    const char *h3[] = {"Coding","Music"};
    const char *h4[] = {"Reading","Travel"};
    const char *h5[] = {"Photography","Travel","Cricket"};
    const char *h6[] = {"Coding","Gaming"};
    const char *h7[] = {"Music","Reading"};
  
    int id1 = addUser(&g, "Kishan", "DPS School",  "IIT Delhi",   "Google",    h1, 3);
    int id2 = addUser(&g, "Aman",   "St. Mary's",  "IIT Delhi",   "Microsoft", h2, 2);
    int id3 = addUser(&g, "Ravi",   "DPS School",  "NIT Bhopal",  "Amazon",    h3, 2);
    int id4 = addUser(&g, "Neha",   "Kendriya",    "IIT Delhi",   "Google",    h4, 2);
    int id5 = addUser(&g, "Rahul",  "St. Mary's",  "NIT Bhopal",  "Infosys",   h5, 3);
    int id6 = addUser(&g, "Priya",  "DPS School",  "BITS Pilani", "TCS",       h6, 2);
    int id7 = addUser(&g, "Rohan",  "Kendriya",    "BITS Pilani", "Wipro",     h7, 2);
    printf("\n");

    printf("Adding connections...\n");
    addConnection(&g, id1, id2);
    addConnection(&g, id1, id3);
    addConnection(&g, id1, id4);
    addConnection(&g, id2, id5);
    addConnection(&g, id3, id5);
    addConnection(&g, id4, id6);
    addConnection(&g, id5, id7);
    addConnection(&g, id6, id7);
    addConnection(&g, id2, id4);
    printf("\n");

    printf("Adding career edges (directed) for topological sort...\n");
    addCareerEdge(&g, id1, id2);   
    addCareerEdge(&g, id1, id4);   
    addCareerEdge(&g, id2, id5);   
    addCareerEdge(&g, id4, id6);  
    addCareerEdge(&g, id5, id7);   
    addCareerEdge(&g, id3, id5);   
    printf("\n");

    displayContactList(&g);
    printf("\n");

    printBanner("Search Profiles by ID");
    displayProfile(&g, id1);
    displayProfile(&g, id5);
    printf("\n");

    displayProfilesByName(&g, "ro");
    printf("\n");

    bfsLevels(&g, id1, 3);
    printf("\n");

    dfsTraversal(&g, id1);
    printf("\n");

    topologicalSort(&g);
    printf("\n");

    primMST(&g);
    printf("\n");

    kruskalMST(&g);
    printf("\n");

    dijkstra(&g, id1);
    printf("\n");

    floydWarshall(&g);
    printf("\n");

    searchByBFS(&g, id1, "hobby",   "Cricket");
    printf("\n");
    searchByDFS(&g, id1, "college", "IIT Delhi");
    printf("\n");

    detectCycle(&g);
    printf("\n");

    allPaths(&g, id1, id5);
    printf("\n");

    graphDiameter(&g);
    printf("\n");

    printBanner("Is Connected? (BFS)");
    checkConnection(&g, id1, id7);
    checkConnection(&g, id1, id5);
    
    printf("\n");
    printBanner("Degree of Separation");
    printDegreeOfSeparation(&g, id1, id7);
    printDegreeOfSeparation(&g, id1, id5);
    printDegreeOfSeparation(&g, id2, id6);

    printf("\n");
    printBanner("Mates Finder");
    findMates(&g, id1, "college");
    findMates(&g, id1, "company");
    findMates(&g, id1, "school");

    printf("\n");
    hobbyGroups(&g);

    latestConnections(&g, id1);

    printf("\n");
    displayUserNetwork(&g, id1);

    printf("\n");
    printBanner("Delete User + Slot Reuse ");
    printf("  Active users before: %d  | slots used: %d\n", g.activeCount, g.userCount);
    deleteUser(&g, id6);  
    printf("  Active users after : %d  | slots used: %d\n", g.activeCount, g.userCount);
    const char *hn[] = {"Cricket","Painting"};
    int idNew = addUser(&g, "Sneha", "DPS School", "IIT Delhi", "Google", hn, 2);
    printf("  New user Sneha occupies slot (slot count still = %d)\n", g.userCount);
    addConnection(&g, id1, idNew);
    printf("\n");

    printBanner("Remove Connection Demo");
    removeConnection(&g, id1, id3);
    printf("  Kishan's connections after removing Ravi:\n    ");
    int ki = findUserIndex(&g, id1);
    for (EdgeNode *e = g.users[ki].adj; e; e = e->next) {
        int j = findUserIndex(&g, e->friendId);
        if (j != -1) printf("%s  ", g.users[j].name);
    }
    printf("\n");

    printf("\n");
    printBanner("isConnected after changes");
    checkConnection(&g, id1, id7);
    checkConnection(&g, id1, idNew);

    printf("\n");
    
    return 0;
}
