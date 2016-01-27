#include "dataProcess.h"
using namespace std;


void DataGroup::init(RootProcess& root){ //collcetive
	printf("process %d initing\n",comRank);
	nProcess = comSize;
	if(comRank==root.rank) // in root processes
		nProcess = root.rootgridList->size();

	mpiErr = MPI_Bcast(&nProcess, 1, MPI_INT,
			   root.rank, comm);CHECK(mpiErr);
	gridList = new int[nProcess];

	if(comRank==root.rank) // in root process;
 	for(int i=0;i!=nProcess;++i)
		gridList[i] = root.rootgridList->at(i);

	mpiErr = MPI_Bcast(gridList, nProcess, MPI_INT,
			   root.rank, comm);CHECK(mpiErr)
		
	printf("process: %d gridlist: ",comRank);
	nGlobal = 0;
	for(int i=0;i!=nProcess;++i){
		nGlobal+=gridList[i];
		printf("%d",gridList[i]);
	}
	nLocal = gridList[comRank];
	printf("\n");
	getchar();
	ierr = VecCreateMPI(comm,nLocal,nGlobal,&u);CHKERRQ(ierr); //init PETSC vec
	ierr = MatCreate(comm,&Au);PCHECK(ierr);CHKERRQ(ierr);     //init PETSC mat
	ierr = MatSetSizes(Au,nLocal,nLocal,nGlobal,nGlobal):CHKERRQ(ierr);
	ierr = MatSetType(Au,MATAIJ);CHKERRQ(ierr);
	ierr = MatMPIAIJSetPreallocation(A,MAX_ROW,NULL,MAX_ROW,NULL);CHKERRQ(ierr);	



	printf("datagrounp NO. %d init complete, dimension %d x %d = %d\n",comRank,nLocal,nProcess,nGlobal);
	//-----test purpose
	Avals = new double*[nLocal];
	Aposi = new int*[nLocal];
	Avals[0]  = new double[nLocal*MAX_ROW];
	Aposi[0]  = new int[nLocal*MAX_ROW];
	for(int i=1;i!=nLocal;++i){
		Avals[i] = &Avals[0][i*MAX_ROW];
		Aposi[i] = &Aposi[0][i*MAX_ROW];
	}
}

void DataGroup::fetchDataFrom(RootProcess& root){ //collective
	int sourceRank=root.rank;

	int destCount=0;
	int* sourceCount = new int[nProcess];
	int* offsets = new int[nProcess];
	double* localArray = NULL; 
	int* localArray2 = NULL;
	printf("start fetching data from root\n");
	offsets[0] = 0;
 	//*************************fetching vecotr U***************************	
	for(int i=1;i!=nProcess;++i){
		sourceCount[i] = gridList[i];
		offsets[i] = offsets[i-1] + sourceCount[i];
	}
	destCount = nLocal;
	mpiErr = VecGetArray(u,&localArray);PCHECK(mpiErr) //fetch raw pointer of PetscVector;

	mpiErr = MPI_Scatterv(root.rootuBuffer,sourceCount,offsets,MPI_DOUBLE,
			localArray,destCount,MPI_DOUBLE,sourceRank,comm); CHECK(mpiErr)
	/*
	for(int i=0;i!=nLocal;++i)	
		printf("array for process %d: %f\n",comRank,localArray[i]);
	*/

	mpiErr = VecRestoreArray(u,&localArray);PCHECK(mpiErr)

 	//*************************fetching Matrix A***************************	
	
	for(int i=1;i!=nProcess;++i){
		sourceCount[i] = gridList[i] * MAX_ROW; // preassume 5 nEntries
		offsets[i] = offsets[i-1] + sourceCount[i];
	}
	destCount = nLocal * MAX_ROW;
	localArray =  Avals[0];

	mpiErr = MPI_Scatterv(root.rootABuffer,sourceCount,offsets,MPI_DOUBLE,
			localArray,destCount,MPI_DOUBLE,sourceRank,comm); CHECK(mpiErr)
	
	localArray2 = Aposi[0];

	mpiErr = MPI_Scatterv(root.rootAPosiBuffer,sourceCount,offsets,MPI_INT,
			localArray2,destCount,MPI_INT,sourceRank,comm); CHECK(mpiErr)
	
	


	delete offsets;
	delete sourceCount;
	printf("complete fetching data from root\n");
}
void DataGroup::buildMatrix(){ //local but should involked in each processes
	int nonZeroDiagnal = 5, noneZeroOffDiagnal = 5;
	double* 
}



