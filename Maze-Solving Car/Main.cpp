#include <Windows.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <tuple>
#include <fstream>
#include "MazeSolve.h"

using namespace MazeSolve;


class GLFWWrapper
{
private:
	static void mouseClick_callback(GLFWwindow* window, int button, int action, int mods);
	static void error_callback(int error, const char* description);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	double relBlockWidth; // The width of one block, as a percentage of the total window width
	double relBlockHeight; // The height of one block, as a percentage of the total window width
	double absBlockWidth; // Width of one block in pixels
	double absBlockHeight; // Height of one block in pix
	int xSize; // Width of window in blocks
	int ySize; //  Height of window in blocks
	bool rPressed = false;
	bool spacePressed = false;
	bool mouseClicked = false;
	bool showGrid = false;
	int mouseClickedX = 0;
	int mouseClickedY = 0;
	int mouseButtonClicked = 0;
	std::string windowTitle = "Maze-solving car";
	GLFWwindow* window;
public:
	~GLFWWrapper();
	GLFWWrapper(int xSize, int ySize, int xBlockSize = 20, int yBlockSize = 20);
	bool getMouseClicked(void);
	void setMouseClicked(bool newStatus);
	int getMouseClickX(void);
	int getMouseClickY(void);
	int getMouseButtonClicked(void);
	bool getSpacePressed(void);
	void setSpacePressed(bool newStatus);
	bool getRPressed(void);
	void setRPressed(bool newStatus);
	int shouldClose(void);
	void drawBorder(void);
	void drawGrid(void);
	void drawSquare(int x, int y);
	void drawCar(int x, int y);
	void refresh(void);
};

GLFWWrapper::GLFWWrapper(int xMazeSize, int yMazeSize, int xBlockSize, int yBlockSize) // x/yMazeSize are the length/width of the entire maze, in blocks. BlockSizes are the dimensions of one block in pixels.
{
	glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	window = glfwCreateWindow((xMazeSize+2)* xBlockSize, (yMazeSize+2)* yBlockSize, "Maze-solving Car", NULL, NULL); // The +2 is to allow room for maze borders
	glfwMakeContextCurrent(window);
	glfwSetErrorCallback(&this->error_callback);
	glfwSetKeyCallback(window, &this->key_callback);
	glfwSetWindowUserPointer(window, this);
	glfwSetMouseButtonCallback(window, &this->mouseClick_callback);
	relBlockWidth = 2.0 / (xMazeSize + 2);
	relBlockHeight = 2.0 / (yMazeSize + 2); 
	absBlockWidth = xBlockSize;
	absBlockHeight = yBlockSize;
	this->xSize = xMazeSize;
	this->ySize = yMazeSize;
}

GLFWWrapper::~GLFWWrapper()
{
	glfwDestroyWindow(this->window);
}

void GLFWWrapper::mouseClick_callback(GLFWwindow* window, int button, int action, int mods)
{
	GLFWWrapper* wrapper = (GLFWWrapper*)glfwGetWindowUserPointer(window);
	if (button == GLFW_MOUSE_BUTTON_LEFT || GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		mouseX -= wrapper->absBlockWidth;
		mouseY += wrapper->absBlockHeight;
		mouseY = (wrapper->absBlockHeight * (wrapper->ySize + 2)) - mouseY;
		mouseX = mouseX / wrapper->absBlockWidth;
		mouseY = mouseY / wrapper->absBlockHeight;
		std::cout << mouseX << ", " << mouseY << std::endl;
		std::cout << int(mouseX) << ", " << int(mouseY) << std::endl;
		wrapper->mouseClicked = true;
		wrapper->mouseClickedX = mouseX;
		wrapper->mouseClickedY = mouseY;
		wrapper->mouseButtonClicked = button;
	}
}

void GLFWWrapper::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	GLFWWrapper* wrapper = (GLFWWrapper*)glfwGetWindowUserPointer(window);
	if (key == GLFW_KEY_R && action == GLFW_PRESS) wrapper->rPressed = true;
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)	glfwSetWindowShouldClose(window, GLFW_TRUE);
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) wrapper->spacePressed = true;
	if (key == GLFW_KEY_G && action == GLFW_PRESS) wrapper->showGrid ^= 1;
	if (key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		int height = 0;
		int width = 0;
		glfwGetWindowSize(window, &width, &height);
		std::cout << "Width: " << width << " Height: " << height << std::endl;
	}
}

void GLFWWrapper::error_callback(int error, const char* description)
{
	std::cout << description << std::endl;
}

void GLFWWrapper::drawGrid(void)
{
	GLFWWrapper* wrapper = (GLFWWrapper*) glfwGetWindowUserPointer(window);
	if (!wrapper->showGrid) return;
	for (int i = 0; i <= wrapper->ySize; i++)
	{
		glColor3f(0.0, 0.0, 0.0);
		glBegin(GL_LINES);
		glVertex2d(-1, -1 + (i * wrapper->relBlockHeight));
		glVertex2d(1, -1 + (i * wrapper->relBlockHeight));
		glEnd();
		glFlush();
	}
	for (int i = 0; i <= wrapper->xSize; i++)
	{
		glColor3f(0.0, 0.0, 0.0);
		glBegin(GL_LINES);
		glVertex2d(-1 + (i * wrapper->relBlockWidth) , -1);
		glVertex2d(-1 + (i * wrapper->relBlockWidth), 1);
		glEnd();
		glFlush();
	}
}

void GLFWWrapper::drawBorder(void)
{
	for (int x = 0; x <= xSize+1; x++)
	{
		int y = 0;
		glColor3f(0.0, 0.0, 0.0);
		glRectd(-1.0 + x * relBlockWidth, -1.0 + y * relBlockHeight, -1.0 + (x + 1) * relBlockWidth, -1.0 + (y + 1) * relBlockHeight);
		y = (ySize + 1);
		glRectd(-1.0 + x * relBlockWidth, -1.0 + y * relBlockHeight, -1.0 + (x + 1) * relBlockWidth, -1.0 + (y + 1) * relBlockHeight);
	}
	for (int y = 0; y <= ySize + 1; y++)
	{
		int x = 0;
		glColor3f(0.0, 0.0, 0.0);
		glRectd(-1.0 + x * relBlockWidth, -1.0 + y * relBlockHeight, -1.0 + (x + 1) * relBlockWidth, -1.0 + (y + 1) * relBlockHeight);
		x = (xSize + 1);
		glRectd(-1.0 + x * relBlockWidth, -1.0 + y * relBlockHeight, -1.0 + (x + 1) * relBlockWidth, -1.0 + (y + 1) * relBlockHeight);
	}
}

void GLFWWrapper::drawSquare(int x, int y)
{
	x += 1;
	y += 1;
	glRectd(-1.0 + x * relBlockWidth, -1.0 + y * relBlockHeight, -1.0 + (x + 1) * relBlockWidth, -1.0 + (y + 1) * relBlockHeight);
}

void GLFWWrapper::drawCar(int x, int y)
{
	x += 1;
	y += 1;
	glColor3f(1.0, 0.0, 0.0);
	double x1 = -1.0 + (double) x*relBlockWidth + (relBlockWidth / 4);
	double y1 = -1.0 + (double) y * relBlockHeight + (relBlockHeight / 4);
	double x2 = -1.0 + (double)(x + 1) * relBlockWidth - (relBlockWidth / 4);
	double y2 = -1.0 + (double)y * relBlockHeight + (relBlockHeight / 4);
	double x3 = -1.0 + (double)x * relBlockWidth + (relBlockWidth / 2);
	double y3 = -1.0 +  (double)(y + 1) * relBlockHeight - (relBlockWidth / 4);
	glBegin(GL_TRIANGLES);
	glVertex2d(x1, y1);
	glVertex2d(x2, y2);
	glVertex2d(x3, y3);
	glEnd();
	glFlush();
}

int GLFWWrapper::shouldClose(void)
{
	return glfwWindowShouldClose(this->window);
}

void GLFWWrapper::refresh(void)
{
	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
	glfwSwapBuffers(window);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwPollEvents();
}



bool GLFWWrapper::getMouseClicked(void)
{
	return this->mouseClicked;
}

void GLFWWrapper::setMouseClicked(bool newStatus)
{
	this->mouseClicked = newStatus;
}

bool GLFWWrapper::getSpacePressed(void)
{
	return this->spacePressed;
}

int GLFWWrapper::getMouseClickX(void)
{
	return this->mouseClickedX;
}

int GLFWWrapper::getMouseClickY(void)
{
	return this->mouseClickedY;
}

int GLFWWrapper::getMouseButtonClicked(void)
{
	return this->mouseButtonClicked;
}
void GLFWWrapper::setSpacePressed(bool newStatus)
{
	this->spacePressed = newStatus;
}

bool GLFWWrapper::getRPressed(void)
{
	return this->rPressed;
}

void GLFWWrapper::setRPressed(bool newStatus)
{
	this->rPressed = newStatus;
}

bool validateInt(std::string input)
{
	bool isValid = true;
	for (char character : input)
	{
		if (isdigit(character)) continue;
		else
		{
			isValid = false;
			break;
		}
	}
	return isValid;
}

std::tuple<int, int, int, int, int, int> getMazeParameters(void)
{
	int mazeXSize = -1, mazeYSize = -1, goalX = -1, goalY = -1, generator = -1, solver = -1;
	while (mazeXSize == -1)
	{
		std::cout << "Please enter the X dimension of the maze: ";
		std::string inputString;
		std::cin >> inputString;
		if (validateInt(inputString))
		{
			mazeXSize = std::atoi(inputString.c_str());
			if (mazeXSize < 8) mazeXSize = -1;
		}
	}
	while (mazeYSize == -1)
	{
		std::cout << "Please enter the Y dimension of the maze: ";
		std::string inputString;
		std::cin >> inputString;
		if (validateInt(inputString))
		{
			mazeYSize = std::atoi(inputString.c_str());
			if (mazeYSize < 8) mazeYSize = -1;
		}
	}
	while (goalX == -1)
	{
		std::cout << "Please enter the X co-ordinate of the goal: ";
		std::string inputString;
		std::cin >> inputString;
		if (validateInt(inputString))
		{
			goalX = std::atoi(inputString.c_str());
			if (goalX < 1|| goalX >= mazeXSize) goalX = -1;
		}
	}
	while (goalY == -1)
	{
		std::cout << "Please enter the Y co-ordinate of the goal: ";
		std::string inputString;
		std::cin >> inputString;
		if (validateInt(inputString))
		{
			goalY = std::atoi(inputString.c_str());
			if (goalY < 1 || goalY >= mazeYSize) goalY = -1;
		}
	}
	while (goalY == -1)
	{
		std::cout << "Please enter the Y co-ordinate of the goal: ";
		std::string inputString;
		std::cin >> inputString;
		if (validateInt(inputString))
		{
			goalY = std::atoi(inputString.c_str());
			if (goalY < 1 || goalY >= mazeYSize) goalY = -1;
		}
	}
	while (generator == -1)
	{
		{
			std::cout << "Please select a maze generation option.\nEnter 1 for Recursive Subdivider.\n";
			std::string inputString;
			std::cin >> inputString;
			if (validateInt(inputString))
			{
				generator = std::atoi(inputString.c_str());
				switch (generator)
				{
				case 1:
					break;
				default:
					generator = -1;
				}
			}
		}
	}
	while (solver == -1)
	{
		{
			std::cout << "Please select a maze solving option.\nEnter 1 for Flood Fill.\nEnter 2 for Dijkstra.\nEnter 3 for A*\n";
			std::string inputString;
			std::cin >> inputString;
			if (validateInt(inputString))
			{
				solver = std::atoi(inputString.c_str());
				switch (solver)
				{
				case 1:
					break;
				case 2:
					break;
				case 3:
					break;
				default:
					solver = -1;
				}
			}
		}
	}
	return std::make_tuple(mazeXSize, mazeYSize, goalX, goalY, generator, solver);
}

int main(void)
{
	bool paused = true; // While this is true, the car isn't moving
	auto [mazeXSize, mazeYSize, goalX, goalY, mazeGenerator, mazeSolver] = getMazeParameters();
	Maze* maze = new Maze(mazeXSize, mazeYSize, goalX, goalY);
	std::chrono::high_resolution_clock::time_point time1 = std::chrono::high_resolution_clock::now();
	std::string solverName;
	switch (mazeGenerator)
	{
	case 1:
		maze->setGenerator(new RecursiveSubdivider(maze));
		break;
	default:
		std::cout << "CRITICAL ERROR: Invalid maze generation method specified!" << std::endl;
		return 1;
	}
	std::chrono::high_resolution_clock::time_point time2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(time2 - time1);
	double mazeGenTime = timeSpan.count();
	time1 = std::chrono::high_resolution_clock::now();
	switch (mazeSolver)
	{
	case 1:
		maze->setSolver(new MazeSolve::FloodFill(maze));
		solverName = "FloodFill";
		break;
	case 2:
		maze->setSolver(new MazeSolve::Dijkstra(maze));
		solverName = "Dijkstra";
		break;
	case 3:
		maze->setSolver(new MazeSolve::AStar(maze));
		solverName = "Astar";
		break;
	default:
		std::cout << "CRITICAL ERROR: Invalid maze generation method specified!" << std::endl;
		return 1;
	}
	time2 = std::chrono::high_resolution_clock::now();
	timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(time2 - time1);
	double mazeSolveInitTime = timeSpan.count();
	GLFWWrapper* mazeWindow = new GLFWWrapper(maze->getXSize(), maze->getYSize(), 8, 8);
	std::cout << glfwGetVersionString() << std::endl;
	while (!mazeWindow->shouldClose())
	{
		for (int i = 0; i < maze->getXSize(); i++)
		{
			for (int j = 0; j < maze->getYSize(); j++)
			{
				std::string blockColour = maze->GetSpace(i, j).GetColour();
				if (blockColour == "White") glColor3f(1.0, 1.0, 1.0);
				if (blockColour == "Red") glColor3f(1.0, 0.0, 0.0);
				if (blockColour == "Green") glColor3f(0.0, 1.0, 0.0);
				if (blockColour == "Black") glColor3f(0.0, 0.0, 0.0);
				mazeWindow->drawSquare(i, j);
			}
		}
		
		mazeWindow->drawCar(maze->GetCar()->GetX(), maze->GetCar()->GetY());
		mazeWindow->drawBorder();
		mazeWindow->drawGrid();
		mazeWindow->refresh();
		if (mazeWindow->getSpacePressed())
		{
			time1 = std::chrono::high_resolution_clock::now();
			paused = !paused;
			mazeWindow->setSpacePressed(false);
		}
		if (mazeWindow->getMouseClicked()) // Debug function - allows for editing the maze in real-time
		{
			try
			{
				if (mazeWindow->getMouseButtonClicked() == GLFW_MOUSE_BUTTON_LEFT)
				{
					maze->SetSpace(mazeWindow->getMouseClickX(), mazeWindow->getMouseClickY(), new Wall());
					mazeWindow->setMouseClicked(false);
				}
				else if (mazeWindow->getMouseButtonClicked() == GLFW_MOUSE_BUTTON_RIGHT)
				{
					maze->SetSpace(mazeWindow->getMouseClickX(), mazeWindow->getMouseClickY(), new Empty());
					mazeWindow->setMouseClicked(false);
				}
			}
			catch(std::out_of_range) // Clicking around the edge of the window can throw this for some reason
			{
				
			}

		}
		if (mazeWindow->getRPressed())
		{
			delete maze;
			delete mazeWindow;
			auto [mazeXSize, mazeYSize, goalX, goalY, mazeGenerator, mazeSolver] = getMazeParameters();
			maze = new Maze(mazeXSize, mazeYSize, goalX, goalY);
			std::chrono::high_resolution_clock::time_point time1 = std::chrono::high_resolution_clock::now();
			switch (mazeGenerator)
			{
			case 1:
				maze->setGenerator(new RecursiveSubdivider(maze));
				break;
			default:
				std::cout << "CRITICAL ERROR: Invalid maze generation method specified!" << std::endl;
				return 1;
			}
			std::chrono::high_resolution_clock::time_point time2 = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(time2 - time1);
			mazeGenTime = timeSpan.count();
			time1 = std::chrono::high_resolution_clock::now();
			switch (mazeSolver)
			{
			case 1:
				maze->setSolver(new MazeSolve::FloodFill(maze));
				break;
			case 2:
				maze->setSolver(new MazeSolve::Dijkstra(maze));
				break;
			case 3:
				maze->setSolver(new MazeSolve::AStar(maze));
				break;
			default:
				std::cout << "CRITICAL ERROR: Invalid maze solving method specified!" << std::endl;
				return 1;
			}	
			time2 = std::chrono::high_resolution_clock::now();
			timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(time2 - time1);
			mazeSolveInitTime = timeSpan.count();
			mazeWindow = new GLFWWrapper(maze->getXSize(), maze->getYSize(), 8, 8);
			mazeWindow->setRPressed(false);
		}
		if (!paused)
		{
			maze->Step();
			if (maze->GetCar()->GetX() == maze->getGoalX() && maze->GetCar()->GetY() == maze->getGoalY())
			{
				time2 = std::chrono::high_resolution_clock::now();
				timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(time2 - time1);
				double mazeSolveTime = timeSpan.count();
				std::cout << "Maze generated in " << mazeGenTime << " seconds." << std::endl;
				std::cout << "Mazesolver inited in " << mazeSolveInitTime << " seconds." << std::endl;
				std::cout << "Maze solved in " << mazeSolveTime << " seconds." << std::endl;
				std::cout << "Total maze solving time: " << mazeSolveInitTime + mazeSolveTime << " seconds" << std::endl;
				mazeWindow->setSpacePressed(false);
				sndPlaySound("tada.wav", SND_ASYNC);
				paused = true;
			}
		}
		else // Constantly redrawing everything is a massive CPU hog - do it less often when there's nothing moving
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}	


	}
	glfwTerminate();
	return 0;
}

