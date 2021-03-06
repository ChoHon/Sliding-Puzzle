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
short static num = 1;

struct Point
{
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
		else if (x == -1 && y == 0) cout << "L ";
	}
};

struct NodeInfo {
	ui id;
	ui parent;
	Point move; // 현재 Node의 blank 바로 전 move
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
	short size;
	ui nodeCount; // Node가 만들어진 갯수, Node ID
	bool isSolved; 

	Point blank;
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
}

void Solver::GetBoard(short* sBoard) {
	for (short y = 0; y < size; ++y) {
		for (short x = 0; x < size; ++x) {
			board[y][x] = sBoard[x + y * size];
		}
	}
}

void Solver::Print(short(*board)[5]) {
	for (short y = 0; y < size; ++y) {
		for (short x = 0; x < size; ++x) {
			if (board[y][x] == 0) printf("%3d", 25);
			else printf("%3d", board[y][x]);
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

bool Solver::isSolvable(short (*board)[5])
{
	int inv_count = 0;
	for (short i = 0; i < size * size; i++) {
		for (short j = i + 1; j < size * size; j++) {
			if (board[i/size][i%size] && board[j/size][j%size] && board[i/size][i%size] > board[j/size][j%size])
				inv_count++;
		}
	}
	return !(inv_count % 2);
}

short Solver::GetHeuristic(short (*board)[5]) {
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
				md += md_5[board[y][x]][x + y * size];
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
	printf("%d ", moveList.size());
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
	ui showinfo = 9000000;
	short g = 0;
	short h = GetHeuristic(board);
	short f = g + h;

	nodeCount = 0;
	ui parent = 0;
	Node node(board, blank, Point(0, 0), nodeCount, parent, f, g);
	vector<NodeInfo> closed;
	priority_queue<Node, vector<Node>, greater<Node>> open;

	if (!isSolvable(board)) {
		cout << -1;
		return;
	}

	if (h == 0) {
		cout << 0;
		return;
	}
	else ExpandNode(open, node);

	while (!open.empty()) {
		node = open.top();
		open.pop();

		if (nodeCount > 9000000) {
			if (node.g > g) g = node.g;
			if (g - node.g > 12) continue;
		}

		if (nodeCount > showinfo) {
			printf(" *** ");
			showinfo += 1000000;
		}

		if (node.f == node.g) {
			MakePath(closed, node.nodeinfo);
			return;
		}

		closed.push_back(node.nodeinfo);
		ExpandNode(open, node);
	}
}

void Solver::ASTAR() {
	AStar();
	PrintPath();
}

void puzzleSolution(void) {
	short puzzle[25][25];
	short t;
	short size = 5;

	scanf("%hd", &t);
	
	for (int j = 0; j < t; j++) {
		for (int i = 0; i < size * size; ++i) {
			scanf("%hd", &puzzle[j][i]);
		}
	}

	printf("\n");
	for (int j = 0; j < t; j++) {
		for (int i = 0; i < size * size; ++i) {
			if (puzzle[j][i] == 25) puzzle[j][i] = 0;
		}
	}

	for (int j = 0; j < t; j++) {
		Solver sol((short)(size), puzzle[j]);
		printf("#%d ", num++);
		sol.ASTAR();
	}
}

int main()
{
	puzzleSolution();

	return 0;
}