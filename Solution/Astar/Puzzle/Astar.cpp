# include <iostream>
# include <stdlib.h>
# include <vector>
# include <ctime>
# include <string.h>
# include <algorithm>
# include <queue>

# pragma warning(disable : 4996)
using namespace std;

typedef unsigned int ui;
short Size;

struct Point {
	short x;
	short y;

	Point(short x = 0, short y = 0) {
		this->x = x;
		this->y = y;
	}

	bool operator !=(const Point& Other) const
	{
		return (x != Other.x || y != Other.y);
	}

	void Print()
	{
		if (x == 0 && y == 1) cout << "D ";
		else if (x == 0 && y == -1) cout << "U ";
		else if (x == 1 && y == 0) cout << "R ";
		else cout << "L ";
	}
};

struct NodeInfo {
	ui id;
	ui parent;
	Point move;
};

struct Node {
	short board[5][5];
	Point blank;
	NodeInfo nodeinfo;
	short f;
	short g;

	Node(short(*bd)[5], Point blk, Point move, ui id, ui parent, short f = 0, short g = 0) {
		CopyBoard(board, bd);
		blank = blk;
		nodeinfo.move = move;
		nodeinfo.id = id;
		nodeinfo.parent = parent;
		this->f = f;
		this->g = g;
	}

	bool operator >(const Node& other) const
	{
		return f > other.f;
	}

	void CopyBoard(short(*dst)[5], const short(*src)[5]) {
		for (short i = 0; i < Size; ++i) {
			for (short j = 0; j < Size; ++j) {
				dst[i][j] = src[i][j];
			}
		}
	}
};

class Solver {
private:
	short board[5][5];
	short Origin_board[5][5];
	short size;
	ui nodeCount;
	bool isSolved;

	Point blank;
	Point Origin_blank;
	Point lastMove;
	Point UP;
	Point DOWN;
	Point LEFT;
	Point RIGHT;

	vector<Point> moveList;

	void GetBlank();
	void MoveTile(short x, short y, Point move, short(*board)[5]);
	void Print(short(*board)[5]);
	void GetBoard(short* strBoard);
	short GetInvCount(short(*board)[5]);
	bool isSolvable(short(*board)[5]);
	short GetHeuristic(short(*board)[5]);
	short GetDistance(short*, short md, short k);
	bool isValidMove(short x, short y, Point move);
	vector<Point> GetValidMove(short x, short y);
	void MakePath(vector<NodeInfo>& vni, const NodeInfo& ni);
	void ExpandNode(priority_queue<Node, vector<Node>, greater<Node>>& open, const Node& pNode);
	void PrintPath();

	void AStar();

public:
	Solver(short size, short* board);
	~Solver();
	
	
	void SetGame(short* board);
	void ASTAR();
	void checkPath(char ipt[], short(*board)[5]);
};

Solver::Solver(short size, short* board) {
	UP = Point(0, -1);
	DOWN = Point(0, 1);
	LEFT = Point(-1, 0);
	RIGHT = Point(1, 0);
	this->size = size;
	Size = size;
	SetGame(board);
}

Solver::~Solver() {
}

void Solver::SetGame(short* strBoard) {
	nodeCount = 0;
	isSolved = false;	
	lastMove = Point();

	GetBoard(strBoard);
	GetBlank();
	Origin_blank.x = blank.x; Origin_blank.y = blank.y;
	Print(this->board); printf("\n");
}

void Solver::GetBoard(short* sBoard) {
	for (short y = 0; y < size; ++y) {
		for (short x = 0; x < size; ++x) {
			board[y][x] = sBoard[x + y * size];
			Origin_board[y][x] = sBoard[x + y * size];
		}
	}
}

void Solver::Print(short(*board)[5]) {
	for (short y = 0; y < size; ++y) {
		for (short x = 0; x < size; ++x) {
			printf("%3d", board[y][x]);
		}
		printf("\n");
	}
}

void Solver::PrintPath() {
	for (auto x : moveList) {
		x.Print();
	}
	moveList.clear();
	printf("\n");
}

void Solver::GetBlank() {
	for (short y = 0; y < size; ++y) {
		for (short x = 0; x < size; ++x) {
			if (board[y][x] == 0) {
				blank.x = x;
				blank.y = y;
			}
		}
	}
}

short Solver::GetInvCount(short (*board)[5])
{
	int inv_count = 0;
	for (short i = 0; i < size * size; i++) {
		for (short j = i + 1; j < size * size; j++) {
			if (board[i/size][i%size] && board[j/size][j%size] && board[i/size][i%size] > board[j/size][j%size])
				inv_count++;
		}
	}
	return inv_count;
}

bool Solver::isSolvable(short (*board)[5]) {
	short invCount = GetInvCount(board);

	if (size & 1)
		return !(invCount & 1);

	else
	{		
		if (blank.y & 1)
			return !(invCount & 1);
		else
			return invCount & 1;
	}
}



short Solver::GetHeuristic(short (*board)[5]) {
	short md_3[9][9] = { {0, 0, 0, 0, 0, 0, 0, 0, 0},  // 0
						{0, 1, 2, 1, 2, 3, 2, 3, 4},  // 1
						{1, 0, 1, 2, 1, 2, 3, 2, 3},  // 2
						{2, 1, 0, 3, 2, 1, 4, 3, 2},  // 3
						{1, 2, 3, 0, 1, 2, 1, 2, 3},  // 4
						{2, 1, 2, 1, 0, 1, 2, 1, 2},  // 5
						{3, 2, 1, 2, 1, 0, 3, 2, 1},  // 6
						{2, 3, 4, 1, 2, 3, 0, 1, 2},  // 7
						{3, 2, 3, 2, 1, 2, 1, 0, 1} }; // 8

	short md_4[16][16] = { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 0
						  {0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5, 6},  // 1
						  {1, 0, 1, 2, 2, 1, 2, 3, 3, 2, 3, 4, 4, 3, 4, 5},  // 2
						  {2, 1, 0, 1, 3, 2, 1, 2, 4, 3, 2, 3, 5, 4, 3, 4},  // 3
						  {3, 2, 1, 0, 4, 3, 2, 1, 5, 4, 3, 2, 6, 5, 4, 3},  // 4
						  {1, 2, 3, 4, 0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5},  // 5
						  {2, 1, 2, 3, 1, 0, 1, 2, 2, 1, 2, 3, 3, 2, 3, 4},  // 6
						  {3, 2, 1, 2, 2, 1, 0, 1, 3, 2, 1, 2, 4, 3, 2, 3},  // 7
						  {4, 3, 2, 1, 3, 2, 1, 0, 4, 3, 2, 1, 5, 4, 3, 2},  // 8
						  {2, 3, 4, 5, 1, 2, 3, 4, 0, 1, 2, 3, 1, 2, 3, 4},  // 9
						  {3, 2, 3, 4, 2, 1, 2, 3, 1, 0, 1, 2, 2, 1, 2, 3},  // 10
						  {4, 3, 2, 3, 3, 2, 1, 2, 2, 1, 0, 1, 3, 2, 1, 2},  // 11
						  {5, 4, 3, 2, 4, 3, 2, 1, 3, 2, 1, 0, 4, 3, 2, 1},  // 12
						  {3, 4, 5, 6, 2, 3, 4, 5, 1, 2, 3, 4, 0, 1, 2, 3},  // 13
						  {4, 3, 4, 5, 3, 2, 3, 4, 2, 1, 2, 3, 1, 0, 1, 2},  // 14
						  {5, 4, 3, 4, 4, 3, 2, 3, 3, 2, 1, 2, 2, 1, 0, 1} }; // 15

	short md_5[25][25] = { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 0
						  {0, 1, 2, 3, 4, 1, 2, 3, 4, 5, 2, 3, 4, 5, 6, 3, 4, 5, 6, 7, 4, 5, 6, 7, 8}, // 1
						  {1, 0, 1, 2, 3, 2, 1, 2, 3, 4, 3, 2, 3, 4, 5, 4, 3, 4, 5, 6, 5, 4, 5, 6, 7}, // 2
						  {2, 1, 0, 1, 2, 3, 2, 1, 2, 3, 4, 3, 2, 3, 4, 5, 4, 3, 4, 5, 6, 5, 4, 5, 6}, // 3
						  {3, 2, 1, 0, 1, 4, 3, 2, 1, 2, 5, 4, 3, 2, 3, 6, 5, 4, 3, 4, 7, 6, 5, 4, 5}, // 4
						  {4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 6, 5, 4, 3, 2, 7, 6, 5, 4, 3, 8, 7, 6, 5, 4}, // 5
						  {1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 1, 2, 3, 4, 5, 2, 3, 4, 5, 6, 3, 4, 5, 6, 7}, // 6
						  {2, 1, 2, 3, 4, 1, 0, 1, 2, 3, 2, 1, 2, 3, 4, 3, 2, 3, 4, 5, 4, 3, 4, 5, 6}, // 7
						  {3, 2, 1, 2, 3, 2, 1, 0, 1, 2, 3, 2, 1, 2, 3, 4, 3, 2, 3, 4, 5, 4, 3, 4, 5}, // 8
						  {4, 3, 2, 1, 2, 3, 2, 1, 0, 1, 4, 3, 2, 1, 2, 5, 4, 3, 2, 3, 6, 5, 4, 3, 4}, // 9
						  {5, 4, 3, 2, 1, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 6, 5, 4, 3, 2, 7, 6, 5, 4, 3}, // 10
						  {2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 1, 2, 3, 4, 5, 2, 3, 4, 5, 6}, // 11
						  {3, 2, 3, 4, 5, 2, 1, 2, 3, 4, 1, 0, 1, 2, 3, 2, 1, 2, 3, 4, 3, 2, 3, 4, 5}, // 12
						  {4, 3, 2, 3, 4, 3, 2, 1, 2, 3, 2, 1, 0, 1, 2, 3, 2, 1, 2, 3, 4, 3, 2, 3, 4}, // 13
						  {5, 4, 3, 2, 3, 4, 3, 2, 1, 2, 3, 2, 1, 0, 1, 4, 3, 2, 1, 2, 5, 4, 3, 2, 3}, // 14
						  {6, 5, 4, 3, 2, 5, 4, 3, 2, 1, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 6, 5, 4, 3, 2}, // 15
						  {3, 4, 5, 6, 7, 2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 1, 2, 3, 4, 5}, // 16
						  {4, 3, 4, 5, 6, 3, 2, 3, 4, 5, 2, 1, 2, 3, 4, 1, 0, 1, 2, 3, 2, 1, 2, 3, 4}, // 17
						  {5, 4, 3, 4, 5, 4, 3, 2, 3, 4, 3, 2, 1, 2, 3, 2, 1, 0, 1, 2, 3, 2, 1, 2, 3}, // 18
						  {6, 5, 4, 3, 4, 5, 4, 3, 2, 3, 4, 3, 2, 1, 2, 3, 2, 1, 0, 1, 4, 3, 2, 1, 2}, // 19
						  {7, 6, 5, 4, 3, 6, 5, 4, 3, 2, 5, 4, 3, 2, 1, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1}, // 20
						  {4, 5, 6, 7, 8, 3, 4, 5, 6, 7, 2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4}, // 21
						  {5, 4, 5, 6, 7, 4, 3, 4, 5, 6, 3, 2, 3, 4, 5, 2, 1, 2, 3, 4, 1, 0, 1, 2, 3}, // 22
						  {6, 5, 4, 5, 6, 5, 4, 3, 4, 5, 4, 3, 2, 3, 4, 3, 2, 1, 2, 3, 2, 1, 0, 1, 2}, // 23
						  {7, 6, 5, 4, 5, 6, 5, 4, 3, 4, 5, 4, 3, 2, 3, 4, 3, 2, 1, 2, 3, 2, 1, 0, 1} }; // 24

	short md = 0;
	short x, y;

	for (y = 0; y < size; ++y) {
		for (x = 0; x < size; ++x) {
			if (size == 3) {
				md += md_3[board[y][x]][x + y * size];
			}
			else if (size == 4) {
				md += md_4[board[y][x]][x + y * size];
			}
			else {
				md += md_5[board[y][x]][x + y * size];
			}
		}

	}

	short i, j;
	short temp[5];

	for (i = 0, x = 1; i < size; ++i, ++x) {
		for (j = 0; j < size; ++j) {
			temp[j] = board[i][j];
		}
		short k = 0;
		for (y = 0; y < size - k; ++y) {
			if (temp[y] > size * x || temp[y] == 0 || temp[y] <= size * (x - 1)) {
				memmove(temp + y, temp + y + 1, sizeof(short) * (size - y - 1));
				k++; y--;
			}
		}
		md = GetDistance(temp, md, size - k);
	}

	for (i = 0, x = 1; i < size; ++i, ++x) {
		for (j = 0; j < size; ++j) {
			temp[j] = board[j][i];
		}
		short k = 0;
		for (y = 0; y < size - k; ++y)
		{
			if (temp[y] == x || temp[y] == x + size || temp[y] == x + size * 2 || temp[y] == x + size * 3 || temp[y] == x + size * 4)
			{
				continue;
			}
			else
			{
				memmove(temp + y, temp + y + 1, sizeof(short) * (size - y - 1));
				k++; y--;
			}
		}
		md = GetDistance(temp, md, size - k);
	}
	return md;
}

short Solver::GetDistance(short* row, short md, short nums) {
	if (nums > 1) {
		if (nums == 2) {
			if (row[0] > row[1]) 
				md += 2;
		}
		else if (nums == 3) {
			if (row[0] > row[1] || row[0] > row[2]) 
				md += 2;
			if (row[1] > row[2]) 
				md += 2;
		}
		else if (nums == 4) {
			if (row[0] > row[1] || row[0] > row[2] || row[0] > row[3])
				md += 2;
			if (row[1] > row[2] || row[1] > row[3])
				md += 2;
			if (row[2] > row[3])
				md += 2;
		}
		else if (nums == 5) {
			if (row[0] > row[1] || row[0] > row[2] || row[0] > row[3] || row[0] > row[4])
				md += 2;
			if (row[1] > row[2] || row[1] > row[3] || row[1] > row[4])
				md += 2;
			if (row[2] > row[3] || row[2] > row[4])
				md += 2;
			if (row[3] > row[4])
				md += 2;
		}
	}
	return md;
}

bool Solver::isValidMove(short x, short y, Point move) {
	x += move.x;
	y += move.y;
	return (x >= 0 && x < size && y >= 0 && y < size);
}

vector<Point> Solver::GetValidMove(short x, short y) {
	Point moves[] = { LEFT, UP, DOWN, RIGHT };
	vector<Point> validMoves;
	for (short i = 0; i < sizeof(moves) / sizeof(moves[0]); i++) {
		if (moves[i] != lastMove) {
			if (isValidMove(x, y, moves[i])) {
				validMoves.push_back(moves[i]);
			}
		}
	}
	return validMoves;
}

void Solver::MoveTile(short x, short y, Point move, short(*board)[5]) {
	short dx = move.x;
	short dy = move.y;
	board[y][x] = board[y + dy][x + dx];
	board[y + dy][x + dx] = 0;
	blank = Point(x + dx, y + dy);
}

void Solver::MakePath(vector<NodeInfo>& vni, const NodeInfo& ni) {
	NodeInfo temp, info = ni;
	if (!moveList.empty()) moveList.clear();
	moveList.push_back(ni.move);
	while (!vni.empty()) {
		temp = vni.back();
		if (temp.id == info.parent) {
			moveList.push_back(temp.move);
			info = temp;
		}
		vni.pop_back();
	}
	reverse(moveList.begin(), moveList.end());
	printf("Path length = %d\n", moveList.size());
}

void Solver::checkPath(char ipt[], short(*board)[5]) {
	char ipt[1000];
	vector<Point> solution_Point;

	blank.x = Origin_blank.x; blank.y = Origin_blank.y;


	for (short move = 0; move != '\0'; move++) {
		if (ipt[move] == 'U') solution_Point.push_back(UP);
		else if (ipt[move] == 'D') solution_Point.push_back(DOWN);
		else if (ipt[move] == 'L') solution_Point.push_back(LEFT);
		else if (ipt[move] == 'R') solution_Point.push_back(RIGHT);
		else continue;
	}

	Print(Origin_board); printf("\n%8c\n%8c\n\n", '|', 'V');

	for (auto move : solution_Point)
		MoveTile(blank.x, blank.y, move, Origin_board);
	
	if (GetHeuristic(Origin_board) == 0) {
		Print(Origin_board); printf("\n");
		cout << "Solution is correct" << endl;
	}
	else {
		Print(Origin_board); printf("\n");
		cout << "Solution is incorrect" << endl;
	}

	return;
}

int cnt = 0;
void Solver::ExpandNode(priority_queue<Node, vector<Node>, greater<Node>>& open, const Node& pNd) {
	Point None(0, 0);
	Node pNode = pNd;
	Point blk = pNode.blank;
	Point p = pNode.nodeinfo.move;

	lastMove = Point(-p.x, -p.y);
	vector<Point> moves = GetValidMove(blk.x, blk.y);
	for (auto move : moves) {
		MoveTile(blk.x, blk.y, move, pNode.board);
		nodeCount += 1;
		short h = GetHeuristic(pNode.board);
		short g = pNode.g + 1;
		Node node(pNode.board, blank, move, nodeCount, pNode.nodeinfo.id, g + h, g);
		open.push(node);
		MoveTile(blank.x, blank.y, Point(-move.x, -move.y), pNode.board);
	}
}

void Solver::AStar() {
	time_t start = clock();
	ui showinfo = 1000000;
	short g = 0;
	short h = GetHeuristic(board);
	short f = g + h;
	printf("h = %d\n", h);

	nodeCount = 0;
	ui parent = 0;
	Node node(board, blank, Point(0, 0), nodeCount, parent, f, g);
	vector<NodeInfo> closed;
	priority_queue<Node, vector<Node>, greater<Node>> open;

	if (!isSolvable(board)) {
		cout << "Not Solvable" << endl; printf("\n");
		return;
	}
	else cout << "Solvable" << endl; printf("\n");

	if (h == 0) {
		printf("Open List = %u, Close List = %u, Total maked node = %u\n", open.size(), closed.size(), nodeCount);
		Print(node.board); printf("\n");
		return;
	}
	else ExpandNode(open, node);

	while (!open.empty()) {
		node = open.top();
		open.pop();

		if (showinfo > 0) {
			if (node.g > g) g = node.g;
			if (g - node.g > 12) continue;
		}

		if (nodeCount > showinfo || node.f == node.g) {
			printf("f = %d, g = %2d, Node Count = %8u, time = %6.2f\n", node.f, node.g, nodeCount, (clock() - start) / 1000.);
			showinfo += 1000000;
		}

		if (node.f == node.g) {
			printf("\n");
			printf("Open List = %u, Close List = %u, Total maked node = %u\n", open.size(), closed.size(), nodeCount);
			Print(node.board);
			MakePath(closed, node.nodeinfo);
			return;
		}

		closed.push_back(node.nodeinfo);
		ExpandNode(open, node);
	}
}

void Solver::ASTAR() {
	time_t start = clock();
	AStar();
	printf("time = %.2f\n", (clock() - start) / 1000.);
	PrintPath();
}

int main()
{
	short puzzle[25] = {0, };
	short size = 5;
	
	printf("Input puzzle data\n");
	for (int i = 0; i < size * size; ++i) {
		scanf("%hd", &puzzle[i]);
	}	

	printf("\n");
	for (int i = 0; i < size * size; ++i) {
		if (puzzle[i] == 25) puzzle[i] = 0;
	}

	Solver sol((short)(size), puzzle);
	sol.ASTAR();

	return 0;
}