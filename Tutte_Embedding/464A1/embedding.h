//
//  embedding.h
//  tutte_embedding
//
//  Created by Chen on 4/14/16.
//  Copyright © 2016 Qiao Chen. All rights reserved.
//

#ifndef embedding_h
#define embedding_h
#include <string.h>
#include "wing_edge_ds.h"
#include <stack>
#include "Eigen/Core"
#include "Eigen/Dense"
#include "Eigen/Sparse"
#include "umfpack.h"
#include "queue"

using namespace Eigen;


void SolveSparse(int* Ap,int* Ai,double* Ax,double* b,double*x, int n){
    void *Symbolic, *Numeric;
    
    /* symbolic analysis */
    umfpack_di_symbolic(n, n, Ap, Ai, Ax, &Symbolic, NULL, NULL);
    
    /* LU factorization */
    umfpack_di_numeric(Ap, Ai, Ax, Symbolic, &Numeric, NULL, NULL);
    umfpack_di_free_symbolic(&Symbolic);
    
    /* solve system */
    umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax, x, b, Numeric, NULL, NULL);
    umfpack_di_free_numeric(&Numeric);
    
    //for (int i=0;i<n;i++)
        //cout <<"i, x"<<i<<","<<x[i];
    
}

void assign_C(W_Vertex *v, MatrixXd &Cint,MatrixXd &Cext)
{
    W_Vertex * v2;
    v->valence= calculate_valence(v);
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
            Cint(v->internal_i,v2->internal_i)=1.0/valence;
            //cout<<1.0/valence<<" ";
        }
        else   // v2 is boundary vertex
            Cext(v->internal_i,v2->boundary_i)=(double)1.0/valence;
    }while(edge!=e0);
}


void embedding(W_Vertex * w_vertices)
{
    string urlstring="/Users/joseph/Documents/MATLAB/match_result.txt";
    string line;
    ifstream myfile (urlstring);
    int patchi=0;
    while ( !myfile.eof())
    {
        patchi++;
        cout<<patchi<<" \n";
        string buffer;
        int path_count[3];
        queue <W_Vertex*> boundarypoints;
        int boundary_i=0;
        int index;
        int i_path;
        for ( i_path=0;i_path<3;i_path++)
        {
            getline(myfile,line);
            stringstream ss(line);
            ss>>buffer;
            path_count[i_path]=stoi(buffer);
            for (int j=0;j<path_count[i_path];j++)  // read path points
            {

                     getline (myfile,line);
                     stringstream ss(line);
                     ss>>buffer;
                     index = stoi(buffer);
                     ss>>buffer;
                     float x= stof(buffer);
                
                     ss>>buffer;
                     float y= stof(buffer);
                    if(j==path_count[i_path]-1)
                        continue;
                     w_vertices[index].x = x;
                     w_vertices[index].y = y;
                     w_vertices[index].isboundary = 1;
                     w_vertices[index].boundary_i = boundary_i++;
                     boundarypoints.push(&w_vertices[index]);
            }
        }
        
        MatrixXd Xext(boundarypoints.size(),1);
        MatrixXd Yext(boundarypoints.size(),1);
        int count=0;
        while(boundarypoints.size())
        {
            //cout<<boundarypoints.front()->boundary_i;
            Xext(count,0)=boundarypoints.front()->x;
            Yext(count,0)=boundarypoints.front()->y;
            count++;
            boundarypoints.pop();
        }
        
        getline (myfile,line);          // reading internal vertices
        stringstream ss2(line);
        ss2>>buffer;
        if (buffer.compare("Internal:")==0)
        {
            ss2>>buffer;
            int inter_cnt=stoi(buffer);
            if(inter_cnt==0)
                continue;
            
            int intArray[inter_cnt];
           
            for (int i=0;i<inter_cnt;i++)
            {
                getline (myfile,line);          // reading internal indices
                stringstream ss(line);
                ss>>buffer;
                int index=stoi(buffer);
                w_vertices[index].internal_i=i;
                w_vertices[index].isboundary=0;
                intArray[i]=index;
            }
            
            MatrixXd Cext=MatrixXd::Zero(inter_cnt,Xext.rows());
            MatrixXd Cint=MatrixXd::Zero(inter_cnt,inter_cnt);
            

            for (int i=0;i<inter_cnt;i++)
            {
                assign_C(&w_vertices[intArray[i]],Cint,Cext);
            }
            // solve internal coordinates
            
            MatrixXd mtx_A=MatrixXd::Identity(inter_cnt,inter_cnt) - Cint;
            MatrixXd rightX= Cext * Xext;
            MatrixXd rightY= Cext * Yext;
            
            SparseMatrix<double> Sparse_A;
            
            Sparse_A = mtx_A.sparseView();
            double X_array[rightX.rows()];
            double Y_array[rightY.rows()];
            
            
            for (int row =0; row < rightX.rows();row++)
            {
                X_array[row]=rightX(row,0);
                Y_array[row]=rightY(row,0);
            }
            
            double Xint_array[rightX.rows()];
            double Yint_array[rightY.rows()];

            SolveSparse(Sparse_A.outerIndexPtr(),Sparse_A.innerIndexPtr(),Sparse_A.valuePtr(),X_array,Xint_array,rightX.rows());
            SolveSparse(Sparse_A.outerIndexPtr(),Sparse_A.innerIndexPtr(),Sparse_A.valuePtr(),Y_array,Yint_array,rightX.rows());
            
            for (int i=0;i<inter_cnt;i++)
            {
                w_vertices[intArray[i]].x=Xint_array[i];
                w_vertices[intArray[i]].y=Yint_array[i];
                if (patchi == 4)
                    cout<<Xint_array[i]<<","<<Yint_array[i]<<"|";
            }
            
            
        }
        
        
    }
}



#endif /* embedding_h */
