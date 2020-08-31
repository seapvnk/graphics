#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 512
#define WINDOW_TITLE "bruh"

#define PI 3.141592
#define P2 (PI / 2)
#define P3 (3 * PI / 2)
#define DR 0.01745329


#define KEY_LEFT 'a'
#define KEY_RIGHT 'd'
#define KEY_TOP 'w'
#define KEY_BOTTOM 's'


typedef struct {
	float x;
	float y;
	float dx;
	float dy;
	float angle;
} Player;
Player player;

typedef struct {
	int sizeX;
	int sizeY;
	int blockSize;
	char *tiles;
} Map;
Map map;

void initMap(Map *map, int sizeX, int sizeY, int blockSize, char *mapPath);
void drawMap2D(Map *map);
void drawRays2D();
float distance(float ax, float ay, float bx, float by, float a);
void drawPlayer();
void display();
void buttons(unsigned char key, int x, int y);
void init();
char *loadMap(char *filename);


int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutCreateWindow(WINDOW_TITLE);
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(buttons);
	glutMainLoop();
}

void drawMap2D(Map *map)
{
	int x, y, xi, yi;
	char currentTile;
	for (y = 0; y < map->sizeY; y++)
	{
		for (x = 0; x < map->sizeX; x++)
		{
			currentTile = map->tiles[y * map->sizeX + x];
			
			if (currentTile == '1')
			{
				glColor3f(1, 1, 1);
			}
			else
			{
				glColor3f(0, 0, 0);
			}
			
			xi = x * map->blockSize;
			yi = y * map->blockSize;
			
			// draw a square
			glBegin(GL_QUADS);
			glVertex2i(xi +1, yi +1);
			glVertex2i(xi +1, yi + map->blockSize -1);
			glVertex2i(xi + map->blockSize -1, yi + map->blockSize -1);
			glVertex2i(xi + map->blockSize -1, yi + 1);
			glEnd();
		}
	}
}

void initMap(Map *map, int sizeX, int sizeY, int blockSize, char *mapPath)
{
	map->sizeX = sizeX;
	map->sizeY = sizeY;
	map->blockSize = blockSize;
	map->tiles = loadMap(mapPath);
}

char *loadMap(char *filename)
{

	FILE *source = fopen(filename, "r");
	char c, *string = malloc(1);
	int len = 1;

	// read file
	while ((c = fgetc(source)) != EOF) {
		if (c == ' ' || c == '\n')
		{
			continue;
		}
		string[len-1] = c;
		string = realloc(string, ++len);
	}

	// end of string
	string = realloc(string, len+2);
	string[len] = 0;
	fclose(source);

	return string;
}

void drawPlayer()
{
	glColor3f(1,1,0);
	glPointSize(8);
	glBegin(GL_POINTS);
	glVertex2i(player.x, player.y);
	glEnd();
	
	glBegin(GL_LINES);
	glVertex2i(player.x, player.y);
	glVertex2i(player.x + player.dx * 5, player.y + player.dy * 5);
	glEnd();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawMap2D(&map);
	drawPlayer();
	drawRays2D();
	glutSwapBuffers();
}

void drawRays2D()
{
	int r, mx, my, mp, dof, finalDistance;
	float rx, ry, ra, xi, yi;
	
	ra = player.angle - DR * 30;
	if (ra < 0)
	{
		ra += 2 * PI;
	}
	if (ra > 2 *  PI)
	{
		ra -= 2 * PI;
	}
	
	for (r = 0; r < 60; r++)
	{
		// Horizontal lines
		dof = 0;
		float disH = 1000000, hx = player.x, hy = player.y;
		float aTan = -1 / tan(ra);
		
		if (ra > PI)
		{
			ry = (((int) player.y >> 6) << 6) - 0.0001;
			rx = (player.y - ry) * aTan + player.x;
			yi = -64;
			xi = -yi * aTan;
		}
		
		if (ra < PI)
		{
			ry = (((int) player.y >> 6) << 6) + 64;
			rx = (player.y - ry) * aTan + player.x;
			yi = 64;
			xi = -yi * aTan;
		}
		
		if (ra == 0 || ra == PI)
		{
			rx = player.x;
			ry = player.y;
			dof = 8;
		}
		
		while (dof < 8)
		{
			mx = (int) (rx) >> 6;
			my = (int) (ry) >> 6;
			mp = my * map.sizeX + mx;
			
			// ray hit the wall
			if (mp > 0 && mp < map.sizeX * map.sizeY && map.tiles[mp] == '1')
			{
				hx = rx;
				hy = ry;
				disH = distance(player.x, player.y, hx, hy, player.angle);
				dof = 8;	
			}
			else
			{
				rx += xi;
				ry += yi;
				dof += 1;
			}
		}
		
		// Vertical lines
		dof = 0;
		float disV = 1000000, vx = player.x, vy = player.y;
		float nTan = -tan(ra);
		
		if (ra > P2 && ra < P3)
		{
			rx = (((int) player.x >> 6) << 6) - 0.0001;
			ry = (player.x - rx) * nTan + player.y;
			xi = -64;
			yi = -xi * nTan;
		}
		
		if (ra < P2 || ra > P3)
		{
			rx = (((int) player.x >> 6) << 6) + 64;
			ry = (player.x - rx) * nTan + player.y;
			xi = 64;
			yi = -xi * nTan;
		}
		
		if (ra == 0 || ra == PI)
		{
			rx = player.x;
			ry = player.y;
			dof = 8;
		}
		
		while (dof < 8)
		{
			mx = (int) (rx) >> 6;
			my = (int) (ry) >> 6;
			mp = my * map.sizeX + mx;
			
			// ray hit the wall
			if (mp > 0 && mp < map.sizeX * map.sizeY && map.tiles[mp] == '1')
			{
				vx = rx;
				vy = ry;
				disV = distance(player.x, player.y, vx, vy, player.angle);
				dof = 8;	
			}
			else
			{
				rx += xi;
				ry += yi;
				dof += 1;
			}
			
			
		}
		
		if (disV < disH)
		{
			glColor3f(1, 0, 0);
			finalDistance = disV;
			rx = vx;
			ry = vy;
		}
		
		if (disH < disV)
		{
			glColor3f(0.8, 0, 0);
			finalDistance = disH;
			rx = hx;
			ry = hy;
		}
		
		if (ra < 0)
		{
			ra += 2 * PI;
		}
		if (ra > 2 *  PI)
		{
			ra -= 2 * PI;
		}
		
		ra += DR;
		
		
		glBegin(GL_LINES);
		glVertex2i(player.x, player.y);
		glVertex2i(rx, ry);
		glEnd();
		
		// Draw the walls
		float ca = player.angle - ra;
		if (ca < 0)
		{
			ca += 2*PI;
		}
		if (ca > 2*PI)
		{
			ca -= 2*PI;
		}
		finalDistance = finalDistance * cos(ca);
		float lineHeight = (map.blockSize * 320) / finalDistance;
		float lineOffset = 160 - lineHeight / 2;
		
		if (lineHeight > 320)
		{
			lineHeight = 320;
		}
		
		glLineWidth(8);
		glBegin(GL_LINES);
		glVertex2i(r*7 + 520, lineOffset);
		glVertex2i(r*7 + 520, lineHeight + lineOffset);
		glEnd();
		
	}
	
	
}

float distance(float ax, float ay, float bx, float by, float a)
{
	return (sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay)));
}

void buttons(unsigned char key, int x, int y)
{
	switch (key)
	{
		case KEY_LEFT:
			player.angle -= 0.1;
			if (player.angle < 0)
			{
				player.angle += 2*PI;
			}
			player.dx = cos(player.angle) * 5;
			player.dy = sin(player.angle) * 5;	
			break;
		case KEY_RIGHT:
			player.angle += 0.1;
			if (player.angle > 2*PI)
			{
				player.angle -= 2*PI;
			}
			player.dx = cos(player.angle) * 5;
			player.dy = sin(player.angle) * 5;
			break;
		case KEY_TOP:
			player.y += player.dy;
			player.x += player.dx;
			break;
		case KEY_BOTTOM:
			player.y -= player.dy;
			player.x -= player.dx;
			break;
	}
	
	glutPostRedisplay();
}

void init()
{
	glClearColor(0.3, 0.3, 0.3, 0);
	gluOrtho2D(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	player.x = 300;
	player.y = 300;
	player.dx = cos(player.angle);
	player.dy = sin(player.angle);
	initMap(&map, 8, 8, 64, "map0.txt");
}

