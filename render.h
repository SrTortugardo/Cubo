#ifndef RENDER_H
#define RENDER_H

typedef struct {
    float x, y, z;
} Vec3; // Nuestro vector solo tiene X,Y,Z, en otras palabras es un punto en el espacio.

typedef struct {
    Vec3 position;
    float nearPlane;
    float fov;

    float yaw;   // izquierda/derecha
    float pitch; // arriba/abajo
} Camera;

void getTerminalSize(int *width, int *height);
void disableRaw();
void enableRaw();

void clearBuffer(char *buffer, int width, int height);
void drawBuffer(char *buffer, int width, int height);

void rotateX(Vec3 *v, float angle);
void rotateY(Vec3 *v, float angle);

void project(Vec3 v, int *screenX, int *screenY, Camera cam, int width, int height);

void putPixel(char *buffer, int width, int height, int x, int y, char c);

void drawLine(char *buffer, int width, int height, int x0, int y0, int x1, int y1);

void drawLine3D(char *buffer, int width, int height, Vec3 a, Vec3 b, Camera cam);

void present(char *buffer, int width, int height);

#endif
// Estas funciones se explican a detalle en el archivo render.c
