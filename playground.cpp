
static char help[] = "PETSC Playground\n";
#include <stdexcept>
#include <petscvec.h>
#include <petscmat.h>
#include <petscksp.h>
#include "dataProcess.h"


#undef __FUNCT__
#define __FUNCT__ "main"
int main(int argc,char **args) try
{
  //Vec            u;
  //Mat            A;
  PetscInt       m=8,n=4;
  PetscErrorCode ierr;
  //PetscScalar    v;

  int myID;
#if defined(PETSC_USE_LOG)
  //PetscLogStage stage;
#endif
  PetscInitialize(&argc,&args,(char*)0,help);
  if(n!=4) throw std::runtime_error("number of processors should be 4\n");
  MPI_Comm_rank(MPI_COMM_WORLD,&myID); 
  DataGroup* dataGroup = new DataGroup; // root will decide the size of each processes
  RootProcess root(0); //this argument must be the SAME for all processes

  /******************** ROOT ONLY ****************************************/
  if(dataGroup->comRank == 0){
	  root.rootuBuffer = new double[m*n];
	  root.read();
	  root.partition(n);
	  getchar();
  }
  /***********************************************************************/

  MPI_Barrier(PETSC_COMM_WORLD);

  dataGroup->init(root); // m,n is decided by root
  getchar();
  ierr = VecSet(dataGroup->u,0.0);CHKERRQ(ierr);
  VecAssemblyBegin(dataGroup->u);
  VecAssemblyEnd(dataGroup->u);
  dataGroup->fetchDataFrom(root);
  getchar();
  MPI_Barrier(PETSC_COMM_WORLD);
  if(dataGroup->comRank == 3){
	  printf("examin the data in process3\n");
	  for(int i=0;i!=dataGroup->nLocal;++i){
		  printf("element");
		  for(int j=0;j!=MAX_ROW;++j)
		  	printf(" %f at %d, ",dataGroup->Avals[i][j],dataGroup->Aposi[i][j]);
		  printf("\n");
	  }
	  
  }
  /*-------------------------------
   *	all processors works from here 
   *-------------------------------*/

  //ierr = PetscOptionsGetInt(NULL,"-m",&m,NULL);CHKERRQ(ierr);
  //ierr = PetscOptionsGetInt(NULL,"-n",&n,NULL);CHKERRQ(ierr);
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   * 	generate a Vector
   * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  //ierr = VecCreateMPI(PETSC_COMM_WORLD,m,m*n,&u);CHKERRQ(ierr);
  //ierr = VecSet(u,0.0);CHKERRQ(ierr);
 
  
  //printf("process %d : from %d --> %d\n",myID,myID*m,(myID+1)*m);
  /*--------------------------------
   * assigning to a vector in local order
   ---------------------------------*/
  /*
  v = 3.0 * (myID+1);
  int begin,end;
  ierr = VecGetOwnershipRange(u,&begin,&end);
  printf("process:%d return range : %d --> %d\n",myID,begin,end);
  for(int it = begin;it!=end;++it){ 
	 VecSetValue(u,it,v,INSERT_VALUES); 
  }
  */


  //ierr = VecAssemblyBegin(u); CHKERRQ(ierr);
  //ierr = VecAssemblyEnd(u); CHKERRQ(ierr);
  /*--------------------------------
   * view the vector 
   *-------------------------------*/
  //ierr = VecView(dataGroup->u,PETSC_VIEWER_DRAW_WORLD);CHKERRQ(ierr); //this vector is too big to view!
  //ierr = VecView(u,PETSC_VIEWER_DRAW_WORLD);CHKERRQ(ierr);

  
  /*--------------------------------
   * generate a matrix;
   *-------------------------------*/

  /*
   int noneZeroDiagnal = 5, noneZeroOffDiagnal = 5;
   double* values = new double[5];
   int* cols = new int[5];

   ierr = MatCreateMPIAIJ(PETSC_COMM_WORLD,m,m*n,m*n,m*n,
		   noneZeroDiagnal,PETSC_NULL,noneZeroOffDiagnal,PETSC_NULL,&A);CHKERRQ(ierr);
   
   ierr = MatAssemblyBegin(mat,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
   ierr = MatAssemblyEnd(mat,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);

   */
  /*
     Free work space.  All PETSc objects should be destroyed when they
     are no longer needed.
  */

  //ierr = VecDestroy(&u);CHKERRQ(ierr);
  dataGroup->deinit();
  delete dataGroup;
  //ierr = MatDestroy(&A);CHKERRQ(ierr);

  /*
     Always call PetscFinalize() before exiting a program.  This routine
       - finalizes the PETSc libraries as well as MPI
       - provides summary and diagnostic information if certain runtime
         options are chosen (e.g., -log_summary).
  */
  ierr = PetscFinalize();
  return 0;
}catch(std::runtime_error &e){
	printf("program failed with mpi runtime error: %s\n",e.what());
	getchar();
}
