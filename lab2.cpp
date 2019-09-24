
// modified by: Kevin Mitra
// date: September 14, 2019
//
// 3350 Fall 2019 HW 2
// lab1.cpp
// This program demonstrates the use of OpenGL and XWindows
//
// Assignment is to modify this program.
// You will follow along with your instructor.
//
// Elements to learned in this lab...
//  .general animation framework
//  .animation loop
//  .object definition and movement
//  .collision detection
//  .mouse/keyboard interaction
//  .object constructor
//  .coding style
//  .defined constraints
//  .use of static varibles
//  .dynamic memory allocation
//  .simple opengl components
//  .git
//
//elements we will add to programm...
//   .Game constructor
//   .multiple particles
//   .gravity
//   .collision detection
//   .more objects
//
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <GL/glut.h>
#include "fonts.h"


using namespace std;

const int MAX_PARTICLES = 6500;
const float GRAVITY = 0.1;

// some structures


struct Vec {
    float x, y, z;
};

struct Shape {
    float width = 80, height = 10;
    float radius;
    Vec center;
};

struct Particle {
    Shape s;
    Vec velocity;
};

class Global {
    public:
        int xres, yres;
        Shape box[5];
        Particle particle[MAX_PARTICLES];
        int n;
        Global();
} g;

class X11_wrapper {
    private:
        Display *dpy;
        Window win;
        GLXContext glc;
    public:
        ~X11_wrapper();
        X11_wrapper();
        void set_title();
        bool getXPending();
        XEvent getXNextEvent();
        void swapBuffers();
} x11;

//Function prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void movement();
void render();

// MAIN FUNCTION //
int main()
{
    srand(time(NULL));
    init_opengl();
    //Main animation loop
    int done = 0;
    while (!done) {
        //Process external events.
        while (x11.getXPending()) {
            XEvent e = x11.getXNextEvent();
            check_mouse(&e);
            done = check_keys(&e);
        }
        movement();
        render();
        x11.swapBuffers();
    }
    return 0;
}

//Global class functions

Global::Global()
{
    xres = 800;
    yres = 600;
    //define a box shape
    //box.width = 100;
    //box.height = 10;
    //box.center.x = 120 + 5*65;
    //box.center.y = 500 - 5*60;
    n = 0;
}

//X11_wrapper class functions

X11_wrapper::~X11_wrapper()
{
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

X11_wrapper::X11_wrapper()
{
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w = g.xres, h = g.yres;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        cout << "\n\tcannot connect to X server\n" << endl;
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if (vi == NULL) {
        cout << "\n\tno appropriate visual found\n" << endl;
        exit(EXIT_FAILURE);
    }
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask =
        ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPress | ButtonReleaseMask |
        PointerMotionMask |
        StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
            InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void X11_wrapper::set_title()
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "3350 Lab1");
}

bool X11_wrapper::getXPending()
{
    //See if there are pending events.
    return XPending(dpy);
}

XEvent X11_wrapper::getXNextEvent()
{
    //Get a pending event.
    XEvent e;
    XNextEvent(dpy, &e);
    return e;
}

void X11_wrapper::swapBuffers()
{
    glXSwapBuffers(dpy, win);
}
//-----------------------------------------


void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, g.xres, g.yres);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
    //Set the screen background color
    glEnable(GL_TEXTURE_2D);
    glClearColor(0.1, 0.1, 0.1,1.0);
    initialize_fonts();
}

void makeParticle(int x, int y)
{
    //Add a particle to the particle system.
    //
    if (g.n >= MAX_PARTICLES)
        return;
    cout << "makeParticle() " << x << "" << y << endl;
    //set postion of particle
    Particle *p = &g.particle[g.n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y = -0.3;
    p->velocity.x = ((double)rand() / (double)RAND_MAX) + 0.5;
    p->velocity.x = ((double)rand() / (double)RAND_MAX) + 0.5 + 0.10;
    ++g.n;
}

void check_mouse(XEvent *e)
{
    static int savex = 0;
    static int savey = 0;

    //Weed out non-mouse events
    if (e->type != ButtonRelease &&
            e->type != ButtonPress && 
            e->type != MotionNotify) {
        return;
    }

    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            //left button was pressed.
            int y = g.yres - e->xbutton.y;
            makeParticle(e->xbutton.x, y);
            return;
        }
        if (e->xbutton.button==3) {
            //Right button was pressed.
            return;
        }
    }
    if (e->type == MotionNotify) {
        //The Mouse moved!
        if (savex != e->xbutton.x || savey != e->xbutton.y) {
            savex = e->xbutton.x;
            savey = e->xbutton.y;
            //Code place here will execue whenever the mouse moves.


            int y = g.yres - e->xbutton.y;
            for (int i=0; i<10; i++)
                makeParticle(e->xbutton.x, y);
        }
    }
}
int check_keys(XEvent *e) 
{
    if (e->type != KeyPress && e->type != KeyRelease)
        return 0;
    int key = XLookupKeysym(&e->xkey, 0);
    if (e->type == KeyPress) {
        switch (key) {
            case XK_1:
                //key 1 was pressed
                break;
            case XK_a:
                //key A was pressed
                break;
            case XK_Escape:
                //Escape key was pressed
                return 1;
        }
    }
    return 0;
}

void movement() 
{
    if (g.n <= 0)
        return;
    for (int i=0; i<g.n; i++) {
        Particle *p =&g.particle[i];
        p->s.center.x += p->velocity.x;
        p->s.center.y += p->velocity.y;
        p->velocity.y -= GRAVITY;

        //check for collision with shapes...
        //Shape *s = &g.box;
        for (int j=0; j<6; j++) {
            if (p->s.center.y < g.box[j].center.y + g.box[j].height &&
                p->s.center.x > g.box[j].center.x - g.box[j].width &&
                p->s.center.x < g.box[j].center.x + g.box[j].width &&
               p->s.center.y > g.box[j].center.y - g.box[j].height) 
            p->velocity.y = 0.2;

        }

        //check for off-screen
        if (p->s.center.y < 0.0) {
            g.particle[i] = g.particle[g.n-1];
            --g.n;
        }
    }
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    //Draw Shapes...
    //draw the box
    glColor3ub(90,140,90);
    float w[5],h[5];
    g.box[0].center.x = 100;
    g.box[0].center.y = 500;
    for (int i=1; i<6; i++) {
        g.box[i].center.x = g.box[i-1].center.x + 90;
        g.box[i].center.y = g.box[i-1].center.y - 80;
    }
    for (int i=1; i<6; i++) {
        glPushMatrix();
        glTranslatef(g.box[i].center.x, g.box[i].center.y, g.box[i].center.z); 
        w[i] = g.box[i].width;
        h[i] = g.box[i].height;
        glBegin(GL_QUADS);
        glVertex2i(-w[i], -h[i]);
        glVertex2i(-w[i],  h[i]);
        glVertex2i( w[i],  h[i]);
        glVertex2i( w[i], -h[i]);
        glEnd();
        glPopMatrix();
    }

    //
    //Draw particles here
    for (int i=0; i<g.n; i++) {
        // only one particle to draw
        glPushMatrix();
        glColor3ub(150,160,220);
        Vec *c = &g.particle[i].s.center;
        w[0] = h[0] = 2;
        glBegin(GL_QUADS);
        glVertex2i(c->x-w[0], c->y-h[0]);
        glVertex2i(c->x-w[0], c->y+h[0]);
        glVertex2i(c->x+w[0], c->y+h[0]);
        glVertex2i(c->x+w[0], c->y-h[0]);
        glEnd();
        glPopMatrix();
    }

    //
    //Draw your 2D text here
    Rect r;
    r.bot = 450;
    r.left = 45;
    r.center = 0;
    ggprint8b(&r, 16, 0x00ff0000, "Requirements");
 
    r.bot = 410;
    r.left = 160;
    r.center = 0;
    ggprint8b(&r, 16, 0x00ff0000, "Design"); 
 
    r.bot = 330;
    r.left = 240;
    r.center = 0;
    ggprint8b(&r, 16, 0x00ff0000, "Implementation"); 
 
    r.bot = 255;
    r.left = 350;
    r.center = 0;
    ggprint8b(&r, 16, 0x00ff0000, "Testing"); 
 
    r.bot = 170;
    r.left = 420;
    r.center = 0;
    ggprint8b(&r, 16, 0x00ff0000, "Maintenance"); 

}

