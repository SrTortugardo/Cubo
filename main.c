#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include "render.h"
int main()
{
    int width,height;
    getTerminalSize(&width,&height);
    char *buffer=malloc(width*height);
    if(!buffer) return 1;
    enableRaw();
    fcntl(STDIN_FILENO,F_SETFL,O_NONBLOCK);
    Camera cam;
    cam.position=(Vec3){0,0,-8};
    cam.nearPlane=0.1f;
    cam.fov=40.0f;
    cam.yaw=0.0f;
    cam.pitch=0.0f;
    float s=2.0f;
    Vec3 cube[8]={
        {-s,-s,5},{ s,-s,5},{ s, s,5},{-s, s,5},
        {-s,-s,9},{ s,-s,9},{ s, s,9},{-s, s,9}
    };
    int cubeEdges[12][2]={
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };
    float sphereX=6.0f;
    float sphereZ=14.0f;
    float radius=2.5f;
    int latSteps=10;
    int lonSteps=16;
    while(1)
    {
        clearBuffer(buffer,width,height);
        for(int i=0;i<12;i++)
        {
            drawLine3D(buffer,width,height,
                       cube[cubeEdges[i][0]],
                       cube[cubeEdges[i][1]],
                       cam);
        }
        for(int i=0;i<latSteps;i++)
        {
            float theta1=M_PI*(-0.5f+(float)i/latSteps);
            float theta2=M_PI*(-0.5f+(float)(i+1)/latSteps);
            for(int j=0;j<lonSteps;j++)
            {
                float phi1=2*M_PI*(float)j/lonSteps;
                float phi2=2*M_PI*(float)(j+1)/lonSteps;
                Vec3 p1={sphereX+radius*cosf(theta1)*cosf(phi1),
                         radius*sinf(theta1),
                         sphereZ+radius*cosf(theta1)*sinf(phi1)};
                Vec3 p2={sphereX+radius*cosf(theta1)*cosf(phi2),
                         radius*sinf(theta1),
                         sphereZ+radius*cosf(theta1)*sinf(phi2)};
                Vec3 p3={sphereX+radius*cosf(theta2)*cosf(phi1),
                         radius*sinf(theta2),
                         sphereZ+radius*cosf(theta2)*sinf(phi1)};
                drawLine3D(buffer,width,height,p1,p2,cam);
                drawLine3D(buffer,width,height,p1,p3,cam);
            }
        }
        present(buffer,width,height);
        char key;
        if(read(STDIN_FILENO,&key,1)>0)
        {
            float speed=0.5f;
            float forwardX=sinf(cam.yaw);
            float forwardZ=cosf(cam.yaw);
            float rightX=cosf(cam.yaw);
            float rightZ=-sinf(cam.yaw);
            if(key=='q') break;
            if(key=='w'){
                cam.position.x+=forwardX*speed;
                cam.position.z+=forwardZ*speed;
            }
            if(key=='s'){
                cam.position.x-=forwardX*speed;
                cam.position.z-=forwardZ*speed;
            }
            if(key=='a'){
                cam.position.x-=rightX*speed;
                cam.position.z-=rightZ*speed;
            }
            if(key=='d'){
                cam.position.x+=rightX*speed;
                cam.position.z+=rightZ*speed;
            }
            if(key=='h') cam.yaw-=0.05f;
            if(key=='l') cam.yaw+=0.05f;
            if(key=='k') cam.pitch+=0.05f;
            if(key=='j') cam.pitch-=0.05f;
        }
        usleep(16000);
    }
    disableRaw();
    free(buffer);
    return 0;
}
