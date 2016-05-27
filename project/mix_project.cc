#include <xcb/xcb.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <unistd.h>
#include <xcb/xproto.h>
#include <stdlib.h>
#include <iostream>

#define _USE_MATH_DEFINES
#include <cmath>

struct Point {
  GLfloat x;
  GLfloat y;
} cursor;

GLfloat get_length(Point a, Point b);
Point segment_division(Point a, Point b, double lambda);
static void error_callback(int error, const char* description);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void drawLine(Point P1, Point P2);
void drawCircle(Point P, GLfloat radius);
double radian_to_degree(double angle);


int main(void) {
  xcb_connection_t    *c;
  xcb_screen_t        *screen;
  xcb_drawable_t       win;
  xcb_gcontext_t       foreground;
  xcb_generic_event_t *e;
  uint32_t             mask = 0;
  uint32_t             values[2];
  xcb_query_pointer_cookie_t cookie;
  xcb_query_pointer_reply_t* rep;
  GLfloat lambda; 
  GLfloat x;
  GLfloat y;
  double angle;

  Point tmp;
  tmp.x = 0.9f;
  tmp.y = 0.0f;

  Point our_focus;
  our_focus.x = 139.0f;
  our_focus.y = 100.0f;

  Point focus_of_window;
  focus_of_window.x = 0.0f;
  focus_of_window.y = 0.0f;

  Point end_in_window;
  
  c = xcb_connect (NULL, NULL);
  GLFWwindow* window;
  glfwSetErrorCallback(error_callback);
  
  if (!glfwInit())
    exit(EXIT_FAILURE);
  
  glfwWindowHint(GLFW_DECORATED, GL_FALSE);
  window = glfwCreateWindow(150, 150, "Simple example", NULL, NULL);
  

  glfwSetWindowPos(window, 64, 25);

  if (!window){
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
    
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  glfwSetKeyCallback(window, key_callback);
  while (!glfwWindowShouldClose(window)){
    screen = xcb_setup_roots_iterator (xcb_get_setup (c)).data;
    win = screen->root;
    cookie = xcb_query_pointer(c, win);
    rep = xcb_query_pointer_reply(c,cookie, 0 );
    cursor.x = rep->root_x;
    cursor.y = rep->root_y;
    //lambda = length/(get_length(our_focus, cursor) - length);
    
    //tmp = segment_division(our_focus,cursor, lambda);
    if((cursor.x >= 139.0f) && (cursor.y <= 100.0f)){// the first quater
      x = cursor.x - our_focus.x;
      y = our_focus.y - cursor.y;
      angle = atan(y/x);  

    }
    if((cursor.x < 139.0f) && (cursor.y > 100.0f)){ // the third quater
      x = cursor.x - our_focus.x;
      y = our_focus.y - cursor.y;
      angle = M_PI + atan(y/x);
    }
    if((cursor.x >= 139.0f) && (cursor.y > 100.0f)){ // the fourth quater
      x = cursor.x - our_focus.x;
      y = cursor.y - our_focus.y;
      angle = 2*M_PI - atan(y/x);
    }
    if((cursor.x < 139.0f) && (cursor.y <= 100.0f)){// the second quater
      x = our_focus.x - cursor.x;
      y = our_focus.y - cursor.y;
      angle = M_PI - atan(y/x);
    }

    angle = radian_to_degree(angle);
    float ratio;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    ratio = width/ (float) height;
    glViewport(0, 0, width, height);
    glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_TRUE);
   // glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(  angle, 0.f, 0.f, 1.f);
        
    drawLine(focus_of_window, tmp); 
    //drawCircle(end_in_window,0.1f);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

GLfloat get_length(Point a, Point b) {
  return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}  

Point segment_division(Point a, Point b, double lambda) {
  Point res;
  printf("%f\n", lambda);
  res.x = (a.x + lambda * b.x) / (1 + lambda);
  res.y = (a.y + lambda * b.y) / (1 + lambda);
  return res;
}

static void error_callback(int error, const char* description) {
  fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

void drawLine(Point P1, Point P2 ){
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glColor4f(1.0, 0.0, 0.0, 0.0);
    glVertex2f(P1.x, P1.y);
    glVertex2f(P2.x, P2.y);
    glEnd(); 
}

void drawCircle(Point P, GLfloat radius){
  int i;
  int triangleAmount = 1000;
  GLfloat twicePi = 2.0f * M_PI;
  glEnable(GL_LINE_SMOOTH);
  glLineWidth(5.0);
  glBegin(GL_LINES);
  glColor4f(1.0, 0.0, 0.0, 0.0);
  for(i = 0; i <= triangleAmount; i++) {
    glVertex2f(P.x, P.y);
    glVertex2f(P.x + (radius * cos(i * twicePi / triangleAmount)), P.y + (radius * sin(i * twicePi / triangleAmount)));
  }
  glEnd();
}

double radian_to_degree(double angle) {
  return angle * 360 / (2 * M_PI);
}