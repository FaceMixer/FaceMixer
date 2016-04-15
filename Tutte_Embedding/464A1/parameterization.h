#include <iostream>
#include <stack>
#include <assert.h>
#include "wing_edge_ds.h"
#include <math.h>
#include "tnt.h"
#include "jama_lu.h"
#include "Eigen/Core"
#include "Eigen/Dense"
#include "Eigen/Sparse"
#include "umfpack.h"

using namespace TNT;
using namespace Eigen;
W_Edge * find_next_boundary_edge(W_Edge * e0, W_Vertex * vertex)
{
    W_Edge *edge=e0;
    do{
        if(edge->start==vertex)
            edge=edge->right_prev;
        else
            edge=edge->left_prev;
    }while(edge==e0||edge->left!=NULL);
    return edge;
}

int calculate_valence(W_Vertex *v)
{
    int valence=0;
    W_Edge* e0=v->edge;
    W_Edge* edge=e0;
    
    do{                                                     // all edges that share v
        if(edge->end ==v) edge=edge->right_next;
        else edge=edge->left_next;
        valence++;
    }while(edge!=e0);
    return valence;
}

void assign_C_mtx(W_Vertex *v,Array2D<float> &Cint,Array2D<float> &Cext)
{
    W_Vertex * v2;
    int valence=v->valence;
    W_Edge* e0=v->edge;
    W_Edge* edge=e0;
    do{                                                     // all adjacent vertices v2
        if(edge->end ==v)
        {
            v2=edge->start;
            edge=edge->right_next;
        }
        else
        {
            v2=edge->end;
            edge=edge->left_next;
        }
        int k;                                       //assign C matrix
        if(!v2->isboundary)//v2 is internal vertex
        {
            k=v->internal_i;
            Cint[v->internal_i][v2->internal_i]=1.0/valence;
        }
        else   // v2 is boundary vertex
            Cext[v->internal_i][v2->boundary_i]=1.0/valence;
    }while(edge!=e0);
}


void output_mat(Array2D<float>&Mat_A,Array2D<float>&Mat_B,Array2D<float>&Mat_C,Array2D<float>&Mat_D)
{
    char* file_url="/Users/joseph/Documents/MATLAB/matA.output";
    ofstream myfile;
    myfile.open(file_url);
    if (myfile.is_open())
    {
        cout<<"dim1:"<<Mat_A.dim1()<<" d2:"<<Mat_A.dim2()<<"\n";
        for(int i=0;i<Mat_A.dim1();i++)
        {
            for(int j=0;j<Mat_A.dim2();j++)
            {
                myfile << Mat_A[i][j];
                if(j!=Mat_A.dim2()-1)
                    myfile << ", ";
            }
            myfile<<"\n";
        }
      
        myfile.close();
    }
    else cout << "Unable to open file";
    
    
    file_url="/Users/joseph/Documents/MATLAB/Cext.output";
    myfile.open(file_url);
    if (myfile.is_open())
    {
        cout<<"dim1:"<<Mat_B.dim1()<<" d2:"<<Mat_B.dim2()<<"\n";
        for(int i=0;i<Mat_B.dim1();i++)
        {
            for(int j=0;j<Mat_B.dim2();j++)
            {
                myfile << Mat_B[i][j];
                if(j!=Mat_B.dim2()-1)
                    myfile << ", ";
            }
            myfile<<"\n";
        }
        
        myfile.close();
    }
    else cout << "Unable to open file";
    
    file_url="/Users/joseph/Documents/MATLAB/Xext.output";
    myfile.open(file_url);
    if (myfile.is_open())
    {
        cout<<"dim1:"<<Mat_C.dim1()<<" d2:"<<Mat_C.dim2()<<"\n";
        for(int i=0;i<Mat_C.dim1();i++)
        {
            for(int j=0;j<Mat_C.dim2();j++)
            {
                myfile << Mat_C[i][j];
                if(j!=Mat_C.dim2()-1)
                    myfile << ", ";
            }
            myfile<<"\n";
        }
        
        myfile.close();
    }
    else cout << "Unable to open file";
    
    file_url="/Users/joseph/Documents/MATLAB/Yext.output";
    myfile.open(file_url);
    if (myfile.is_open())
    {
        cout<<"dim1:"<<Mat_D.dim1()<<" d2:"<<Mat_D.dim2()<<"\n";
        for(int i=0;i<Mat_D.dim1();i++)
        {
            for(int j=0;j<Mat_D.dim2();j++)
            {
                myfile << Mat_D[i][j];
                if(j!=Mat_D.dim2()-1)
                    myfile << ", ";
            }
            myfile<<"\n";
        }
        
        myfile.close();
    }
    else cout << "Unable to open file";

    
}

void read_proccessed_data(Array2D<float> &Xint, Array2D<float> &Yint)
{
    string urlstring="/Users/joseph/Documents/MATLAB/Xint.output";
    string line;
    ifstream myfile (urlstring);
    if (myfile.is_open())
    {
        int i=0;
        while ( getline (myfile,line) )
        {
            Xint[i++][0]=stof(line);
        }
        myfile.close();
    }
    else cout<<"File Error!";
    
    urlstring="/Users/joseph/Documents/MATLAB/Yint.output";
    myfile.open(urlstring);
    if (myfile.is_open())
    {
        int i=0;
        while ( getline (myfile,line) )
        {
            Yint[i++][0]=stof(line);
        }
        myfile.close();
    }
    else cout<<"File Error!";

}


void DemoTry2(int* Ap,int* Ai,double* Ax,double* b,double*x, int n){
    void *Symbolic, *Numeric;
    
    /* symbolic analysis */
    umfpack_di_symbolic(n, n, Ap, Ai, Ax, &Symbolic, NULL, NULL);
    
    /* LU factorization */
    umfpack_di_numeric(Ap, Ai, Ax, Symbolic, &Numeric, NULL, NULL);
    umfpack_di_free_symbolic(&Symbolic);
    
    /* solve system */
    umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax, x, b, Numeric, NULL, NULL);
    umfpack_di_free_numeric(&Numeric);
    
}

void solve_internal_coord(int &internal_num,int vertex_num,stack<W_Vertex*> &boundary_vertices,W_Vertex *w_vertices, adj_element ** adj_List)
{
    //internal_num--; // TODO REMOVE
    cout<<"Total Internal Points:"<<internal_num<<"\n";
    Array2D<float> Mtx_I(internal_num,internal_num,0.0);
    Array2D<float> Cint(internal_num,internal_num,0.0);
    Array2D<float> Xint(internal_num,1,0.0);
    Array2D<float> Yint(internal_num,1,0.0);
    Array2D<float> Cext(internal_num,(int)boundary_vertices.size(),0.0);
    Array2D<float> Xext((int)boundary_vertices.size(),1,0.0);
    Array2D<float> Yext((int)boundary_vertices.size(),1,0.0);

    
    int j=0;
    for(int i=1;i<=vertex_num;i++)
    {
        if (!w_vertices[i].isboundary) // for internal points
        {
            w_vertices[i].z=0;
            if(w_vertices[i].edge==NULL)
                continue;
            w_vertices[i].internal_i=j; // TODO ORDER
            j++;
            w_vertices[i].valence=calculate_valence(&w_vertices[i]);
        }
        else // for boundary points
        {
            Xext[w_vertices[i].boundary_i][0]=w_vertices[i].x;
            Yext[w_vertices[i].boundary_i][0]=w_vertices[i].y;
        }
    }
    
    for(int i=1;i<=vertex_num;i++)
    {
        if (!w_vertices[i].isboundary&&w_vertices[i].edge!=NULL) // for internal points
        {
            assign_C_mtx(&w_vertices[i],Cint,Cext);
        }
    }

    
    for(int i=0;i<internal_num;i++)
                Mtx_I[i][i]=1.0;
    
    Array2D<float> Mtx_A=Mtx_I-Cint;
   // output_mat(Mtx_A,Cext, Xext, Yext);
    
    MatrixXd Mtx_A0(Mtx_A.dim1(),Mtx_A.dim2());
    MatrixXd Mtx_Cext(Cext.dim1(),Cext.dim2());
    MatrixXd Mtx_Xext(Xext.dim1(),1);
    MatrixXd Mtx_Yext(Yext.dim1(),1);
    
    for (int row =0; row<Mtx_A.dim1(); row ++)
    {
        for ( int col =0;col<Mtx_A.dim2();col ++ )
        {
            Mtx_A0(row,col)=Mtx_A[row][col];
        }
    }
    
    for (int row =0; row<Cext.dim1(); row ++)
    {
        for ( int col =0;col<Cext.dim2();col ++ )
        {
            Mtx_Cext(row,col)=Cext[row][col];
        }
    }
    
    for (int row =0; row<Xext.dim1();row++)
    {
        Mtx_Xext(row,0)=Xext[row][0];
        Mtx_Yext(row,0)=Yext[row][0];
    }
    
    MatrixXd ResultX=Mtx_Cext*Mtx_Xext;
    MatrixXd ResultY=Mtx_Cext*Mtx_Yext;
    
    SparseMatrix<double> Sparse_A;
    
    Sparse_A = Mtx_A0.sparseView();
    double X_array[ResultX.rows()];
    double Y_array[ResultY.rows()];
    
    for (int row =0; row < ResultX.rows();row++)
    {
        X_array[row]=ResultX(row,0);
        Y_array[row]=ResultY(row,0);
    }
    double Xint_array[ResultX.rows()];
    double Yint_array[ResultY.rows()];
    
    DemoTry2(Sparse_A.outerIndexPtr(),Sparse_A.innerIndexPtr(),Sparse_A.valuePtr(),X_array,Xint_array,ResultX.rows());
    DemoTry2(Sparse_A.outerIndexPtr(),Sparse_A.innerIndexPtr(),Sparse_A.valuePtr(),Y_array,Yint_array,ResultX.rows());
    
    /*getchar();
    read_proccessed_data(Xint,Yint);
    for(int i=1;i<=vertex_num;i++)
    {
        if (!w_vertices[i].isboundary && w_vertices[i].edge!=NULL) // for internal points
        {
            w_vertices[i].x=Xint[w_vertices[i].internal_i][0];
            w_vertices[i].y=Yint[w_vertices[i].internal_i][0];
        }
    }
     */
    for(int i=1;i<=vertex_num;i++)
    {
        if (!w_vertices[i].isboundary && w_vertices[i].edge!=NULL) // for internal points
        {
            w_vertices[i].x=Xint_array[w_vertices[i].internal_i];
            w_vertices[i].y=Yint_array[w_vertices[i].internal_i];
        }
    }
    
}
