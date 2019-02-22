#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#ifdef __APPLE__  // include Mac OS X verions of headers
#include <GLUT/glut.h>
#else // non-Mac OS X operating systems
#include <GL/glut.h>
#endif

#define XOFF          50
#define YOFF          50
#define WINDOW_WIDTH  600
#define WINDOW_HEIGHT 600

void display(void);
void myinit(void);
void draw_circle(int x, int y, int r);
void circlePoint(int x, int y);
int floorSqrt(int dist);
void idle(void);

int x;
int y;
int r;
double scale = 2.52;
//double scalecounter = 1.0;
int numberOfLoops;

//k = #of max frames and framenum = current frame
double framenum = 1.0;
double k = 5000;


/* Function to handle file input; modification may be needed */
void file_in(void);


struct myCoords {
	vector<int> xcoords;
	vector<int> ycoords;
	vector<int> rcoords;

};
myCoords worldCordinates;

/*-----------------
The main function
------------------*/
int main(int argc, char **argv)
{
	/*

	MOVED TO FILEIN()

	std::cout << "Enter the radius: ";
	std::cin >> r;
	std::cout << std::endl;

	std::cout << "Enter origin x coordinate: ";
	std::cin >> x;
	std::cout << std::endl;

	std::cout << "Enter origin y coordinate: ";
	std::cin >> y;
	std::cout << std::endl;
	*/ 
	glutInit(&argc, argv);

	/* Use both double buffering and Z buffer */
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowPosition(XOFF, YOFF);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("CS6533/CS4533 Assignment 1");

	glutDisplayFunc(display);
	
	/* Function call to handle file input here */
	file_in();
	glutIdleFunc(idle);
	myinit();
	glutMainLoop();

	return 0;
}

/*----------
file_in(): file input function. Modify here.
------------*/
void file_in(void)
{
	ifstream myfile;
	myfile.open("Example.txt");
	if (!myfile) {
		cerr << "Unable to open file datafile.txt";
		exit(1);   // call system to stop
	}

	myfile >> numberOfLoops;

	int xtemp;
	int ytemp;
	int rtemp;

	while (myfile >> xtemp >> ytemp >> rtemp) {
		worldCordinates.xcoords.push_back(xtemp);
		worldCordinates.ycoords.push_back(ytemp);
		worldCordinates.rcoords.push_back(rtemp);
		
	}


	myfile.close();
}


/*---------------------------------------------------------------------
display(): This function is called once for _every_ frame.
---------------------------------------------------------------------*/
void display(void)
{
		
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glColor3f(1.0, 0.84, 0);              /* draw in golden yellow */
		glPointSize(1.0);                     /* size of each point */

		glBegin(GL_POINTS);

		//glVertex2i(300, 300);               /* draw a vertex here */
		for (int i = 0; i < numberOfLoops; ++i) {
			double finscale =  framenum / k;
			//cout << finscale << " ";
			x = (worldCordinates.xcoords[i] / scale) + WINDOW_WIDTH / 2;
			y = (worldCordinates.ycoords[i] / scale) + WINDOW_HEIGHT / 2;
			r = (worldCordinates.rcoords[i] / scale) * finscale;
			//cout << x << " " << y << " " << r << endl;
			draw_circle(x, y, r);				  /* draw our circle */
		}

		//circlePoint(x, y);				  /* Test for Circle Point */	
		glEnd();

		glFlush();                            /* render graphics */

		glutSwapBuffers();                    /* swap buffers */
	
}

/*---------------------------------------------------------------------
myinit(): Set up attributes and viewing
---------------------------------------------------------------------*/
void myinit()
{
	glClearColor(0.0, 0.0, 0.92, 0.0);    /* blue background*/

	/* set up viewing */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT);
	glMatrixMode(GL_MODELVIEW);
}

/*---------------------------------------------------------------------
idle(): How Animation Works
---------------------------------------------------------------------*/
void idle(void) {
	//scalecounter = scalecounter  * 1.022;
	if (framenum > k) {
		//scalecounter = 1.0;
		framenum = 1.0;
	}
	framenum = framenum + 1.0;
	glutPostRedisplay();
}

/*---------------------------------------------------------------------
circlePoint(): Draws our circle points, 8-fold symmetric locations!
---------------------------------------------------------------------*/

void circlePoint(int xtemp, int ytemp) {
	int centerX = x;//WINDOW_WIDTH/2;
	int centerY = y;//WINDOW_HEIGHT/2; 

	//Sample x = 50
	//Sample y = 45


	glVertex2i(centerX + xtemp, centerY + ytemp);
	glVertex2i(centerX - xtemp, centerY - ytemp);
	glVertex2i(centerX - xtemp, centerY + ytemp);
	glVertex2i(centerX + xtemp, centerY - ytemp);
	glVertex2i(centerX + ytemp, centerY + xtemp);
	glVertex2i(centerX - ytemp, centerY - xtemp);
	glVertex2i(centerX - ytemp, centerY + xtemp);
	glVertex2i(centerX + ytemp, centerY - xtemp);

	
}


/*---------------------------------------------------------------------
draw_circle(): Draws our circle
---------------------------------------------------------------------*/
void draw_circle(int x, int y, int r) {
	int xtemp = 0;
	int ytemp = -r;

	int dcurr = 1 - r;
	circlePoint(xtemp, ytemp);

	while (-ytemp > xtemp) {
		if (dcurr <= 0) {
			dcurr += 2 * xtemp + 3;
			xtemp++;
		}
		else {
			dcurr += 2 * (xtemp + ytemp) + 5;
			xtemp++;
			ytemp++;
		}
		circlePoint(xtemp, ytemp);
	}
}

