/*********************************************
bankers.c
Matthew Frey
**********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct systemData {
    int numProcesses;
    int numResources;
    int* allocationMatrix;
    int* maxMatrix;
    int* availableMatrix;
    int* needsMatrix;
    int* safeMatrix;
    int* orderMatrix;
    int* workMatrix;
};

struct systemData getSystemData(char* filePath);
int getBankersMetrics(FILE* filePath, char* metricType);
int getBankersMatrix(FILE * fp, int* matrix, char* matrixType, int matrixHeight, int matrixWidth);
int* getNeedsMatrix(struct systemData sysData);
int* initProcessMatrix(int matrixSize);
int* initWorkMatrix(int* availableMatrix, int matrixSize);
void getOrderMatrix(struct systemData sysData);
void printResults(struct systemData sysData);
void freeMatrices(struct systemData sysData, int isError);
void freeMatrix(int* matrix, int isError);

int main(int argc, char *argv[]) {
    
    if(argc != 2) {
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        exit(-1);
    }
    char* filePath = argv[1];

    struct systemData sysData = getSystemData(filePath);

    sysData.needsMatrix = getNeedsMatrix(sysData);
    sysData.safeMatrix = initProcessMatrix(sysData.numProcesses);
    sysData.orderMatrix = initProcessMatrix(sysData.numProcesses);
    sysData.workMatrix = initWorkMatrix(sysData.availableMatrix, sysData.numResources);

    getOrderMatrix(sysData);
    
    printResults(sysData);

    freeMatrices(sysData, 0);
    return 0;
}

struct systemData getSystemData(char* filePath) {
    struct systemData sysData;
    char* numProcessesTag = "numProcesses";
    char* numResourcesTag = "numResources";
    char* allocationTag = "Allocation";
    char* maxTag = "Max";
    char* availableTag = "Available";

    FILE * fp;
    fp = fopen(filePath, "r");
    if(!fp) {
        fprintf(stderr, "Failed to open file location %s.\n", filePath);
        exit(-1);
    }
    
    sysData.numProcesses = getBankersMetrics(fp, numProcessesTag);
    sysData.numResources = getBankersMetrics(fp, numResourcesTag);

    int matrixSize = sysData.numProcesses * sysData.numResources;

    sysData.allocationMatrix = (int*)calloc(matrixSize, sizeof(int));
    if(getBankersMatrix(fp, sysData.allocationMatrix, allocationTag, sysData.numProcesses, sysData.numResources) == -1) {
        fclose(fp);
        freeMatrix(sysData.allocationMatrix, 1);
        exit(-1);
    }

    sysData.maxMatrix = (int*)calloc(matrixSize, sizeof(int));
    if(getBankersMatrix(fp, sysData.maxMatrix, maxTag, sysData.numProcesses, sysData.numResources) == -1) {
        fclose(fp);
        freeMatrix(sysData.allocationMatrix, 1);
        freeMatrix(sysData.maxMatrix, 1);
        exit(-1);
    }

    sysData.availableMatrix = (int*)calloc(sysData.numResources, sizeof(int));
    if(getBankersMatrix(fp, sysData.availableMatrix, availableTag, 1, sysData.numResources) == -1) {
        fclose(fp);
        freeMatrix(sysData.allocationMatrix, 1);
        freeMatrix(sysData.maxMatrix, 1);
        freeMatrix(sysData.availableMatrix, 1);
        exit(-1);
    }
    

    fclose(fp);
    return sysData;
}

int getBankersMetrics(FILE * fp, char* metricType) {
    char lines[150];
    int metric = -1;
    int typeFound = 0;

    while(!feof(fp)) {
        fgets(lines, 150, fp);
        if(strstr(lines, metricType)) {
            typeFound = 1;
            char* num = strtok(lines, "=");
            while(num != NULL) {
                metric = atoi(num);
                num = strtok(NULL, "=");
            }
            break;
        }
    }

    if (typeFound == 0) {
        fprintf(stderr, "%s not found in input file.\n", metricType);
        exit(-1);
    }
    return metric;
}

int getBankersMatrix(FILE * fp, int* matrix, char* matrixType, int matrixHeight, int matrixWidth) {
    char lines[150];
    int matrixIdentifier = 0;
    
    int i;
    int j;
    int k = 0;
    while(!feof(fp)) {
        fgets(lines, 150, fp);
        if(strstr(lines, matrixType)) {
            matrixIdentifier = 1;
            break;
        }
    }
    if(matrixIdentifier == 0) {
        fprintf(stderr, "%s matrix not found in input file.\n", matrixType);
        return(-1);
    }
    for(i = 0; i < matrixHeight; i++) {
        fgets(lines, 150, fp);
        if(strstr(lines, "]")) {
            break;
        }
        char* num = strtok(lines, " ");
        for(j = 0; j < matrixWidth; j++) {
            matrix[k] = atoi(num);
            num = strtok(NULL, " ");
            k++;
        }
    }

    return 0;
}

int* getNeedsMatrix(struct systemData sysData) {
    int matrixSize = sysData.numProcesses * sysData.numResources;

    sysData.needsMatrix = (int*)calloc(matrixSize, sizeof(int));
    
    int i;
    for(i=0; i<matrixSize; i++) {
        sysData.needsMatrix[i] = sysData.maxMatrix[i] - sysData.allocationMatrix[i];
        if(sysData.needsMatrix[i] < 0) {
            fprintf(stderr, "Allocation Matrix cannot contain value for a resource greater than Max Matrix.\n");
            freeMatrix(sysData.allocationMatrix, 1);
            exit(-1);
        }
     }

    return sysData.needsMatrix;
}

int* initProcessMatrix(int matrixSize) {
    int* processMatrix = (int*)calloc(matrixSize, sizeof(int));

    return processMatrix;
}

int* initWorkMatrix(int* allocationMatrix, int matrixSize) {
    int* workMatrix = (int*)calloc(matrixSize, sizeof(int));

    int i;
    for(i=0;i<matrixSize;i++) {
        workMatrix[i] = allocationMatrix[i];
    }

    return workMatrix;
}

void getOrderMatrix(struct systemData sysData) {

    int i;
    int j;
    int k;
    int meetsNeeds;
    int processesCompleted = 0;
    int lastCompleted = -1;
    while(1) {
        if(processesCompleted == sysData.numProcesses) {
            printf("%s", "System is in safe state.\n");
            printf("%s", "Safe process order: ");
            break;
        }
        if(processesCompleted == lastCompleted) {
            printf("%s", "System is in unsafe state.\nNumber of processes completed: ");
            printf("%d\n", processesCompleted);
            printf("%s", "Partial process order: ");
            break;
        }
        lastCompleted = processesCompleted;
        for(i=0;i<sysData.numProcesses;i++) { 
            if(!sysData.safeMatrix[i]) {
                meetsNeeds = 1;
                for(j=0;j<sysData.numResources;j++) {
                    k = i*sysData.numResources + j;
                    if(sysData.needsMatrix[k] > sysData.workMatrix[j]) {
                        meetsNeeds = 0;
                    }
                }
                if(meetsNeeds) {
                    sysData.safeMatrix[i] = 1;
                    sysData.orderMatrix[processesCompleted] = i;
                    processesCompleted++;
                    for(j=0;j<sysData.numResources;j++) {
                        k = i*sysData.numResources + j;
                        sysData.workMatrix[j] += sysData.allocationMatrix[k];
                    }
                }
            }
        }
    }

    return;
}

void printResults(struct systemData sysData) {
    int i;
    for(i=0;i<sysData.numProcesses;i++) {
        if(sysData.safeMatrix[i] == 1) {
            printf("P(%d) ", sysData.orderMatrix[i]);
        }
    }
    printf("\n");
}

void freeMatrices (struct systemData sysData, int isError) {
    if(sysData.allocationMatrix) {free(sysData.allocationMatrix);}
    if(sysData.maxMatrix) {free(sysData.maxMatrix);}
    if(sysData.availableMatrix) {free(sysData.availableMatrix);}
    if(sysData.needsMatrix) {free(sysData.needsMatrix);}
    if(sysData.safeMatrix) {free(sysData.safeMatrix);}
    if(sysData.orderMatrix) {free(sysData.orderMatrix);}
    if(sysData.workMatrix) {free(sysData.workMatrix);}
    if(isError) {
        exit(-1);
    }
}

void freeMatrix (int* matrix, int isError) {
    free(matrix);
}
