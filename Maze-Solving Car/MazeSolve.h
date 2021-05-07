#pragma once
#include <vector>
#include <stack>
#include <string>
#include <stdio.h>
#include <exception>
#include <iostream>
#include <random>
#include <chrono> 



namespace MazeSolve
{

	class MazeGenerator;
	class RecursiveSubdivider;
	class MazeSolver;
	class FloodFill;
	class Cell;
	class Dijkstra;
	class AStar;
	class Node;
	class Space;
	class Maze;
	class Car;
	class Empty;

	class MazeGenerator
	{
	protected:
		std::vector<std::vector<Space*>>* mazeSpaces;
	public:
		MazeGenerator(Maze* maze);
		void setMazeSpaces(std::vector<std::vector<Space*>>* spaces);
		virtual ~MazeGenerator(void);
		virtual void generateMaze(void);
	};

	class RecursiveSubdivider : public MazeGenerator
	{
	private:
		class Chamber
		{
		private:
			RecursiveSubdivider* parent;
			int x1, y1, x2, y2;
		public:
			Chamber(int x1, int y1, int x2, int y2, RecursiveSubdivider* parent);
			int getXSize(void);
			int getYSize(void);
			std::vector<Chamber*> HDivide(int minSize);
			std::vector<Chamber*> VDivide(int minSize);
		};
		int minChamberSize;
		std::vector<Chamber*> chambers;
	public:
		~RecursiveSubdivider(void);
		RecursiveSubdivider(Maze* maze, int minSize = 2);
		void generateMaze(void);
	};

	class MazeSolver
	{
	protected:
		Car* parentCar;
	public:
		MazeSolver(void);
		virtual ~MazeSolver(void);
		void setParentCar(Car* car);
		virtual void MoveToNext(void);
	};

	class FloodFill : public MazeSolver
	{
	private:
		std::vector<std::vector<Cell*>> floodMap;
		std::vector<std::vector<Cell*>> generateFloodMap(void);
		std::vector<Cell*> GetAdjacentCells(int x, int y);

	public:
		FloodFill(Maze* maze);
		~FloodFill(void);
		void MoveToNext();
	};


	class Cell
	{
	private:
		int floodValue;
		int x;
		int y;
	public:
		Cell(int value, int x, int y);
		int GetFloodValue();
		void incValue(int amount = 1);
		void decValue(int amount = 1);
		void setValue(int amount);
		int getX(void);
		int getY(void);
	};


	class Dijkstra : public MazeSolver
	{
	private:
		std::vector <std::vector<Node*>> nodes;
		std::vector<Node*> unvisitedNodes;
		std::stack<Node*> route;
		Node* currentNode;
		Node* goalNode;
		void findShortestPath(void);
	public:
		void MoveToNext(void);
		Dijkstra(Maze* maze);
		Node* getClosestedUnvisitedNode(void);
	};

	class AStar : public MazeSolver
	{
	private:
		std::vector <std::vector<Node*>> nodes;
		std::vector<Node*> unvisitedNodes;
		std::stack<Node*> route;
		Node* currentNode;
		Node* goalNode;
		void findShortestPath(void);
	public:
		void MoveToNext(void);
		AStar(Maze* maze);
		Node* getClosestedUnvisitedNode(void);
	};

	class Node
	{
	private:
		Node* previousNode;
		std::vector<Node*> neighbours;
		int weight;
		int distance;
		int x;
		int y;
	public:
		std::vector<Node*>* getNeighbours(void);
		void setDistance(int newDist);
		int getDistance(void);
		int getX(void);
		int getY(void);
		Node* getNearestNeighbour(void);
		Node* getPrevious(void);
		void setPrevious(Node* newPrevious);
		Node(int X, int Y, int weight = 0);
		int getWeight(void);

	};



	class Space
	{
	protected:
		bool Passable;
		std::string Colour;
	public:
		bool GetPassable(void);
		std::string GetColour(void);
	};

	class Maze
	{
	private:
		std::vector<std::vector<Space*>> spaces;
		MazeGenerator* mazeGenerator;
		int xSize;
		int ySize;
		int goalX;
		int goalY;
		Car* car;
	public:
		~Maze(void);
		Maze(int xSize, int ySize, int goalX, int goalY);
		Space GetSpace(int x, int y);
		void SetSpace(int x, int y, Space* newSpace);
		void Step(void);
		void setGenerator(MazeGenerator* mazeGenerator);
		void setSolver(MazeSolver* mazeSolver);
		std::vector<std::vector <Space*>>* getSpaces(void);
		Car* GetCar(void);
		int getXSize(void);
		int getYSize(void);
		int getGoalX(void);
		int getGoalY(void);
	};

	class Car
	{
	private:
		int xPos;
		int yPos;
		Maze* parentMaze;
		MazeSolver* mazeSolver;
	public:
		~Car(void);
		Car(Maze* maze);
		void SetPos(int x, int y);
		void setMazeSolver(MazeSolver* mazeSolver);
		int GetX(void);
		int GetY(void);
		void MoveToNext(void);
		Maze* getParentMaze(void);
	};

	class Empty : public Space
	{
	public:
		Empty();
	};

	class Wall : public Space
	{
	public:
		Wall();
	};

	class Goal : public Space
	{
	public:
		Goal();
	};
}