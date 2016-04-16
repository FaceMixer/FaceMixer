#include "include/Angel.h"
#include "include/imageloader.h"
#include "include/funcNeed.h"

const int NumVertices = 30000; //max number of vertices
point4 points[NumVertices];
texture2 texture[NumVertices];
mat4  mvp;
GLuint  model_view;
GLuint textureId; //The id of the texture buffer
GLuint texId;


void initRendering(string address) {
    Image* grassimage = loadBMP(address);
    textureId = loadTexture(grassimage);
    delete grassimage;
}

// OpenGL initialization
void init()
{
    updateGeometry("./smf/face3D.smf","./smf/result1.smf",points, texture);
    
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    
    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(texture),NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(texture), texture );
    
    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "./vshader.glsl", "./fshader.glsl" );
    glUseProgram( program );
    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    
    // set up texture arrays
    GLuint texCoordID = glGetAttribLocation( program, "s_vTexCoord");
    glEnableVertexAttribArray( texCoordID );
    glVertexAttribPointer( texCoordID, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));
    
    model_view = glGetUniformLocation( program, "model_view" );
    texId = glGetUniformLocation( program, "texture_s" );
    glUniform1i(texId, 0);
    
    mvp *= Translate(0, 0, -1.0);
    glEnable( GL_DEPTH_TEST );
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
}

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glUniformMatrix4fv( model_view, 1, GL_TRUE, mvp );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
    
    glutPostRedisplay();
    glutSwapBuffers();
}

void keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
        case 033: // Escape Key
        case 'q': case 'Q':
            exit( EXIT_SUCCESS );
            break;
        case 'j': mvp *= RotateY(2);break;
        case 'l': mvp *= RotateY(-2);break;
        case 'i': mvp *= RotateX(2);break;
        case 'k': mvp *= RotateX(-2);break;
    }
    
    glutPostRedisplay();
}

void reshape( int width, int height )
{
    glViewport( 0, 0, width, height );
}

int main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
    glutCreateWindow( "FaceMixer" );
    
    init();
    initRendering("./bmp/tiger.bmp");
    
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
    glutReshapeFunc( reshape );
    
    glutMainLoop();
    return 0;
}
