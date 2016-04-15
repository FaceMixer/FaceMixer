#include "MacTypes.h"
#include <string.h>
#include "GLUI/glui.h"
#include <GLUT/glut.h>
#include <Cocoa/Cocoa.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <OpenGL/OpenGL.h>
#include <stack>
#include "glm.hpp"
#include "wing_edge_ds.h"
#include <math.h>
#include "parameterization.h"

using namespace std;

#define file_url "/tmp/output.smf"
#define file_url2 "/tmp/fixedpoints.output"
#define MATH_PI  3.14159265358979323846
float xy_aspect;
int   last_x, last_y;
float rotationX = 0.0, rotationY = 0.0;
float delta_x=0;
float delta_y=-0;
/** These are the live variables passed into GLUI ***/
int   wireframe = 0;
int   segments = 8;
int   light0_enabled = 1;
int   light1_enabled = 0;
float light0_intensity = 1.0;
float light1_intensity = 1.0;
int   main_window;
int   counter = 0;
float scale = 1;
int vertex_num = 0;
int face_num = 0;
int edge_num;
int boundary_point_num=0;
bool fileexist=false;
bool WireOn=false;
bool Fp_On=false;
bool TutteOn=false;
int shading_op;
GLfloat fixedpoint[15][2];
int fp_cnt=0;
int fp_index[15];

GLfloat rec_points[12][2]={{-0.4,-1.2},{-1.2,-1.2},{-1.2,-0.4},{-1.2,0.4},{-1.2,1.2},{-0.4,1.2},{0.4,1.2},{1.2,1.2},{1.2,0.4},{1.2,-0.4},{1.2,-1.2},{0.4,-1.2}};
stack<glm::vec3> v_boundary_faces;

GLuint DisplayList;			// object display list
GLuint WireFrameList;
GLuint SmoothList;
GLuint FpList;
GLuint TutteList;


/** Pointers to the windows and some of the controls we'll create **/
GLUI *cmd_line_glui=0, *glui;
GLUI_Panel       *obj_panel;
GLUI_Button      *open_console_btn;
GLUI_Listbox *listbox;
GLUI_FileBrowser *fb;

/********** User IDs for callbacks ********/
#define OPEN_CONSOLE_ID      100
#define CMD_HIST_RESET_ID    101
#define CMD_CLOSE_ID         102
#define LIGHT0_ENABLED_ID    200
#define LIGHT1_ENABLED_ID    201
#define LIGHT0_INTENSITY_ID  250
#define LIGHT1_INTENSITY_ID  251

/********** Miscellaneous global variables **********/

GLfloat light0_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
GLfloat light0_diffuse[] =  {.6f, .6f, 1.0f, 1.0f};
GLfloat light0_position[] = {.5f, .5f, 1.0f, 0.0f};

GLfloat light1_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
GLfloat light1_diffuse[] =  {.9f, .6f, 0.0f, 1.0f};
GLfloat light1_position[] = {-1.0f, -1.0f, 1.0f, 0.0f};


W_Vertex * w_vertices;
W_Face * w_faces;
adj_element **adj_List;
W_Edge ** w_edges_ptr_array;
stack<W_Edge*> w_edges;
stack<W_Edge*> w_edges_buffer;
stack<W_Vertex*> boundary_vertices;
stack<W_Vertex*> internal_vertices;


/*** flat_shaded mesh ***/
void
InitLists( )
{
    DisplayList = glGenLists( 1 );
    glNewList( DisplayList, GL_COMPILE );
    glBegin( GL_TRIANGLES);
    
    for(int i=1;i<=face_num;i++)
    {
        if(w_faces[i].edge==NULL)
            continue;
        glColor3f( 0., 0., 0.6 );
        W_Edge *e0 = w_faces[i].edge;
        W_Edge *edge = e0;
        glm::vec3 *points=(glm::vec3*)malloc(3*sizeof(glm::vec3));
        int j=0;
        do
        {   
            if(edge->right==&(w_faces[i]))
                edge=edge->right_prev;
            else
                edge=edge->left_prev;
            
            if(edge->left == &(w_faces[i]))
                points[j++]=glm::vec3(edge->end->x, edge->end->y, edge->end->z);
            else
                points[j++]=glm::vec3(edge->start->x, edge->start->y, edge->start->z);
        }while(edge!=e0);
        glm::vec3 normal=glm::cross((points[0]-points[1]),(points[1]-points[2]));
        normal=glm::normalize(normal);
        w_faces[i].normal=normal;
        glNormal3f(normal.x, normal.y, normal.z);
        glVertex3f( points[0].x, points[0].y, points[0].z);
        glVertex3f( points[1].x, points[1].y, points[1].z);
        glVertex3f( points[2].x, points[2].y, points[2].z);
        free(points);
    }
    
    glEnd( );
    
    glEndList( );
}

void
InitTutteLists( )
{
    TutteList = glGenLists( 1 );
    glNewList( TutteList, GL_COMPILE );
    glBegin( GL_TRIANGLES);
    
    for(int i=1;i<=face_num;i++)
    {
        if(w_faces[i].edge==NULL)
        continue;
        glColor3f( 0., 0., 0.6 );
        W_Edge *e0 = w_faces[i].edge;
        W_Edge *edge = e0;
        glm::vec3 *points=(glm::vec3*)malloc(3*sizeof(glm::vec3));
        int j=0;
        do
        {
            if(edge->right==&(w_faces[i]))
            edge=edge->right_prev;
            else
            edge=edge->left_prev;
            
            if(edge->left == &(w_faces[i]))
            points[j++]=glm::vec3(edge->end->x, edge->end->y, edge->end->z);
            else
            points[j++]=glm::vec3(edge->start->x, edge->start->y, edge->start->z);
        }while(edge!=e0);
        glm::vec3 normal=glm::cross((points[0]-points[1]),(points[1]-points[2]));
        normal=glm::normalize(normal);
        w_faces[i].normal=normal;
        glNormal3f(normal.x, normal.y, normal.z);
        glVertex3f( points[0].x, points[0].y, points[0].z);
        glVertex3f( points[1].x, points[1].y, points[1].z);
        glVertex3f( points[2].x, points[2].y, points[2].z);
        free(points);
    }
    
    glEnd( );
    
    glEndList( );
}

void init_fp_list(){
    WireOn=false;
    Fp_On =true;
    FpList = glGenLists(1);
    glNewList(FpList, GL_COMPILE);
    glBegin(GL_LINES);
    for (int i=0;i<15;i++)
    {
        glVertex3f(w_vertices[fp_index[i]].x,w_vertices[fp_index[i]].y,w_vertices[fp_index[i]].z);
    }
    glEnd();
    glEndList();
}

void parameterization_cb(int control)
{
                                                                    //1.find boundary points
    w_edges_buffer=w_edges;
    W_Edge * edge;
    bool firstflag=true;
    W_Edge * firstboundaryedge;
    while(w_edges_buffer.size()>0)
    {
        edge=w_edges_buffer.top();
        w_edges_buffer.pop();
        if (edge->left==NULL)  // boundary edge
        {
            edge->isboundary=true;
            if (edge->start->isboundary==false)
            {
                edge->start->isboundary=true;
                boundary_point_num++;
                edge->start->haspushed=true;
                if(firstflag)
                {
                    firstflag=false;
                    firstboundaryedge=edge;
                }
            }
            if (edge->end->isboundary==false)
            {
                edge->end->isboundary=true;
                boundary_point_num++;
                edge->end->haspushed=true;
            }
        }
    }
    
    w_edges_buffer=w_edges;
    while(w_edges_buffer.size()>0)
    {
        edge=w_edges_buffer.top();
        w_edges_buffer.pop();
    }
    
    cout<<"total boundary points#:"<<boundary_point_num<<"\n";
    if(boundary_point_num == 0 )
        return;
    
                                                                        // Store bounding points counter-clockwisely
    W_Edge* e0=firstboundaryedge;
    W_Edge* c_edge=e0;
    int j=0;
    do
    {
        c_edge->start->boundary_i=j++;
        boundary_vertices.push(c_edge->start);
        c_edge=find_next_boundary_edge(c_edge,c_edge->start);
    }while(c_edge!=e0);
    

                                                                        //2. Create Bounding Unit Circle
    W_Vertex * vertex=boundary_vertices.top();
    float theta0=-MATH_PI/2;//atan(vertex->y/vertex->x);
    float d_theta=2*MATH_PI/boundary_point_num;
    stack<W_Vertex*> boundary_vertices_buffer=boundary_vertices;
    int i=0;
    float theta;
    int bp_i=0;
    int prev_bp_i=0;
    W_Vertex * prev_vertex=NULL;
    W_Vertex *firstvertex=NULL;
    W_Vertex * tmpvertex;
    while(boundary_vertices_buffer.size()>0)
    {
        theta=theta0-i*d_theta;
        bp_i=(i*d_theta)/(MATH_PI/6);
        tmpvertex=boundary_vertices_buffer.top();
        boundary_vertices_buffer.pop();
        tmpvertex->x=cos(theta);
        tmpvertex->y=sin(theta);
        tmpvertex->z=0;
        if(prev_vertex==NULL)
            firstvertex=tmpvertex;
        else
        {
            v_boundary_faces.push(glm::vec3(prev_vertex->index,tmpvertex->index,vertex_num+bp_i+1));
        }
        if(bp_i!=prev_bp_i)
        {
            v_boundary_faces.push(glm::vec3(prev_vertex->index,vertex_num+bp_i+1,vertex_num+prev_bp_i+1));
            prev_bp_i=bp_i;
        }
        prev_vertex=tmpvertex;
        i++;
    }
    v_boundary_faces.push(glm::vec3(tmpvertex->index,firstvertex->index,vertex_num+bp_i+1));
    v_boundary_faces.push(glm::vec3(firstvertex->index,vertex_num+1,vertex_num+bp_i+1));
        prev_bp_i=bp_i;
                                                                   //3. Solve For Internal Coordinates
    int internal_num=vertex_num-boundary_point_num;
    solve_internal_coord(internal_num,vertex_num,boundary_vertices,w_vertices,adj_List);
    TutteOn=true;
    InitTutteLists();
}

void choose_point_cb( int control)
{
    if(control==0)
    {
        fp_cnt=0;
        cout<<"The points have been reset.\n";
        return;
    }
    if(fp_cnt<14)
    {
        cout<<"Need 15 points, current number:"<<fp_cnt<<"\n";
        return;
    }
    for(int i=0;i<15;i++)  // in the object, find the closest point to fixed points
    {
        float dist;
        int min_index=-1;
        float mindist=INFINITY;
        for(int j=1;j<=vertex_num;j++)
        {
            dist = pow(w_vertices[j].x-fixedpoint[i][0],2) + pow(w_vertices[j].y-fixedpoint[i][1],2);
            if(dist<mindist&&w_vertices[j].isfixed==false)
            {
                mindist=dist;
                min_index=w_vertices[j].index;
            }
        }
        fp_index[i]=min_index;
        w_vertices[min_index].isfixed=true;
    }
    
    ofstream myfile;
    myfile.open(file_url2);
    if (myfile.is_open())
    {
        for(int i=0;i<15;i++)
        {
            myfile<<fp_index[i];
            myfile<<"\n";
        }
        myfile.close();
    }
    else cout << "Unable to open file";
    init_fp_list();
}

void shading_cb( int control )
{
    switch (shading_op) {
        case 0:
            TutteOn=false;
             WireOn = false;
            glShadeModel( GL_FLAT );
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            break;
        case 1:
            TutteOn=false;
            WireOn = false;
            glShadeModel( GL_FLAT );
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            break;
    }
    GLUI_Master.sync_live_all();
    glutPostRedisplay();
}

void edge_op(W_Edge* prevedge, W_Edge *tmpedge, bool is_right)
{
    if(prevedge!=tmpedge)//for two counter-clockwise consecutive edges
    {
        if(is_right)
                tmpedge->right_next=prevedge;
        else
                tmpedge->left_next=prevedge;
        
        if (prevedge->end == tmpedge->start || prevedge->end == tmpedge->end)
                prevedge->left_prev=tmpedge;
        else if(prevedge->start == tmpedge->start || prevedge->start == tmpedge->end)
            prevedge->right_prev=tmpedge;
    }

}


/*File Manipulation Callback*/
void fileOpen(GLUI_Control* control ) //this function also initializes winged-edge structure
{
    NSOpenPanel* panel = [NSOpenPanel openPanel];
    [panel beginWithCompletionHandler:^(NSInteger result){
        if (result == NSFileHandlingPanelOKButton) {
            NSURL*  url = [[panel URLs] objectAtIndex:0];
            const char * urlstring= [url.absoluteString UTF8String];
            urlstring=urlstring+7;
            cout<<"File Path:"<<urlstring<<'\n';
            string line;
            ifstream myfile (urlstring);
            if (myfile.is_open())
            {
                int i=1;//vertex_number
                int j=1;//face_number
                
                getline (myfile,line);
                stringstream ss(line);
                string buffer;
                ss>>buffer;
                
                if (buffer.compare("#")==0)   // read first line
                {
                    ss>>buffer;
                    vertex_num=stoi(buffer);
                    ss>>buffer;
                    face_num=stoi(buffer);
                    w_vertices=(W_Vertex *)malloc((vertex_num+1)*sizeof(W_Vertex)); //vertex array
                    w_faces=(W_Face *)malloc((face_num+1)*sizeof(W_Face)); //face array
                    adj_List=(adj_element**)malloc((vertex_num+1)*sizeof(adj_element*));  // adjacent table
                    
                    //Initializing
                    memset(w_vertices, 0, (vertex_num+1)*sizeof(W_Vertex));
                    for (int i=0;i<=face_num;i++)
                    {
                        w_faces[i].edge=NULL;
                    }
                    for (int i=0;i<=vertex_num;i++)
                    {
                        adj_List[i]=(adj_element*)malloc((vertex_num+1)*sizeof(adj_element));
                        memset(adj_List[i],0,(vertex_num+1)*sizeof(adj_element));
                    }
                    
                }
                
                while ( getline (myfile,line) )
                {
                   // cout << line << '\n';
                    stringstream ss(line);
                    string buffer;
                    ss>>buffer;

                    
                    if (buffer.compare("v")==0) //read vertex lines
                    {
                        ss>>buffer;
                        w_vertices[i].x=stof(buffer);
                        ss>>buffer;
                        w_vertices[i].y=stof(buffer);
                        ss>>buffer;
                        w_vertices[i].z=stof(buffer);
                        w_vertices[i].index=i;
                        i++;
                    }
                    if (buffer.compare("f")==0) //read face lines
                    {
                        ss>>buffer;
                        int end=stoi(buffer); //counter clock wise
                        int head=end;
                        int tail=end;
                        W_Edge *prevedge=NULL;
                        W_Edge *firstedge=NULL;
                        bool isfirstright=false;
                        bool isfirst=true;
                        while(ss>>buffer) // create adjacent list by face info
                        {
                            head=stoi(buffer);
                            W_Edge *tmpedge=(W_Edge*)malloc(sizeof(W_Edge));
                            tmpedge->left=NULL;
                            tmpedge->right=NULL;
                            if(prevedge==NULL)
                                prevedge=tmpedge;
                            if((adj_List[head][tail]).adj==false && (adj_List[tail][head]).adj==false) // undefined
                            {
                                if (w_faces[j].edge==NULL)
                                {
                                    w_faces[j].edge=tmpedge;
                                    w_faces[j].index=j;
                                }
                                adj_List[head][tail].adj=true;
                                tmpedge->start=&(w_vertices[head]);
                                tmpedge->end=&(w_vertices[tail]);
                                tmpedge->right=&(w_faces[j]);
                                (adj_List[head][tail]).edge=tmpedge;
                                (w_vertices[head]).edge=tmpedge;
                                if(isfirst)
                                {
                                    isfirst=false;
                                    firstedge=tmpedge;
                                    isfirstright=true;
                                }
                                w_edges.push(tmpedge);
                                edge_op(prevedge, tmpedge,1);
                                prevedge=tmpedge;
                            }
                            else if (adj_List[tail][head].adj==true) {   //defined before
                                if (w_faces[j].edge==NULL)
                                {
                                    w_faces[j].edge=adj_List[tail][head].edge;
                                    w_faces[j].index=j;
                                }
                                (adj_List[tail][head].edge)->left=&(w_faces[j]);
                                edge_op(prevedge,adj_List[tail][head].edge,0);
                                prevedge=adj_List[tail][head].edge;
                                if(isfirst)
                                {
                                    isfirst=false;
                                    firstedge=adj_List[tail][head].edge;
                                }
                            }
                            tail=head;
                        }
                        
                        
                        if(adj_List[end][head].adj==false && adj_List[head][end].adj==false) // for last vertex
                        {
                            W_Edge *tmpedge=(W_Edge*)malloc(sizeof(W_Edge));
                            tmpedge->left=NULL;
                            tmpedge->right=NULL;
                            if (w_faces[j].edge==NULL)
                            {
                                w_faces[j].edge=tmpedge;
                                w_faces[j].index=j;
                            }
                            adj_List[end][head].adj=true;
                            tmpedge->start=&(w_vertices[end]);
                            tmpedge->end=&(w_vertices[head]);
                            tmpedge->right=&(w_faces[j]);
                            (adj_List[end][head]).edge=tmpedge;
                            (w_vertices[end]).edge=tmpedge;
                            w_edges.push(tmpedge);
                            edge_op(prevedge,tmpedge,1);
                            edge_op(tmpedge,firstedge,isfirstright);
                            prevedge=tmpedge;
                        }
                        else if ((adj_List[head][end]).adj==true)
                        {
                            if (w_faces[j].edge==NULL)
                            {
                                w_faces[j].edge=adj_List[head][end].edge;
                                w_faces[j].index=j;
                            }
                            ((adj_List[head][end]).edge)->left=&(w_faces[j]);
                            edge_op(prevedge, adj_List[head][end].edge,0);
                            edge_op(adj_List[head][end].edge,firstedge,isfirstright);
                            prevedge=adj_List[head][end].edge;
                        }
                        j++;
                    }
                }
                cout<<"Total Edges:"<<w_edges.size()<<"\n";
                myfile.close();
                fileexist=true;
                InitLists();
            }
            else cout << "Unable to open file";
        }
    }];
}
void fileSave(GLUI_Control* control )
{
    if(!fileexist)
        return ;
    ofstream myfile;
    myfile.open(file_url);
    if (myfile.is_open())
    {
        myfile << "# "<<vertex_num+12<<" "<<face_num+v_boundary_faces.size()<<"\n";
        for(int i=1;i<=vertex_num;i++)
            myfile << "v "<<w_vertices[i].x<<" "<<w_vertices[i].y<<" "<<w_vertices[i].z<<"\n";
        
        for(int i=0;i<12;i++)
            myfile << "v "<<rec_points[i][0]<<" "<<rec_points[i][1]<<" 0.0\n";
        
        for(int i=1;i<=face_num;i++)
        {
            W_Edge *e0 = w_faces[i].edge;
            W_Edge *edge = e0;
            myfile<<"f ";
            do
            {
                if(edge->right==&(w_faces[i]))
                    edge=edge->right_prev;
                else
                    edge=edge->left_prev;
                
                if(edge->left == &(w_faces[i]))
                    myfile<<((long)(edge->end)-(long)&(w_vertices[0]))/sizeof(W_Vertex)<<" ";
                else
                    myfile<<((long)(edge->start)-(long)&(w_vertices[0]))/sizeof(W_Vertex)<<" ";
            }while(edge!=e0);
            myfile<<"\n";
        }
        
        while(v_boundary_faces.size()>0)
        {
            myfile<<"f "<<v_boundary_faces.top().x<<" "<<v_boundary_faces.top().y<<" "<<v_boundary_faces.top().z<<"\n";
            v_boundary_faces.pop();
        }
        
        myfile.close();
    }
    else cout << "Unable to open file";
    
}



/**************************************** myGlutKeyboard() **********/

void myGlutKeyboard(unsigned char Key, int x, int y)
{
    switch(Key)
    {
        case 'q':
            exit(0);
            break;
        case 'e':
            scale+=0.1;
            break;
        case 'r':
            scale-=0.1;
            break;
        case 'w':
            delta_y+=0.1;
            break;
        case 's':
            delta_y-=0.1;
            break;
        case 'a':
            delta_x-=0.1;
            break;
        case 'd':
            delta_x+=0.1;
            break;
    };
    GLUI_Master.sync_live_all();
    glutPostRedisplay();
}


/***************************************** myGlutMenu() ***********/

void myGlutMenu( int value )
{
    myGlutKeyboard( value, 0, 0 );
}


/***************************************** myGlutIdle() ***********/

void myGlutIdle( void )
{
    if ( glutGetWindow() != main_window )
        glutSetWindow(main_window);
    
    glutPostRedisplay();
    counter++;
    glui->sync_live();
}

/***************************************** myGlutMouse() **********/

void myGlutMouse(int button, int button_state, int x, int y )
{
    if ( button == GLUT_LEFT_BUTTON && button_state == GLUT_DOWN ) {
        last_x = x;
        last_y = y;
        if(fp_cnt<15)
        {
        //cout<<"No."<<fp_cnt<<" (x,y):" << last_x<<","<<last_y<<"...";
        fixedpoint[fp_cnt][0]=(last_x-300.0)*3.0/2800.0;
        fixedpoint[fp_cnt][1]=(last_y-300.0)*3.0/2800.0;
        //cout<<fixedpoint[fp_cnt][0]<<","<<fixedpoint[fp_cnt][1]<<"\n";
        fp_cnt++;

        }
        else
        {
        cout<<"All points have been set.\n";
        }
    }
}

/***************************************** myGlutMotion() **********/

void myGlutMotion(int x, int y )
{
    rotationX += (float) (y - last_y);
    rotationY += (float) (x - last_x);
    
    last_x = x;
    last_y = y;
    
    glutPostRedisplay();
}

/**************************************** myGlutReshape() *************/

void myGlutReshape( int x, int y )
{
    xy_aspect = (float)x / (float)y;
    glViewport( 0, 0, x, y );
    
    glutPostRedisplay();
}

/***************************************** myGlutDisplay() *****************/

void myGlutDisplay( void )
{
    glClearColor(1, 1, 1, 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -xy_aspect*.08, xy_aspect*.08, -.08, .08, .1, 15.0 );
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0+delta_x, 0.0+delta_y, -1.2f );
    glRotatef( rotationY, 0.0, 1.0, 0.0 );
    glRotatef( rotationX, 1.0, 0.0, 0.0 );
    glShadeModel( GL_SMOOTH );
    glScalef( scale, scale, scale );
    glLineWidth(0.3);
   
    if(TutteOn==true)
    {
        glShadeModel( GL_FLAT );
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            glCallList(TutteList);
        }
    else
    {
        glCallList(DisplayList);
    }
    
    glutSwapBuffers();
    glFlush();
}

/**************************************** main() ********************/

int main(int argc, char* argv[])
{
    /****************************************/
    /*   Initialize GLUT and create window  */
    /****************************************/
    
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( 600, 600 );
    
    main_window = glutCreateWindow( "464 Assignment 1" );
    glutDisplayFunc( myGlutDisplay );
    glutReshapeFunc( myGlutReshape );
    glutKeyboardFunc( myGlutKeyboard );
    glutMotionFunc( myGlutMotion );
    glutMouseFunc( myGlutMouse );
    
    /****************************************/
    /*       Set up OpenGL lights           */
    /****************************************/
    
    glEnable(GL_LIGHTING);
    glEnable( GL_NORMALIZE );
    
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    
    
    glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glEnable ( GL_COLOR_MATERIAL );
    
    
    /****************************************/
    /*          Enable z-buferring          */
    /****************************************/
    
    glEnable(GL_DEPTH_TEST);
    
    /****************************************/
    /*         Here's the GLUI code         */
    /****************************************/
    
    printf( "GLUI version: %3.2f\n", GLUI_Master.get_version() );
    
    glui = GLUI_Master.create_glui( "Panel", 0, 800, 0 ); /* name, flags,
                                                           x, and y */
    
    /***********************Object Panel**********************/
    obj_panel = new GLUI_Panel(glui, "Shading" );
    listbox = new GLUI_Listbox(obj_panel,"",&shading_op,1,shading_cb);
    listbox->add_item(0,"flat shaded");
    listbox->add_item(1,"wire frame");
    new GLUI_Separator( obj_panel );
    
    /***********************Parameterization Button**********************/
    
    new GLUI_Button(glui, "Parameterization", 0, parameterization_cb);
    
    
    /***************************File Panel************************/
    GLUI_Panel *file_panel = new GLUI_Panel(glui, "File" );
    /*Buttons*/
    new GLUI_Button(file_panel, "Open", 0, fileOpen);
    new GLUI_Button(file_panel, "Save", 0, fileSave);
    new GLUI_Button(file_panel, "Quit", 0,(GLUI_Update_CB)exit );
    
    /**** Link windows to GLUI, and register idle callback ******/
    glui->set_main_gfx_window( main_window );
    /* We register the idle callback with GLUI, not with GLUT */
    GLUI_Master.set_glutIdleFunc( myGlutIdle );
    /**** Regular GLUT main loop ****/
    glutMainLoop();
    free(w_vertices);
    free(adj_List);
    free(w_faces);
    free(&w_edges);
    return EXIT_SUCCESS;
}




