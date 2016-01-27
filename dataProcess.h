#include <fstream>
#include <stdio.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <petscvec.h>
#include <petscmat.h>
#ifndef _DATA_PROCESS_H_
#define _DATA_PROCESS_H_

#define MAX_ROW 5

#ifdef SHOULD_CHECK_MPI_ERROR 

#define CHECK(err) throwError("MPI_ERROR");
#define PCHECK(err) throwError("PETSC_ERROR");
#else
#define CHECK(err)
#define PCHECK(err)

#endif
/******************************************************
 *  	this data group lies each on a processor
 *  	a simple pack of u,v,w, etc.
 ******************************************************/
class RootProcess;
struct DataGroup{ 
	Vec u;
	Mat Au;
	PetscErrorCode ierr;
	MPI_Comm comm;
	int mpiErr;
	int comRank;
	int comSize;
	int nLocal;
	int nGlobal;
	int nProcess;
	int* gridList;  //size of nGlobal , gridList[comRank] == nLocal;
	double** Avals; //temp for file reading test;
	int** Aposi; 
	DataGroup():
		comm(MPI_COMM_WORLD),
		nLocal(0),
		nGlobal(0),
		nProcess(0),
		gridList(NULL),
		Avals(NULL), //preassume the size
		Aposi(NULL),
		errorCounter(0)
	{
		mpiErr = MPI_Comm_rank(comm,&comRank);CHECK(mpiErr)
		mpiErr = MPI_Comm_size(comm,&comSize);CHECK(mpiErr)
	}
	void init(RootProcess& root); // communicate to get local size on each processes , collective call
	void deinit(){ // a normal deconstructor seems not working in MPI
		printf("datagroup NO. %d died\n",comRank);
		ierr = VecDestroy(&u);PCHECK(ierr);
		delete []Avals;
		delete []Aposi;
		delete []gridList;
	}
	~DataGroup(){
	}
	void fetchDataFrom(RootProcess& root);
	void buildMatrix();
	int errorCounter;
	void throwError(const std::string& msg){ // only check error when SHOULD_CHECK_MPI_ERROR is defined
		char temp[256];
		printf("*****************************ON ERROR***************************************");
		sprintf(temp,"MPI_Failure in process %d\n message: %s\n",comRank,msg.c_str()); 
		printf("error msg: %s",temp);
		printf("error in call: %d",errorCounter++);
		printf("*****************************END ERROR***************************************");
		throw std::runtime_error(temp);
	}

};
/******************************************************
 *  	this class should work only in main processor
 ******************************************************/
class RootProcess{
public:
	int rank; //rank of the root;
	/*******************this part should only own by ROOT*****/ 
	std::ifstream* infileptr;
	int rootNGlobal;
	double* rootuBuffer;
	double* rootABuffer;
	int* rootAPosiBuffer;
	std::vector<int>* rootgridList;
	/*********************************************************/
	RootProcess(int r):
		rank(r),
		infileptr(NULL),
		rootNGlobal(-1),
		rootuBuffer(NULL), //NULL if not root
		rootABuffer(NULL),
		rootAPosiBuffer(NULL),
		rootgridList(NULL)
	{}
	~RootProcess(){
		if(rootuBuffer!=NULL)
			delete rootuBuffer;
		if(rootABuffer!=NULL)
			delete []rootABuffer;
		if(rootAPosiBuffer!=NULL)
			delete []rootAPosiBuffer;

	}
	void read(){ //should involk only in root process
		printf("start reading in root\n");
		char ctemp[256];
		int itemp=0;
		int maxRow = 0;
		infileptr = new std::ifstream("Au.dat");
		std::ifstream& infile = *infileptr;	
		infile>>ctemp>>rootNGlobal;
		infile>>ctemp>>itemp;
		infile>>ctemp>>maxRow;

		rootuBuffer = new double[rootNGlobal];
		rootABuffer = new double[rootNGlobal*MAX_ROW];
		rootAPosiBuffer = new int[rootNGlobal*MAX_ROW];

		for(int i=0;i!=rootNGlobal*MAX_ROW;++i){
			rootABuffer[i] = 0.0;
			rootAPosiBuffer[i] = -1; //-1 means no zeros in mat;
		}
		for(int i=0;i!=rootNGlobal;++i){
			size_t nCol = 0;
			double dump;
			double posi;
			double vals;
			double bVal;
			infile>>nCol>>ctemp;
			for(int j=0;j!=nCol;++j){
				infile>>posi>>vals;
				posi--;
				rootABuffer[i*MAX_ROW+j] = vals;
				rootAPosiBuffer[i*MAX_ROW+j] = posi;
			}
			infile>>ctemp>>bVal>>dump;
			rootuBuffer[i] = bVal;
		}
		infileptr->close();
		printf("complete reading in root\n");
		printf("now the input array is \n");
	}
	void partition(int N){
		printf("start partitioning in root \n");
		/*****************DATA PARTITION*******************/
		rootgridList = new std::vector<int>(N,0);
		int n = rootNGlobal / N;
		int counter = 0;
		for(std::vector<int>::iterator it = rootgridList->begin(); it!=rootgridList->end()-1; ++it){
			*it =  n;
			counter+=n;
		}
		rootgridList->back() = rootNGlobal - counter;
		/**************************************************/
		printf("complete partitioning in root \n");
	}
};
#endif
