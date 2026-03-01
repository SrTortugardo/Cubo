/*
HASTE UN FAVOR Y NO VEAS ESTE CODIGO, ¡AHORRA 10 MINUTOS DE TU VIDA! :
ESTA **HORRIBLEMENTE HECHO** Y HORIBLEMENTE EXPLICADO(literal los comentarios solo son para recordar cuando reanude el proyecto en 6 meses)

*/


#include <stdio.h>      // printf, putchar
#include <math.h>       // sinf, cosf
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>    // Asi tenemos input si esperar a enter
#include <fcntl.h>      // Para las operaciones sobre descriptores
#include "render.h"     // Las peruanas funciones


struct termios original; // La terminal antes del modo RAW, esto para cuando salgamos de la terminal, restablecerla a como estaba antes

void
enableRaw()
{
    tcgetattr(STDIN_FILENO, &original); // configuracion actual
    struct termios raw = original;
    raw.c_lflag &= ~(ICANON | ECHO); // desactivar el modo canonico y el ECHO(que se muestre lo que escribes)
    tcsetattr(STDIN_FILENO, TCSANOW, &raw); // ahora activamos estos cambios ggez
}

void
disableRaw()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original); // volvemos a como estaba antes del raw
}

void 
getTerminalSize(int *width, int *height) // terminal dime cuantas filas y columnas tienes
{
    struct winsize w;
    // Si el peruano de ioctl devuelve -1, es recomendable comprobar, porque puede significar que hubo un error
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) { // le pido al kernel porfavor y gracias el tamaño de la terminal
        // si falla, pues no arriesgar, solo ponemos valores basicos, defaults pues
        *width = 80;
        *height = 24;
        return;
    }
    *width = w.ws_col;
    *height = w.ws_row;
}

void
clearBuffer(char *buffer, int width, int height) // Querido buffer simplemente pinta el buffer con espacios en blanco(' ') btw buffer es una matriz de una dimension representada como de 2 con index = y*width +x
{
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            buffer[y * width + x] = ' ';
}

void
drawBuffer(char *buffer, int width, int height) // Solo muestra el contenido del buffer en la pantalla :P
{
    // \033[H = cursor, \033[J es limpiar desde cursor(inicio) hasta final de pantalla, Poner el cursor al principio evita que la terminal se ponga a hacer cosas raras
    printf("\033[H\033[J");

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++)
            putchar(buffer[y * width + x]); // imprime un carácter
        putchar('\n');
    }
}


void
rotateX(Vec3 *v, float angle) // Rotar un punto alrededor del eje X
{
    // Fórmula de rotación 3D (eje X). ni yo entiendo bien esto, no cambies valores
    float y = v->y * cosf(angle) - v->z * sinf(angle);
    float z = v->y * sinf(angle) + v->z * cosf(angle);
    v->y = y;
    v->z = z;
}


void
rotateY(Vec3 *v, float angle) // Lo mismo pero en Y
{
    float x = v->x * cosf(angle) + v->z * sinf(angle);
    float z = -v->x * sinf(angle) + v->z * cosf(angle); // Arreglado, lo calcule como y al principio :P
    v->x = x;
    v->z = z;
}


void
project(Vec3 v, int *screenX, int *screenY, Camera cam, int width, int height)
{
    float fovRad = 1.0f / tanf(cam.fov * 0.5f * M_PI / 180.0f); // Fov son los grados, se convierte en focal factor con cot(fov/2)

    if (v.z <= 0.01f) // evitamos divisiones entre zero como buen subnormal aunque nos quita presicion
        v.z = 0.01f;
    
    // Proyeccion basica, mi cabeza no da para poner algo mas complejo que esto, asi que es lo que hay
    float x = v.x * fovRad / v.z;
    float y = v.y * fovRad / v.z;
    
    // Akchuali debemos mapear [-1,1] a [0,width-1] y [0,height-1] | en pocas palabras convertir cordenadas matematicas a las de la pantalla(pixeles pues)
    *screenX = (int)((x + 1.0f) * width * 0.5f);
    *screenY = (int)((y + 1.0f) * height * 0.5f);
}


void
putPixel(char *buffer, int width, int height, int x, int y, char c) // poner un caracter en el buffer, dentro del area
{
    if (x >= 0 && x < width && y >= 0 && y < height)
        buffer[y * width + x] = c;
    // Si esta fuera de "la pantalla" simplemente no existe porque es faik
}


void
drawLine(char *buffer, int width, int height, int x0, int y0, int x1, int y1) // algoritmo de tu abuela(Bresenham) para dibujar una simple y tonta linea recta
{
    /*
     * Se que el algoritmo de Bresenham tiene como 60 años pero es ultra-rapido ademas de "simple", no veo necesario algo mas preciso :P
     * */
    int dx = abs(x1 - x0); // Distancia en horizontal
    int dy = abs(y1 - y0); // Distancia en horizontal
    /*
    Esto de arriba es como :
    Tenemos estos dos puntos (2,3) y (8,6)
    Con una simple resta tenemos la distancia entre ellos, que es Dx y Dy


    */

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    
    /*
    Lo de arriba es simplemente responder a la pregunta : voy derecha o izquierda y lo mismo verticalmente
    */

    int err = dx - dy; // el bresenham no usa pendiente real, solo error acumulado, osea no usa decimales y pues nos tenemos que adaptar a esta cosa hecha para los años 60.

    while (1)
    {
        putPixel(buffer, width, height, x0, y0, '#');
      

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void
drawLine3D(char *buffer, int width, int height, Vec3 a, Vec3 b, Camera cam) // aqui llega lo ricolin
{
    Vec3 ra, rb; // punto A relativo a la camara y lo mismo con b, en otras palabras son versiones transformadas de A y B 

    ra.x = a.x - cam.position.x;
    ra.y = a.y - cam.position.y;
    ra.z = a.z - cam.position.z;

    rb.x = b.x - cam.position.x;
    rb.y = b.y - cam.position.y;
    rb.z = b.z - cam.position.z;
    

    // rotar por yaw
    rotateY(&ra, -cam.yaw); // ¿Porque en negativo? ; porque no giramos la camara, movemos el flipante mundo al reves.
    rotateY(&rb, -cam.yaw);

    // rotar por pitch
    rotateX(&ra, -cam.pitch);
    rotateX(&rb, -cam.pitch);
    // CLIPPING contra near plane
    // El near plane hace que si algo esta detras no se vea y ya, si no esta ocasiona que todas las aristas se renderizen, y pues... no queremos eso
    
    // Ra y Rb atras = BORRAR
    if (ra.z <= cam.nearPlane && rb.z <= cam.nearPlane)
        return;

    // Si solo es a, pues solo quitamos a
    if (ra.z <= cam.nearPlane)
    {
        float t = (cam.nearPlane - ra.z) / (rb.z - ra.z); // Esto calcula cuanto avanzar desde A hacia B, interpolacion lineal

        ra.x = ra.x + t * (rb.x - ra.x);
        ra.y = ra.y + t * (rb.y - ra.y);
        ra.z = cam.nearPlane;
    }

    // La misma caquita, solo b solo se quita b
    if (rb.z <= cam.nearPlane)
    {
        float t = (cam.nearPlane - rb.z) / (ra.z - rb.z);

        rb.x = rb.x + t * (ra.x - rb.x);
        rb.y = rb.y + t * (ra.y - rb.y);
        rb.z = cam.nearPlane;
    }

    //* */Proyectar jamon
    int x0, y0, x1, y1;


    project(ra, &x0, &y0, cam, width, height);
    project(rb, &x1, &y1, cam, width, height);
    //**/ Dibujar como el buen pintor que soy
    drawLine(buffer, width, height, x0, y0, x1, y1); // AQUI ENTRA EL GUAPO DE BRESENHAM COMO LO ODIO Y LO AMO
}

void
present(char *buffer, int width, int height) // se escucha dificil pero pone el cursor arriba a la derecha
{
    printf("\033[H"); // mover cursor arriba izquierda

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            putchar(buffer[y * width + x]);
        }
        putchar('\n');
    }

    fflush(stdout);
}
