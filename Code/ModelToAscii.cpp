#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>

void DrawPoint(HANDLE, SHORT, SHORT, char);
float* MatrixMultiply(float[4][4], float[4]);

void DrawPoint(HANDLE handle, SHORT x, SHORT y, char c)
{
	SetConsoleCursorPosition(handle, { x * 2, y });
	std::cout << c;
}

float* MatrixMultiply(float matrix[4][4], float point[4])
{
	float result[4];
	for (int i = 0; i < 4; i++) 
	{
		float n = 0;
		for (int j = 0; j < 4; j++) 
		{
			n += matrix[i][j] * point[j];
		}
		result[i] = n;
	}
	return result;
}

int main()
{
	std::cout << "Renders a .obj file with perspective.\nWASD - Translate\nArrow Keys - Rotate\nE/F - Scale\n\n";
	
	std::string name;
	std::cout << "Enter File Name: ";
	std::cin >> name;

	std::vector<std::vector<float>> modelPoints;

	std::ifstream file(name);
	while (!file.eof())
	{
		std::string line;
		std::getline(file, line);
		if (line[0] == 'v' && line[1] == ' ')
		{
			std::string floats[3] = { "","","" };
			std::string str = "";
			int n = 0;
			for (int i = 2; i < line.length(); i++) 
			{
				if (line[i] == ' ' || i == line.length()-1) 
				{
					floats[n] = str;
					str = "";
					n++;
				}
				str += line[i];
			}

			std::vector<float> point = 
			{ 
				(float)atof(floats[0].c_str()), 
				(float)atof(floats[1].c_str()), 
				(float)atof(floats[2].c_str()), 
				1 
			};

			modelPoints.push_back(point);
		}
	}

	HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	//SetConsoleTextAttribute(stdHandle, FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_BLUE); // https://learn.microsoft.com/en-us/windows/console/console-screen-buffers?redirectedfrom=MSDN#_win32_character_attributes

	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(stdHandle, &info);
	SMALL_RECT size = info.srWindow;
	int width = size.Right / 2;
	int height = size.Bottom;

	float xPos = 30;
	float yPos = 15;
	float zPos = 20;

	int scale = 75;

	float xAngle = 0;
	float yAngle = 0;

	while(!GetAsyncKeyState(0x2D)) // ins
	{
		bool input = false;
		
		if (GetAsyncKeyState(0x57)) { yPos -= 10; input = true; } // W
		if (GetAsyncKeyState(0x41)) { xPos-=10; input = true; } // A
		if (GetAsyncKeyState(0x53)) { yPos+=10; input = true; } // S
		if (GetAsyncKeyState(0x44)) { xPos+=10; input = true; } // D

		if (GetAsyncKeyState(0x45)) { scale++; input = true; } // E
		if (scale > 1 && GetAsyncKeyState(0x46)) { scale--; input = true; } // F

		if (GetAsyncKeyState(0x26)) { xAngle-=4; input = true; } // UP ARROW
		if (GetAsyncKeyState(0x25)) { yAngle+=4; input = true; } // LEFT ARROW
		if (GetAsyncKeyState(0x28)) { xAngle+=4; input = true; } // DOWN ARROW
		if (GetAsyncKeyState(0x27)) { yAngle-=4; input = true; } // RIGHT ARROW

		if (!input) { continue; }

		Sleep(30);
		system("cls");

		if (xAngle >= 360) { xAngle = 0; }
		if (yAngle >= 360) { yAngle = 0; }
		if (xAngle < 0) { xAngle = 359; }
		if (yAngle < 0) { yAngle = 359; }
		float xRadians = xAngle * 0.01745329;
		float yRadians = yAngle * 0.01745329;

		float rotationX[4][4] =
		{
			{ 1, 0, 0, 0 },
			{ 0, cos(xRadians), -sin(xRadians), 0 },
			{ 0, sin(xRadians), cos(xRadians), 0 },
			{ 0, 0, 0, 1 }
		};

		float rotationY[4][4] =
		{
			{ cos(yRadians), 0, sin(yRadians), 0 },
			{ 0, 1, 0, 0 },
			{ -sin(yRadians), 0, cos(yRadians), 0 },
			{ 0, 0, 0, 1 }
		};

		/*
		float rotationZ[4][4] =
		{
			{ cos(radians), -sin(radians), 0, 0 },
			{ sin(radians), cos(radians), 0, 0 },
			{ 0, 0, 1, 0 },
			{ 0, 0, 0, 1 }
		};
		*/

		float translationAndScale[4][4] =
		{
			{ scale, 0, 0, xPos },
			{ 0, scale, 0, yPos },
			{ 0, 0, 1, zPos },
			{ 0, 0, 0, 1 }
		};

		float view[4][4] =
		{
			{ 1, 0, 0, 30 },
			{ 0, 1, 0, 15 },
			{ 0, 0, 1, 0 },
			{ 0, 0, 0, 1 }
		};

		float minZ = 999;
		float maxZ = 0;
		std::vector<std::vector<float>> screenPoints;
		for (int i = 0; i < modelPoints.size(); i++)
		{
			float* temp;

			// Rotate Model's X
			temp = MatrixMultiply(rotationX, (float*)&modelPoints[i][0]);
			float rotatedX[4] = { temp[0], temp[1], temp[2], 1 };
			// Rotate Model's Y
			temp = MatrixMultiply(rotationY, rotatedX);
			float rotatedXY[4] = { temp[0], temp[1], temp[2], 1 };

			// Translate and Scale the Model
			temp = MatrixMultiply(translationAndScale, rotatedXY);
			float model[4] = { temp[0], temp[1], temp[2], 1 };

			// Perspective Divide
			if (model[2] != 0)
			{
				model[0] /= model[2];
				model[1] /= model[2];
			}

			// Translate "Camera"; It should be looking straight at the model to start
			temp = MatrixMultiply(view, model);
			float viewed[4] = { temp[0], temp[1], temp[2], 1 };

			// Clipping
			if (viewed[0] >= 0 && viewed[0] <= width && viewed[1] >= 0 && viewed[1] <= height)
			{
				screenPoints.push_back({viewed[0], viewed[1], viewed[2]});
				if (viewed[2] < minZ) { minZ = viewed[2]; }
				else if (viewed[2] > maxZ) { maxZ = viewed[2]; }
			}
		}

		for (int i = 0; i < screenPoints.size(); i++) 
		{
			char c = '@';
			float mapped = (maxZ-screenPoints[i][2]) / (maxZ-minZ);
			if (mapped < 0.1) { c = '-'; }
			else if (mapped < 0.2) { c = '~'; }
			else if (mapped < 0.3) { c = '^'; }
			else if (mapped < 0.4) { c = '+'; }
			else if (mapped < 0.5) { c = '*'; }
			else if (mapped < 0.6) { c = '#'; }
			else if (mapped < 0.7) { c = '$'; }
			else if (mapped < 0.8) { c = '%'; }
			else if (mapped < 0.9) { c = '&'; }
			DrawPoint(stdHandle, screenPoints[i][0], screenPoints[i][1], c);
		}
	}
}