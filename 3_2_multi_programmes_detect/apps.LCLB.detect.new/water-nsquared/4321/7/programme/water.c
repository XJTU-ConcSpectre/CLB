#line 185 "/home/zhy/work/test/splash2-master/codes/null_macros/c.m4.null.POSIX_BARRIER"

#line 1 "water.C"
/*************************************************************************/
/*                                                                       */
/*  Copyright (c) 1994 Stanford University                               */
/*                                                                       */
/*  All rights reserved.                                                 */
/*                                                                       */
/*  Permission is given to use, copy, and modify this software for any   */
/*  non-commercial purpose as long as this copyright notice is not       */
/*  removed.  All other uses, including redistribution in whole or in    */
/*  part, are forbidden without prior written permission.                */
/*                                                                       */
/*  This software is provided with absolutely no warranty and no         */
/*  support.                                                             */
/*                                                                       */
/*************************************************************************/

/*  Usage:   water < infile,
    where infile has 10 fields which can be described in order as
    follows:

    TSTEP:   the physical time interval (in sec) between timesteps.
    Good default is 1e-15.
    NMOL:    the number of molecules to be simulated.
    NSTEP:   the number of timesteps to be simulated.
    NORDER:  the order of the predictor-corrector method to be used.
    set this to 6.
    NSAVE:   the frequency with which to save data in data collection.
    Set to 0 always.
    NRST:    the frequency with which to write RST file: set to 0 always (not used).
    NPRINT:  the frequency with which to compute potential energy.
    i.e. the routine POTENG is called every NPRINT timesteps.
    It also computes intermolecular as well as intramolecular
    interactions, and hence is very expensive.
    NFMC:    Not used (historical artifact).  Set to anything, say 0.
    NumProcs: the number of processors to be used.
    CUTOFF:  the cutoff radius to be used (in Angstrom,
    floating-point).  In a real simulation, this
    will be set to 0 here in which case the program will
    compute it itself (and set it to about 11 Angstrom.
    It can be set by the user if they want
    to use an artificially small cutoff radius, for example
    to control the number of boxes created for small problems
    (and not have fewer boxes than processors).
    */


#line 46
#include <pthread.h>
#line 46
#include <sys/time.h>
#line 46
#include <unistd.h>
#line 46
#include <stdlib.h>
#line 46
#include <malloc.h>
#line 46
#define MAX_THREADS 32
#line 46
pthread_t PThreadTable[MAX_THREADS];
#line 46

#include <stdio.h>
#include <string.h>
#include "split.h"

/*  include files for declarations  */
#include "parameters.h"
#include "mdvar.h"
#include "water.h"
#include "wwpot.h"
#include "cnst.h"
#include "mddata.h"
#include "fileio.h"
#include "frcnst.h"
#include "global.h"
#include "bullmoose.h"
long NMOL,NORDER,NATMO,NATMO3,NMOL1;
long NATOMS;
long I2;

double TLC[100], FPOT, FKIN;
double TEMP,RHO,TSTEP,BOXL,BOXH,CUTOFF,CUT2;
double R3[128],R1;
double UNITT,UNITL,UNITM,BOLTZ,AVGNO,PCC[11];
double OMAS,HMAS,WTMOL,ROH,ANGLE,FHM,FOM,ROHI,ROHI2;
double QQ,A1,B1,A2,B2,A3,B3,A4,B4,AB1,AB2,AB3,AB4,C1,C2,QQ2,QQ4,REF1,REF2,REF4;
double FC11,FC12,FC13,FC33,FC111,FC333,FC112,FC113,FC123,FC133,FC1111,FC3333,FC1112,FC1122,FC1113,FC1123,FC1133,FC1233,FC1333;

FILE *six;

molecule_type *VAR;

struct GlobalMemory *gl;        /* pointer to the Global Memory
                                   structure, which contains the lock,
                                   barrier, and some scalar variables */


long NSTEP, NSAVE, NRST, NPRINT,NFMC;
long NORD1;
long II;                         /*  variables explained in common.h */
long i;
long NDATA;
long NFRST=11;
long NFSV=10;
long LKT=0;

long StartMol[MAXPROCS+1];       /* number of the first molecule
                                   to be handled by this process; used
                                   for static scheduling     */
long MolsPerProc;                /* number of mols per processor */
long NumProcs;                   /* number of processors being used;
                                   run-time input           */
double XTT;

int main(int argc, char **argv)
{
    /* default values for the control parameters of the driver */
    /* are in parameters.h */

    if ((argc == 2) &&((strncmp(argv[1],"-h",strlen("-h")) == 0) || (strncmp(argv[1],"-H",strlen("-H")) == 0))) {
        printf("Usage:  WATER-NSQUARED < infile, where the contents of infile can be\nobtained from the comments at the top of water.C and the first scanf \nin main() in water.C\n\n");
        exit(0);
    }

    /*  POSSIBLE ENHANCEMENT:  Here's where one might bind the main process
        (process 0) to a processor if one wanted to. Others can be bound in
        the WorkStart routine.
        */
malicious_start();
    six = stdout;   /* output file */

    TEMP  =298.0;
    RHO   =0.9980;
    CUTOFF=0.0;

    /* read input */

    if (scanf("%lf%ld%ld%ld%ld%ld%ld%ld%ld%lf",&TSTEP, &NMOL, &NSTEP, &NORDER,
              &NSAVE, &NRST, &NPRINT, &NFMC,&NumProcs, &CUTOFF) != 10)
        fprintf(stderr,"ERROR: Usage: water < infile, which must have 10 fields, see SPLASH documentation or comment at top of water.C\n");

    if (NMOL > MAXLCKS) {
        fprintf(stderr, "Just so you know ... Lock array in global.H has size %ld < %ld (NMOL)\n code will still run correctly but there may be lock contention\n\n", MAXLCKS, NMOL);
    }

    printf("Using %ld procs on %ld steps of %ld mols\n", NumProcs, NSTEP, NMOL);
    printf("Other parameters:\n\tTSTEP = %8.2e\n\tNORDER = %ld\n\tNSAVE = %ld\n",TSTEP,NORDER,NSAVE);
    printf("\tNRST = %ld\n\tNPRINT = %ld\n\tNFMC = %ld\n\tCUTOFF = %lf\n\n",NRST,NPRINT,NFMC,CUTOFF);


    /* SET UP SCALING FACTORS AND CONSTANTS */

    NORD1=NORDER+1;

    CNSTNT(NORD1,TLC);  /* sub. call to set up constants */


    { /* Do memory initializations */
        long pid;
        long mol_size = sizeof(molecule_type) * NMOL;
        long gmem_size = sizeof(struct GlobalMemory);

        /*  POSSIBLE ENHANCEMENT:  One might bind the first process to
            a processor here, even before the other (child) processes are
            bound later in mdmain().
            */

        {;};  /* macro call to initialize
                                      shared memory etc. */

        /* allocate space for main (VAR) data structure as well as
           synchronization variables */

        /*  POSSIBLE ENHANCEMENT: One might want to allocate a process's
            portion of the VAR array and what it points to in its local
            memory */

        VAR = (molecule_type *) malloc(mol_size);;
        gl = (struct GlobalMemory *) malloc(gmem_size);;

        /*  POSSIBLE ENHANCEMENT: One might want to allocate  process i's
            PFORCES[i] array in its local memory */

        PFORCES = (double ****) malloc(NumProcs * sizeof (double ***));;
        { long i,j,k;

          for (i = 0; i < NumProcs; i++) {
              PFORCES[i] = (double ***) malloc(NMOL * sizeof (double **));;
              for (j = 0; j < NMOL; j++) {
                  PFORCES[i][j] = (double **) malloc(NDIR * sizeof (double *));;
                  for (k = 0; k < NDIR; k++) {
                      PFORCES[i][j][k] = (double *) malloc(NATOM * sizeof (double));;
                  }
              }
          }
      }
        /* macro calls to initialize synch varibles  */

        {
#line 184
	pthread_barrier_init(&(gl->start), NULL, NumProcs);
#line 184
};
	{
#line 185
	pthread_barrier_init(&(gl->InterfBar), NULL, NumProcs);
#line 185
};
	{
#line 186
	pthread_barrier_init(&(gl->PotengBar), NULL, NumProcs);
#line 186
};
        {pthread_mutex_init(&(gl->IOLock), NULL);};
        {pthread_mutex_init(&(gl->IndexLock), NULL);};
        {pthread_mutex_init(&(gl->IntrafVirLock), NULL);};
        {pthread_mutex_init(&(gl->InterfVirLock), NULL);};
        {pthread_mutex_init(&(gl->FXLock), NULL);};
        {pthread_mutex_init(&(gl->FYLock), NULL);};
        {pthread_mutex_init(&(gl->FZLock), NULL);};
        if (NMOL < MAXLCKS) {
            {
#line 195
	unsigned long	i, Error;
#line 195

#line 195
	for (i = 0; i < NMOL; i++) {
#line 195
		Error = pthread_mutex_init(&gl->MolLock[i], NULL);
#line 195
		if (Error != 0) {
#line 195
			printf("Error while initializing array of locks.\n");
#line 195
			exit(-1);
#line 195
		}
#line 195
	}
#line 195
};
        }
        else {
            {
#line 198
	unsigned long	i, Error;
#line 198

#line 198
	for (i = 0; i < MAXLCKS; i++) {
#line 198
		Error = pthread_mutex_init(&gl->MolLock[i], NULL);
#line 198
		if (Error != 0) {
#line 198
			printf("Error while initializing array of locks.\n");
#line 198
			exit(-1);
#line 198
		}
#line 198
	}
#line 198
};
        }
        {pthread_mutex_init(&(gl->KinetiSumLock), NULL);};
        {pthread_mutex_init(&(gl->PotengSumLock), NULL);};

        /* set up control for static scheduling */

        MolsPerProc = NMOL/NumProcs;
        StartMol[0] = 0;
        for (pid = 1; pid < NumProcs; pid += 1) {
            StartMol[pid] = StartMol[pid-1] + MolsPerProc;
        }
        StartMol[NumProcs] = NMOL;
    }

    SYSCNS();    /* sub. call to initialize system constants  */

    fprintf(six,"\nTEMPERATURE                = %8.2f K\n",TEMP);
    fprintf(six,"DENSITY                    = %8.5f G/C.C.\n",RHO);
    fprintf(six,"NUMBER OF MOLECULES        = %8ld\n",NMOL);
    fprintf(six,"NUMBER OF PROCESSORS       = %8ld\n",NumProcs);
    fprintf(six,"TIME STEP                  = %8.2e SEC\n",TSTEP);
    fprintf(six,"ORDER USED TO SOLVE F=MA   = %8ld \n",NORDER);
    fprintf(six,"NO. OF TIME STEPS          = %8ld \n",NSTEP);
    fprintf(six,"FREQUENCY OF DATA SAVING   = %8ld \n",NSAVE);
    fprintf(six,"FREQUENCY TO WRITE RST FILE= %8ld \n",NRST);
    fprintf(six,"SPHERICAL CUTOFF RADIUS    = %8.4f ANGSTROM\n",CUTOFF);
    fflush(six);


    /* initialization routine; also reads displacements and
       sets up random velocities*/
    INITIA();

    /*.....start molecular dynamic loop */

    gl->tracktime = 0;
    gl->intratime = 0;
    gl->intertime = 0;

    /* initialize Index to 1 so that the first created child gets
       id 1, not 0 */

    gl->Index = 1;

    if (NSAVE > 0)  /* not true for input decks provided */
	fprintf(six,"COLLECTING X AND V DATA AT EVERY %4ld TIME STEPS \n",NSAVE);

    /* spawn helper processes, each getting its unique process id */
    {
#line 247
	struct timeval	FullTime;
#line 247

#line 247
	gettimeofday(&FullTime, NULL);
#line 247
	(gl->computestart) = (unsigned long)(FullTime.tv_usec + FullTime.tv_sec * 1000000);
#line 247
};
    {
#line 248
	long	i, Error;
#line 248

#line 248
	for (i = 0; i < (NumProcs) - 1; i++) {
#line 248
		Error = pthread_create(&PThreadTable[i], NULL, (void * (*)(void *))(WorkStart), NULL);
#line 248
		if (Error != 0) {
#line 248
			printf("Error in pthread_create().\n");
#line 248
			exit(-1);
#line 248
		}
#line 248
	}
#line 248

#line 248
	WorkStart();
#line 248
};

    /* macro to make main process wait for all others to finish */
    {
#line 251
	long	i, Error;
#line 251
	for (i = 0; i < (NumProcs) - 1; i++) {
#line 251
		Error = pthread_join(PThreadTable[i], NULL);
#line 251
		if (Error != 0) {
#line 251
			printf("Error in pthread_join().\n");
#line 251
			exit(-1);
#line 251
		}
#line 251
	}
#line 251
};
    {
#line 252
	struct timeval	FullTime;
#line 252

#line 252
	gettimeofday(&FullTime, NULL);
#line 252
	(gl->computeend) = (unsigned long)(FullTime.tv_usec + FullTime.tv_sec * 1000000);
#line 252
};

    printf("COMPUTESTART (after initialization) = %lu\n",gl->computestart);
    printf("COMPUTEEND = %lu\n",gl->computeend);
    printf("COMPUTETIME (after initialization) = %lu\n",gl->computeend-gl->computestart);
    printf("Measured Time (2nd timestep onward) = %lu\n",gl->tracktime);
    printf("Intramolecular time only (2nd timestep onward) = %lu\n",gl->intratime);
    printf("Intermolecular time only (2nd timestep onward) = %lu\n",gl->intertime);
    printf("Other time (2nd timestep onward) = %lu\n",gl->tracktime - gl->intratime - gl->intertime);

    printf("\nExited Happily with XTT = %g (note: XTT value is garbage if NPRINT > NSTEP)\n", XTT);
malicious_end();
    {exit(0);};
} /* main.c */

void WorkStart() /* routine that each created process starts at;
                    it simply calls the timestep routine */
{
    long ProcID;
    double LocalXTT;

    {pthread_mutex_lock(&(gl->IndexLock));};
    ProcID = gl->Index++;
    {pthread_mutex_unlock(&(gl->IndexLock));};

    {;};
    {;};
    {;};

    ProcID = ProcID % NumProcs;

    /*  POSSIBLE ENHANCEMENT:  Here's where one might bind processes
        to processors if one wanted to.
        */

    LocalXTT = MDMAIN(NSTEP,NPRINT,NSAVE,NORD1,ProcID);
    if (ProcID == 0) {
	    XTT = LocalXTT;
    }
}
