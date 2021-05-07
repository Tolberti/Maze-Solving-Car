#include "MazeSolve.h"

using namespace MazeSolve;

MazeGenerator::MazeGenerator(Maze* maze)
{
	this->mazeSpaces = maze->getSpaces();
	return;
}

 MazeGenerator::~MazeGenerator(void) // The base MazeGenerator class doesn't have any memory it needs to free, this is just to satisfy the virtual declaration.
{
	 return;
}

void MazeGenerator::generateMaze(void)
{
	return;
}

void MazeGenerator::setMazeSpaces(std::vector<std::vector<Space*>>* spaces)
{
	this->mazeSpaces = spaces;
}

RecursiveSubdivider::Chamber::Chamber(int x1, int y1, int x2, int y2, RecursiveSubdivider* parent)
{
	this->parent = parent; 
	this->x1 = x1;
	this->y1 = y1;
	this->x2 = x2;
	this->y2 = y2;
}

int RecursiveSubdivider::Chamber::getXSize(void)
{
	return abs(this->x2 - this->x1);
}

int RecursiveSubdivider::Chamber::getYSize(void)
{
	return abs(this->y2 - this->y1);
}

std::vector<RecursiveSubdivider::Chamber*> RecursiveSubdivider::Chamber::HDivide(int minSize) // Horizontally divide a chamber at a randomly chosen Y
{
	std::vector<Chamber*> returnVector;
	if (this->getYSize() <= minSize) return returnVector;
	if (this->getXSize() == 0) return returnVector;
	std::default_random_engine randomGen((std::chrono::system_clock::now().time_since_epoch()).count());
	std::uniform_int_distribution<int> randomY(y1+1, y2-1);
	std::uniform_int_distribution<int> randomX(x1, x2);
	int yPosition = randomY(randomGen);
	while (yPosition % 2 != 1) // We only want walls to be drawn on odd rows
	{
		yPosition = randomY(randomGen);
	}
	int gapPos = randomX(randomGen);
	while (gapPos % 2 != 0) // ...and gaps to be on even columns
	{
		gapPos = randomX(randomGen);
	}
	for (int i = this->x1; i <= this->x2; i++)
	{
		delete (*parent->mazeSpaces)[i][yPosition];
		(*parent->mazeSpaces)[i][yPosition] = new Wall();
	}
	delete (*parent->mazeSpaces)[gapPos][yPosition];
	(*parent->mazeSpaces)[gapPos][yPosition] = new Empty();
	returnVector.push_back(new Chamber(x1, y1, x2, yPosition-1, this->parent));
	returnVector.push_back(new Chamber(x1, yPosition+1, x2, y2, this->parent));
	return returnVector;
}

std::vector<RecursiveSubdivider::Chamber*> RecursiveSubdivider::Chamber::VDivide(int minSize) // Vertically divide a chamber at a randomly chosen X
{
	std::vector<Chamber*> returnVector;
	if (this->getXSize() <= minSize || this->getXSize() <= 2) return returnVector;
	if (this->getYSize() == 0) return returnVector;
	std::default_random_engine randomGen((std::chrono::system_clock::now().time_since_epoch()).count());
	std::uniform_int_distribution<int> randomY(y1, y2);
	std::uniform_int_distribution<int> randomX(x1+1, x2-1);
	int xPosition = randomX(randomGen); 
	while (xPosition % 2 != 1) // We only want walls to be drawn on odd columns
	{
		xPosition = randomX(randomGen);
	}
	int gapPos = randomY(randomGen);
	while (gapPos % 2 != 0) // ...and gaps to be on even rows
	{
		gapPos = randomY(randomGen);
	}
	for (int i = this->y1; i <= this->y2; i++)
	{
		delete (*parent->mazeSpaces)[xPosition][i];
		(*parent->mazeSpaces)[xPosition][i] = new Wall();
	}
	delete (*parent->mazeSpaces)[xPosition][gapPos];
	(*parent->mazeSpaces)[xPosition][gapPos] = new Empty();
	returnVector.push_back(new Chamber(x1, y1, xPosition - 1, y2, this->parent));
	returnVector.push_back(new Chamber(xPosition + 1, y1, x2, y2, this->parent));
	return returnVector;
}
RecursiveSubdivider::RecursiveSubdivider(Maze* maze, int minSize) : MazeGenerator(maze)
{
	this->minChamberSize = minSize;
}

void RecursiveSubdivider::generateMaze(void)
{
	this->chambers.push_back(new Chamber(0, 0, mazeSpaces->size() - 1, (*mazeSpaces)[0].size() - 1, this));
	std::vector<Chamber*> newChambers;
	while (!chambers.empty())
	{
		if (chambers[0]->getYSize() > chambers[0]->getXSize()) newChambers = chambers[0]->HDivide(minChamberSize);
		else newChambers = chambers[0]->VDivide(minChamberSize);
		delete chambers[0]; // Erasing a vector element doesn't call its destructor
		chambers.erase(chambers.begin());
		chambers.insert(chambers.begin(), newChambers.begin(), newChambers.end());
	}
	return;
}

RecursiveSubdivider::~RecursiveSubdivider(void) // Make sure to delete all the chambers when you're done!
{
	for (Chamber* chamber : this->chambers)
	{
		delete chamber;
	}
}

Maze::Maze(int xSize, int ySize, int goalX, int goalY) // Maze must be at least 8x8, goal must be within specified dimensions
{
	if (xSize < 8 || ySize < 8)
	{
		throw std::invalid_argument("Mazes must be 8x8 or greater!");
	}
	if (goalX >= xSize || goalY >= ySize)
	{
		throw std::invalid_argument("The goal can't be outside the maze!");
	}
	for (int i = 0; i < xSize; i++) // Populate the maze with empty spaces and the goal before running the actual generator
	{
		this->spaces.push_back(std::vector<Space*>());
		for (int j = 0; j < ySize; j++)
		{
			this->spaces[i].push_back(new Empty());
		}
	}
	this->spaces[goalX][goalY] = new Goal();
	this->xSize = xSize;
	this->ySize = ySize;
	this->goalX = goalX;
	this->goalY = goalY;
	this->car = new Car(this);
	
}

Maze::~Maze(void)
{
	for (std::vector<Space*> spaceVector : this->spaces)
	{
		for (Space* space : spaceVector)
		{
			delete space;
		}
	}
	delete this->car;
	delete this->mazeGenerator;
}

void Maze::Step(void)
{
	this->car->MoveToNext();
}

std::vector<std::vector <Space*>>* Maze::getSpaces(void)
{
	return &this->spaces;
}

Space Maze::GetSpace(int x, int y) // It's up to the caller to handle the exception if they try to access a non-existent element
{
	if (x < 0 || y < 0 || x >= spaces.size() || y >= spaces[x].size())
	{
		throw std::out_of_range("Index out of range");
	}
	return *this->spaces[x][y];
}

void Maze::SetSpace(int x, int y, Space* newSpace) // It's up to the caller to handle the exception if they try to access a non-existent element here too
{
	if (x < 0 || y < 0 || x >= spaces.size() || y >= spaces[x].size())
	{
		throw std::out_of_range("Index out of range");
	}
	delete(this->spaces[x][y]);
	this->spaces[x][y] = newSpace;
}

int Maze::getGoalX(void)
{
	return this->goalX;
}

int Maze::getGoalY(void)
{
	return this->goalY;
}

int Maze::getYSize(void)
{
	return this->ySize;
}

int Maze::getXSize(void)
{
	return this->xSize;
}

Car* Maze::GetCar(void)
{
	return this->car;
}

void Maze::setGenerator(MazeGenerator* mazeGenerator)
{
	this->mazeGenerator = mazeGenerator;
	mazeGenerator->generateMaze();
}

void Maze::setSolver(MazeSolver* mazeSolver)
{
	this->car->setMazeSolver(mazeSolver);
}

Car::Car(Maze* maze)
{
	this->parentMaze = maze;
}

Car::~Car(void)
{
	delete this->mazeSolver;
}

void Car::setMazeSolver(MazeSolver* mazeSolver)
{
	this->mazeSolver = mazeSolver;
}

void Car::SetPos(int x, int y)
{
	this->xPos = x;
	this->yPos = y;
}

int Car::GetX(void)
{
	return this->xPos;
}

int Car::GetY(void)
{
	return this->yPos;
}

void Car::MoveToNext(void) // Try to move to a neighbouring cell with a lower flood value. If there isn't one, update the floodmap.
{
	this->mazeSolver->MoveToNext();
}

Maze* Car::getParentMaze(void)
{
	return this->parentMaze;
}

MazeSolver::MazeSolver(void)
{

}

MazeSolver::~MazeSolver(void)
{

}

void MazeSolver::setParentCar(Car* car)
{
	this->parentCar = car;
}

void MazeSolver::MoveToNext(void)
{
	return;
}

FloodFill::FloodFill(Maze* maze)
{
	this->setParentCar(maze->GetCar());
	floodMap = generateFloodMap();
}

FloodFill::~FloodFill(void)
{
	for (std::vector<Cell*> cellRow : this->floodMap)
	{
		for (Cell* cell : cellRow)
		{
			delete cell;
		}
	}
}

std::vector<std::vector<Cell*>> FloodFill::generateFloodMap(void) // Generate the initial floodmap, values are the Manhattan Distance from the goal
{
	Maze* parentMaze = this->parentCar->getParentMaze();
	std::vector<std::vector<Cell*>> returnVector;
	for (int i = 0; i < parentMaze->getXSize(); i++)
	{
		returnVector.push_back(std::vector<Cell*>());
		for (int j = 0; j < parentMaze->getYSize(); j++)
		{
			int value = abs(i - parentMaze->getGoalX()) + abs(j - parentMaze->getGoalY());
			returnVector[i].push_back(new Cell(value, i, j));
		}
	}

	return returnVector;
}

void FloodFill::MoveToNext(void)
{
	int localCellValue = floodMap[parentCar->GetX()][parentCar->GetY()]->GetFloodValue();
	int xPos = parentCar->GetX();
	int yPos = parentCar->GetY();
	Cell* lowestNeighbour = NULL;
	bool canMove = true;
	if (xPos == parentCar->getParentMaze()->getGoalX() && yPos == parentCar->getParentMaze()->getGoalY()) return; // Don't move if already at goal
	for (Cell* neighbourCell : this->GetAdjacentCells(xPos, yPos)) // Search all neighbouring cells for one with a lower flood value
	{
		if (lowestNeighbour == NULL)
		{
			if (neighbourCell->GetFloodValue() < localCellValue) lowestNeighbour = neighbourCell;
		}
		else
		{
			if (neighbourCell->GetFloodValue() < lowestNeighbour->GetFloodValue()) lowestNeighbour = neighbourCell;
		}
	}
	if (lowestNeighbour != NULL) this->parentCar->SetPos(lowestNeighbour->getX(), lowestNeighbour->getY());
	else
	{
		std::stack<Cell*> cellStack;
		Cell* topCell;
		std::vector<Cell*> adjacentCells;
		cellStack.push(floodMap[xPos][yPos]);
		while (!cellStack.empty())
		{
			lowestNeighbour = NULL;
			topCell = cellStack.top();
			cellStack.pop();
			for (Cell* neighbourCell : this->GetAdjacentCells(topCell->getX(), topCell->getY())) // Get the lowest neighbour
			{
				if (lowestNeighbour == NULL)
				{
					lowestNeighbour = neighbourCell;
				}
				else
				{
					if (neighbourCell->GetFloodValue() < lowestNeighbour->GetFloodValue()) lowestNeighbour = neighbourCell;
				}
			}
			std::cout << "Topcell value: " << topCell->GetFloodValue() << " Neighbour value: " << lowestNeighbour->GetFloodValue() << std::endl;
			if (topCell->GetFloodValue() <= lowestNeighbour->GetFloodValue())
			{
				topCell->setValue(lowestNeighbour->GetFloodValue() + 1);
				for (Cell* neighbourCell : this->GetAdjacentCells(topCell->getX(), topCell->getY()))
				{
					cellStack.push(neighbourCell);
				}
			}

		}
	}


}

std::vector<Cell*> FloodFill::GetAdjacentCells(int x, int y)
{
	std::vector<Cell*> adjacentCells;
	if (x > 0)
	{
		if (this->parentCar->getParentMaze()->GetSpace(x-1,  y).GetPassable())
		{
			adjacentCells.push_back(floodMap[x - 1][y]);
		}
	}
	if (y > 0)
	{
		if (this->parentCar->getParentMaze()->GetSpace(x, y - 1).GetPassable())
		{
			adjacentCells.push_back(floodMap[x][y - 1]);
		}
	}
	if (x < (parentCar->getParentMaze()->getXSize() - 1))
	{
		if (this->parentCar->getParentMaze()->GetSpace(x + 1, y).GetPassable())
		{
			adjacentCells.push_back(floodMap[x + 1][y]);
		}
	}
	if (y < (parentCar->getParentMaze()->getYSize() - 1))
	{
		{
			if (this->parentCar->getParentMaze()->GetSpace(x, y + 1).GetPassable())
			{
				adjacentCells.push_back(floodMap[x][y + 1]);
			}
		}
	}
	std::random_device randomiser;
	std::shuffle(adjacentCells.begin(), adjacentCells.end(), randomiser);
	return adjacentCells;
}

Cell::Cell(int value, int x, int y)
{
	this->floodValue = value;
	this->x = x;
	this->y = y;
}
int Cell::GetFloodValue(void)
{
	return this->floodValue;
}

void Cell::decValue(int amount)
{
	this->floodValue += amount;
}

void Cell::incValue(int amount)
{
	this->floodValue += amount;
}

void Cell::setValue(int amount)
{
	this->floodValue = amount;
}

int Cell::getX(void)
{
	return this->x;
}

int Cell::getY(void)
{
	return this->y;
}

Dijkstra::Dijkstra(Maze* maze)
{
	this->parentCar = maze->GetCar();
	for (int i = 0; i < maze->getXSize(); i++) // Populate the node set with nodes that correspond to spaces in the maze. 
	{
		this->nodes.push_back(std::vector<Node*>());
		for (int j = 0; j < maze->getYSize(); j++)
		{
			if (maze->GetSpace(i, j).GetPassable())
			{
				Node* newNode = new Node(i, j);
				nodes[i].push_back(newNode);
				unvisitedNodes.push_back(newNode);
			}
			else nodes[i].push_back(NULL);
		}
	}
	this->goalNode = nodes[maze->getGoalX()][maze->getGoalY()];
	for (int i = 0; i < nodes.size(); i++) // Now iterate through every node in the vector and connect them to their neighbours
	{
		for (int j = 0; j < nodes[i].size(); j++)
		{
			if (nodes[i][j] == NULL)
			{
				continue;
			}
			else
			{
				for (int iOffset = -1; iOffset <= 1; iOffset += 2)
				{
					try
					{
						if (nodes.at(i + iOffset).at(j) != NULL) nodes[i][j]->getNeighbours()->push_back(nodes[i + iOffset][j]);
					}
					catch (std::out_of_range)
					{
						continue;
					}
				}
				for (int jOffset = -1; jOffset <= 1; jOffset += 2)
				{
					try
					{
						if (nodes.at(i).at(j + jOffset) != NULL) nodes[i][j]->getNeighbours()->push_back(nodes[i][j + jOffset]);
					}
					catch (std::out_of_range)
					{
						continue;
					}
				}
			}
		}
	}
	findShortestPath();
}

Node* Dijkstra::getClosestedUnvisitedNode(void)
{
	Node* closestNode = NULL;
	for (Node* node : unvisitedNodes)
	{
		if (closestNode == NULL) closestNode = node;
		else
		{
			if (node->getDistance() < closestNode->getDistance()) closestNode = node;
		}
	}

	return closestNode;
}

void Dijkstra::findShortestPath(void)
{
	nodes[0][0]->setDistance(0); // The starting node, which is assumed to be (0,0) is always initialised to a distance of 0
	currentNode = nodes[0][0];
	int initialSize = unvisitedNodes.size();
	while (currentNode != goalNode && currentNode->getDistance() != INT_MAX)
	{
		for (Node* neighbourNode : *currentNode->getNeighbours())
		{
			if (neighbourNode->getDistance() == INT_MAX || neighbourNode->getDistance() > currentNode->getDistance() + 1)
			{
				neighbourNode->setPrevious(currentNode);
				neighbourNode->setDistance(currentNode->getDistance() + 1);
			}
		}
		unvisitedNodes.erase(std::find(unvisitedNodes.begin(), unvisitedNodes.end(), currentNode));
		currentNode = getClosestedUnvisitedNode();
	}
	std::cout << "Found solution after examining " << initialSize - unvisitedNodes.size() << " nodes"  << std::endl;

	while (currentNode->getPrevious() != NULL)
	{
		route.push(currentNode);
		currentNode = currentNode->getPrevious();
	}
}

void Dijkstra::MoveToNext(void)
{
	if (route.empty()) return;
	int nextX = route.top()->getX();
	int nextY = route.top()->getY();
	this->parentCar->SetPos(nextX, nextY);
	route.pop();
}

void AStar::MoveToNext(void)
{
	if (route.empty()) return;
	int nextX = route.top()->getX();
	int nextY = route.top()->getY();
	this->parentCar->SetPos(nextX, nextY);
	route.pop();
}

AStar::AStar(Maze* maze)
{
	this->parentCar = maze->GetCar();
	for (int i = 0; i < maze->getXSize(); i++) // Populate the node set with nodes that correspond to spaces in the maze. 
	{
		this->nodes.push_back(std::vector<Node*>());
		for (int j = 0; j < maze->getYSize(); j++)
		{
			if (maze->GetSpace(i, j).GetPassable())
			{
				int nodeWeight = abs(i + maze->getGoalX()) + abs(j + maze->getGoalY());
				Node* newNode = new Node(i, j, nodeWeight);
				nodes[i].push_back(newNode);
				unvisitedNodes.push_back(newNode);
			}
			else nodes[i].push_back(NULL);
		}
	}
	this->goalNode = nodes[maze->getGoalX()][maze->getGoalY()];
	for (int i = 0; i < nodes.size(); i++) // Now iterate through every node in the vector and connect them to their neighbours
	{
		for (int j = 0; j < nodes[i].size(); j++)
		{
			if (nodes[i][j] == NULL)
			{
				continue;
			}
			else
			{
				for (int iOffset = -1; iOffset <= 1; iOffset += 2)
				{
					try
					{
						if (nodes.at(i + iOffset).at(j) != NULL) nodes[i][j]->getNeighbours()->push_back(nodes[i + iOffset][j]);
					}
					catch (std::out_of_range)
					{
						continue;
					}
				}
				for (int jOffset = -1; jOffset <= 1; jOffset += 2)
				{
					try
					{
						if (nodes.at(i).at(j + jOffset) != NULL) nodes[i][j]->getNeighbours()->push_back(nodes[i][j + jOffset]);
					}
					catch (std::out_of_range)
					{
						continue;
					}
				}
			}
		}
	}
	findShortestPath();
}

void AStar::findShortestPath(void)
{
	nodes[0][0]->setDistance(0); // The starting node, which is assumed to be (0,0) is always initialised to a distance of 0
	currentNode = nodes[0][0];
	int initialSize = unvisitedNodes.size();
	while (currentNode != goalNode && currentNode->getDistance() != INT_MAX)
	{
		for (Node* neighbourNode : *currentNode->getNeighbours())
		{
			if (neighbourNode->getDistance() == INT_MAX || neighbourNode->getDistance() + neighbourNode->getWeight() > currentNode->getDistance() + neighbourNode->getWeight() + 1)
			{
				neighbourNode->setPrevious(currentNode);
				neighbourNode->setDistance(currentNode->getDistance() + neighbourNode->getWeight() + 1);
			}
		}
		unvisitedNodes.erase(std::find(unvisitedNodes.begin(), unvisitedNodes.end(), currentNode));
		currentNode = getClosestedUnvisitedNode();
	}
	std::cout << "Found solution after examining " << initialSize - unvisitedNodes.size() << " nodes" << std::endl;
	while (currentNode->getPrevious() != NULL)
	{
		route.push(currentNode);
		currentNode = currentNode->getPrevious();
	}
}

Node* AStar::getClosestedUnvisitedNode()
{
	Node* closestNode = NULL;
	for (Node* node : unvisitedNodes)
	{
		if (node->getDistance() == INT_MAX) continue;
		if (closestNode == NULL) closestNode = node;
		else
		{
			if (node->getDistance() + node->getWeight() < closestNode->getDistance() + closestNode->getWeight()) closestNode = node;
		}
	}

	if (closestNode == NULL) throw(std::runtime_error("No connected unvisited nodes available!"));
	return closestNode;
}

Node::Node(int X, int Y, int weight)
{
	this->x = X;
	this->y = Y;
	this->distance = INT_MAX;
	this->previousNode = NULL;
	this->weight = weight;
}

void Node::setDistance(int newDist)
{
	this->distance = newDist;
}

int Node::getDistance(void)
{
	return this->distance;
}

Node* Node::getPrevious(void)
{
	return this->previousNode;
}

void Node::setPrevious(Node* newPrevious)
{
	this->previousNode = newPrevious;
}

std::vector<Node*>* Node::getNeighbours(void)
{
	return &this->neighbours;
}

Node* Node::getNearestNeighbour(void)
{
	Node* nearestNeighbour = this->neighbours[0];
	for (Node* node : neighbours)
	{
		if (node->getDistance() + node->weight < nearestNeighbour->getDistance() + nearestNeighbour->getWeight()) nearestNeighbour = node;
	}
	return nearestNeighbour;
}

int Node::getX(void)
{
	return this->x;
}

int Node::getY(void)
{
	return this->y;
}

int Node::getWeight(void)
{
	return this->weight;
}

bool Space::GetPassable(void)
{
	return this->Passable;
}

std::string Space::GetColour(void)
{
	return this->Colour;
}

Empty::Empty()
{
	this->Colour = "White";
	this->Passable = true;
}

Wall::Wall()
{
	this->Colour = "Black";
	this->Passable = false;
}

Goal::Goal()
{
	this->Colour = "Green";
	this->Passable = true;
}

