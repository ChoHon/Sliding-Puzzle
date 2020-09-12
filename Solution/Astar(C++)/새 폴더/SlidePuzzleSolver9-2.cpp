# include <iostream>
# include <ctime>
# include <queue>
# include <vector>
# include <algorithm>
# include <cstdlib>

# pragma warning(disable : 4996)
using namespace std;

typedef unsigned int ui;
typedef unsigned char uc;

bool isSolved = false;
int Size;
char mdTable[25][25];
char movableTable[25][4];
uc path[256];
ui nodeCount;
char dirs[4] = {-1, 1, -2, 2};
char moves[24][4];

struct NodeInfo {
	ui id;
	ui parent;
	char blank;
	char move;
};

struct Node {
	char board[25];
	char move;
	NodeInfo nodeInfo;
	uc f, g;

	Node(char* bd, char blk, char move, ui id, ui parent, uc f = 0, uc g = 0)
	{
		for (int i = 0; i < Size * Size; ++i)
		{
			board[i] = bd[i];
		}
		nodeInfo.blank = blk;
		nodeInfo.id = id;
		nodeInfo.parent = parent;
		nodeInfo.move = move;
		this->f = f;
		this->g = g;
	}

	bool operator >(const Node& other) const
	{
		/*
		if(f == other.f)
		{
			return g < other.g;
		}
		else
		{
			return f > other.f;
		}
		*/
		return f > other.f;
	}
};

void MakeDirections()
{
	int i = 0;
	char dirs[] = {0, 1, 2, 3};
	do{
		moves[i][0] = dirs[0];
		moves[i][1] = dirs[1];
		moves[i][2] = dirs[2];
		moves[i++][3] = dirs[3];
	}while ( next_permutation(dirs, dirs + 4) );
}

void MakeMovableTable(int size)
{
	int moves[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
	int board[5][5];
	
	for(int y = 0; y < size; ++y)
	{
		for(int x = 0; x < size; ++x)
		{
			board[y][x] = x + y * size;
		}
	}
	
	
	int dx, dy, j;
	for(int y = 0; y < size; ++y)
	{
		for(int x = 0; x < size; ++x)
		{
			for(int i = 0; i < 4; ++i)
			{
				dx = moves[i][0];
				dy = moves[i][1];
				if(x + dx < 0 || y + dy < 0 || x + dx >= size || y + dy >= size) 
				{
					j = -1;
				}
				else
				{
					j = board[y + dy][x + dx];
				} 
				movableTable[x + y * size][i] = j;
			}
		}
	}
}

void MakeMDTable(int size)
{
	for(int y = 0, i = 1; y < size * size; ++y, (++i) % (size * size))
	{
		for(int x = 0; x < size * size; ++x)
		{
			if(i == 0)
			{
				mdTable[i][x] = 0;
			}
			else
			{
				mdTable[i][x] = abs((y / size) - (x / size)) + abs((y % size) - (x % size));
			}
		}
	}
}

char GetDistance(int *row, char md, int nums)
{
	if (nums > 1)
	{
		if (nums == 2)
		{
			if (row[0] > row[1])
				md += 2;
		}
		else if (nums == 3)
		{
			if (row[0] > row[1] || row[0] > row[2])
				md += 2;
			if (row[1] > row[2])
				md += 2;
		}
		else if (nums == 4)
		{
			if (row[0] > row[1] || row[0] > row[2] || row[0] > row[3])
				md += 2;
			if (row[1] > row[2] || row[1] > row[3])
				md += 2;
			if (row[2] > row[3])
				md += 2;
		}
		else if (nums == 5)
		{
			if (row[0] > row[1] || row[0] > row[2] || row[0] > row[3] || row[0] > row[4])
				md += 2;
			if (row[1] > row[2] || row[1] > row[3] || row[1] > row[4])
				md += 2;
			if (row[2] > row[3] || row[2] > row[4])
				md += 2;
			if(row[3] > row[4])
				md += 2;
		}
	}
	return md;
}

char GetHeuristic(char* puzzle)
{
	int i, j, x, md = 0;
	int k, n;
	int temp[5];

	for(i = 0; i < Size * Size; ++i)
	{
		md += mdTable[puzzle[i]][i];
	}
	// return md;

	
	// row(가로) 방향을 검사합니다. 
	for (i = 0, x = 1; i < Size; ++i, ++x)
	{
		k = 0;
		for (j = 0; j < Size; ++j)
		{
			n = puzzle[i * Size + j];
			if (n <= Size * x && n > Size * (x - 1))
			{
				temp[k++] = n;
			}
		}
		md = GetDistance(temp, md, k);
	}
	
	// column(세로) 방향을 검사합니다. 
	for (i = 0, x = 1; i < Size; ++i, ++x)
	{
		k = 0;
		for (j = 0; j < Size; ++j)
		{
			n = puzzle[j * Size + i];
			if (n == x || n == x + Size || n == x + Size * 2 || n == x + Size * 3)
			{
				temp[k++] = n;
			}
		}
		md = GetDistance(temp, md, k);
	}
	return md;
} 

char GetBlank(char *puzzle)
{
	for(int i = 0; i < Size * Size; ++i)
	{
		if (puzzle[i] == 0)
		{
			return i;
		}
	}
}

void PrintPath(char depth)
{
	for(int i = 0; i < depth; ++i)
	{
		printf("%2d  ",path[i]);
		if((i + 1) % 10 == 0) printf("\n");
	}
	printf("\n\n");
}

void PrintPuzzle(char* puzzle)
{
	for(int i = 0; i < Size; ++i)
	{
		for(int j = 0; j < Size; ++j)
		{
			printf("%2d ", puzzle[i * Size + j]);
		}
		printf("\n");
	}
	printf("\n");
}

int i = 0;
void IdaStar(char maxDepth, char* puzzle, char lastMove, char blank, char dir)
{
	if (maxDepth == 0)
	{
		//PrintPuzzle(puzzle);
		isSolved = true;
		return;
	} 
	else 
	{
		for (int j = 0; j < 4; ++j)
		{
			if(lastMove == -1 || (dirs[moves[dir][j]] + dirs[lastMove]) != 0)
			{
				char newBlank = movableTable[blank][moves[dir][j]];
				if(newBlank == -1) continue;
	
				puzzle[blank] = puzzle[newBlank];
				puzzle[newBlank] = 0; 
	
				uc h = GetHeuristic(puzzle);
				if(h < maxDepth && !isSolved)
				{
					nodeCount += 1;
					path[i++] = puzzle[blank];
					IdaStar(maxDepth - 1, puzzle, moves[dir][j], newBlank, dir);
					--i;
				}
				puzzle[newBlank] = puzzle[blank];
				puzzle[blank] = 0;
			}
		}
	}
}

void IDAStar(char* puzzle, char dir)
{
	time_t start, end;
	uc h, depth;
	char lastMove = -1;
	char blank = GetBlank(puzzle);
	
	isSolved = false;
	ui totalCount = 0;
	
	h = GetHeuristic(puzzle);
	depth = h;
	start = clock();
	while (true)
	{
		nodeCount = 0;
		IdaStar(depth, puzzle, lastMove, blank, dir);
		end = clock();
		totalCount += nodeCount;
		printf("Depth = %d, Node Count = %10u, Total Count = %10u, time = %6.2f\n",
			depth, nodeCount, totalCount, (end - start) / 1000.);
		if (isSolved)
		{
			printf("Total Node Count = %u, shortest path length = %d, time = %.2f\n",
				totalCount, depth, (end - start) / 1000.);
			PrintPath(depth);
			return;
		}
		depth += 2;
	}
}

void ExpandNode(priority_queue<Node, vector<Node>, greater<Node>> &open, const Node &nd)
{
	Node pNode = nd;
	char lastMove = pNode.nodeInfo.move;
	char blank = pNode.nodeInfo.blank;
	for (int move = 0; move < 4; ++move)
	{
		if(lastMove == -1 || (move + lastMove) % 4 != 1)
		{
			char newBlank = movableTable[blank][move];
			if(newBlank == -1) continue;

			pNode.board[blank] = pNode.board[newBlank];
			pNode.board[newBlank] = 0; 

			nodeCount += 1;
			uc h = GetHeuristic(pNode.board);
			uc g = pNode.g + 1;
			
			Node node(pNode.board, newBlank, move, nodeCount, pNode.nodeInfo.id, g + h, g);
			open.push(node);
			
			pNode.board[newBlank] = pNode.board[blank];
			pNode.board[blank] = 0; 
		}
	}
}

void MakePath(vector<NodeInfo> &vni, char* puzzle, char depth)
{
	bool isFirst = true;
	NodeInfo temp, info;
	while (!vni.empty())
	{
		temp = vni.back();
		if (isFirst || temp.id == info.parent)
		{
			if(temp.move % 2) temp.move -= 1;
			else temp.move += 1;
			char newBlank = movableTable[temp.blank][temp.move];
			
			puzzle[temp.blank] = puzzle[newBlank];
			puzzle[newBlank] = 0; 
			path[--depth] = puzzle[temp.blank];

			info = temp;
			isFirst = false;
		}
		vni.pop_back();
	}
	//printf("charest path length = %d\n", moveList.size());
}

char AStar(char* puzzle)
{
	time_t start = clock();
	ui showInfo = 10;
	uc g = 0;
	uc h = GetHeuristic(puzzle);
	uc f = g + h;
	printf("h = %d\n", h);

	nodeCount = 0;
	ui parent = 0;
	Node node(puzzle, GetBlank(puzzle), -1, nodeCount, parent, f, g);
	vector<NodeInfo> closed;
	priority_queue<Node, vector<Node>, greater<Node>> open;

	ExpandNode(open, node);
	while (!open.empty())
	{
		node = open.top();
		open.pop();

		if (node.g > g) g = node.g;
		if (g - node.g > 12) continue;

		if(node.g > g) g = node.g;
		if(g - node.g > 12) continue;

		if (nodeCount > showInfo || node.f == node.g)
		{
			printf("f = %d, g = %2d, Node Count = %8u, time = %6.2f\n", node.f, node.g, nodeCount, (clock() - start) / 1000.);
			if (showInfo < 10000000) showInfo *= 10;
			else showInfo += 10000000;
		}

		if (node.f == node.g)
		{
			printf("Open List = %u, Close List = %u, Total maked node = %u\n", open.size(), closed.size(), nodeCount);
			PrintPuzzle(node.board);
			closed.push_back(node.nodeInfo);
			MakePath(closed, node.board, node.g);
			return node.g;
		}

		closed.push_back(node.nodeInfo);
		ExpandNode(open, node);
	}
	// http://hochulshin.com/cpp-priority-queue/
	// https://en.cppreference.com/w/cpp/container/priority_queue}
}

void ASTAR(char* puzzle)
{
	time_t start = clock();
	uc depth = AStar(puzzle);
	printf("Elipsed time = %.2f\n", (clock() - start) / 1000.);
	PrintPath(depth);
}

int main()
{
	int  size = 5;
	char puzzle[25];
	char p[25];
	
	srand((ui)time(NULL));
	char dir = rand() % 24;
	
	// https://www.geeksforgeeks.org/check-instance-15-puzzle-solvable/

	while (1)
	{
		while (size < 3 || size > 5)
		{
			printf("Input board size(8 puzzle : 3, 15 puzzle : 4, end = 0)\n");
			scanf("%d", &size);
			if (size == 0) return 0;
		}
		MakeMDTable(size);
		MakeMovableTable(size);
		MakeDirections();
		/*
		for(int i = 0; i < 24; ++i)
		{
			for(int j = 0; j < 4; ++j)
			{
				printf("%2d ", moves[i][j]);
			}
			printf("\n");
		}
		printf("\n");
		*/
		Size = size;

		printf("Input puzzle data\n");
		for (int i = 0; i < size * size; ++i)
		{
			scanf("%d", &puzzle[i]);
		}
		cout << endl;
		PrintPuzzle(puzzle);

		for (int i = 0; i < size * size; ++i)
		{
			p[i] = puzzle[i];
		}

		printf("\n===== IDA Star Algiorithm =====\n");
		//IDAStar(puzzle, dir);

		printf("\n===== A Star Algotithm =====\n");
		for (int i = 0; i < size * size; ++i)
		{
			puzzle[i] = p[i];
		}
		isSolved = false;
		ASTAR(puzzle);
	}

	return 0;
}
