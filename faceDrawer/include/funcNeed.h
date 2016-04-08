using namespace std;
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
typedef Angel::vec2  texture2;
#include <iostream>
#include <string>
#include <fstream>

void divideStringBySpace(char* origin,char derivative[4][10]){
    int i = 0,j = 0,k = 0;
    
    while(origin[i] != '\r' && origin[i] != '\n' && origin[i] != '\0'){
        while(origin[i]!=' ' && origin[i] != '\r' && origin[i] != '\n' && origin[i] != '\0')
        {
            derivative[j][k] = origin[i];
            k++;
            i++;
        }
        if(origin[i] == '\r' || origin[i] == '\n' || origin[i] == '\0') break;
        derivative[j][k] = '\0';
        k = 0;
        j++;
        i++;
    }
    derivative[j][k] = '\0';
    derivative[j][k+1] = '\0';
}

//Makes the image into a texture, and returns the id of the texture
GLuint loadTexture(Image* image) {
    GLuint textureId;
    glGenTextures(1, &textureId); //Make room for our texture
    glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit
    //Map the image to the texture
    glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
                 0,                            //0 for now
                 GL_RGB,                       //Format OpenGL uses for image
                 image->width, image->height,  //Width and height
                 0,                            //The border of the image
                 GL_RGB, //GL_RGB, because pixels are stored in RGB format
                 GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
                 //as unsigned numbers
                 image->pixels);               //The actual pixel data
    return textureId; //Returns the id of the texture
}

void updateGeometry(string face3D,string face2D, point4 *points, texture2 *texture){
    string line;
    char line_[90];
    char lineSplit[4][10];
    int face[30000][3];
    float vert[15000][3];
    int face2[30000][3];
    float vert2[15000][3];
    int numOfVert;
    int numOfFace;
    int numOfVert2;
    int numOfFace2;
    int count = 0;
    
    ifstream input(face3D);
    ifstream input2(face2D);
    
    getline(input,line);
    strcpy(line_, line.c_str());
    divideStringBySpace(line_, lineSplit);
    numOfVert = atoi(lineSplit[1]);
    numOfFace = atoi(lineSplit[2]);
    
    for (int i = 0;i < numOfVert;i++){
        getline(input,line);
        strcpy(line_, line.c_str());
        divideStringBySpace(line_, lineSplit);
        vert[i][0] = atof(lineSplit[1]);
        vert[i][1] = atof(lineSplit[2]);
        vert[i][2] = atof(lineSplit[3]);
    }
    
    for (int i = 0;i < numOfFace;i++){
        getline(input,line);
        strcpy(line_, line.c_str());
        divideStringBySpace(line_, lineSplit);
        face[i][0] = atoi(lineSplit[1]);
        face[i][1] = atoi(lineSplit[2]);
        face[i][2] = atoi(lineSplit[3]);
    }
    
    getline(input2,line);
    strcpy(line_, line.c_str());
    divideStringBySpace(line_, lineSplit);
    numOfVert2 = atoi(lineSplit[1]);
    numOfFace2 = atoi(lineSplit[2]);
    
    for (int i = 0;i < numOfVert2;i++){
        getline(input2,line);
        strcpy(line_, line.c_str());
        divideStringBySpace(line_, lineSplit);
        vert2[i][0] = atof(lineSplit[1]);
        vert2[i][1] = atof(lineSplit[2]);
        vert2[i][2] = atof(lineSplit[3]);
    }
    
    for (int i = 0;i < numOfFace2;i++){
        getline(input2,line);
        strcpy(line_, line.c_str());
        divideStringBySpace(line_, lineSplit);
        face2[i][0] = atoi(lineSplit[1]);
        face2[i][1] = atoi(lineSplit[2]);
        face2[i][2] = atoi(lineSplit[3]);
    }
    
    for (int i = 0;i < numOfFace;i++){
        int ind1 = face[i][0] - 1;
        int ind2 = face[i][1] - 1;
        int ind3 = face[i][2] - 1;
        
        int ind1_ = face2[i][0] - 1;
        int ind2_ = face2[i][1] - 1;
        int ind3_ = face2[i][2] - 1;
        
        float scale1 = 5.6;
        float scale2 = 5.6;
        
        float scale1_ = 1.0 / 2;
        float scale2_ = 1.0;
        
        vec4 p1 = vec4(vert[ind1][0]*scale1,vert[ind1][1]*scale1,vert[ind1][2]*scale2,1.0);
        vec4 p2 = vec4(vert[ind2][0]*scale1,vert[ind2][1]*scale1,vert[ind2][2]*scale2,1.0);
        vec4 p3 = vec4(vert[ind3][0]*scale1,vert[ind3][1]*scale1,vert[ind3][2]*scale2,1.0);
        
        vec4 p1_ = vec4(vert2[ind1_][0]*scale1_,vert2[ind1_][1]*scale1_,vert2[ind1_][2]*scale2_,1.0);
        vec4 p2_ = vec4(vert2[ind2_][0]*scale1_,vert2[ind2_][1]*scale1_,vert2[ind2_][2]*scale2_,1.0);
        vec4 p3_ = vec4(vert2[ind3_][0]*scale1_,vert2[ind3_][1]*scale1_,vert[ind3_][2]*scale2_,1.0);
        
        
        vec4 move = vec4(0.0,-1.0,0.0,0.0);
        
        p1 = p1 + move;
        p2 = p2 + move;
        p3 = p3 + move;
        
        points[count] = p1;
        texture[count] = vec2(p1_.x+1.0/2,p1_.y+1.0/2);
        
        points[count+1] = p2;
        texture[count+1] = vec2(p2_.x+1.0/2,p2_.y+1.0/2);
        
        points[count+2] = p3;
        texture[count+2] = vec2(p3_.x+1.0/2,p3_.y+1.0/2);
        
        count = count + 3;
    }
}

