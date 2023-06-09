//
//  main.c
//  Parallel and cluster computing final project 2020
//
//  Created by mahmoud mohamed and Bassil Osama Eloraby on 6/3/20.
//

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#define TEMPRATURE_DIFFERENCE_THRESHOLD 5

pthread_mutex_t mutex;
const int rows = 36;
const int cols = 46;
int a[rows][cols];

typedef struct {
    int endRows;
    int startRows;
    int endCols;
    int startCols;
    
} HeatProblemArgs;

void print2DArray(int rows, int cols, int a[rows][cols]) {
    for (int i = 0 ; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("Temp for point (%d,%d): %d \n", i, j, a[i][j]);
//            printf("%d \t", a[i][j]);
        }
        printf("\n");
    }
}

void sequentialSolution(int rows, int cols, int temperatures[rows][cols]) {
    for (int x = 1; x < rows; x++) {
        for (int y = 1; y < cols; y++) {
            int newTemp = ceil(0.2 * (a[x][y] + a[x][y-1] + a[x-1][y] + a[x][y+1] + a[x+1][y]));
            int oldTemp = a[x][y];
            a[x][y] = newTemp;
            
            if (abs(oldTemp - newTemp) > TEMPRATURE_DIFFERENCE_THRESHOLD) {
                y -= 1;
            }
        }
    }
}

void *parallelSolution(void *args) {
    printf("Thread has started \n");
    
    HeatProblemArgs heatProblemArgs = *(HeatProblemArgs*) args;
    int endRow=heatProblemArgs.endRows;
    int endCol=heatProblemArgs.endCols;
    int startRows=heatProblemArgs.startRows;
    int startCols=heatProblemArgs.startCols;
    int colCounter = (startCols == 0) ? startCols+1 : startCols;
    for (int i = startRows+1; i<endRow; i++) {
        for (int j = colCounter; j<endCol; j++) {
            
            pthread_mutex_lock(&mutex);
            int newTemp = ceil(0.2 * (a[i][j] + a[i][j-1] + a[i-1][j] + a[i][j+1] + a[i+1][j]));
            int oldTemp = a[i][j];
            a[i][j] = newTemp;
//            pthread_mutex_unlock(&mutex);
            
            if (abs(oldTemp - newTemp) > TEMPRATURE_DIFFERENCE_THRESHOLD) {
                j -= 1;
            }
            
        }
        pthread_mutex_unlock(&mutex);
    }
    printf("Thread has finished \n");
    return NULL;
}

static int executeParallelSolution() {
    clock_t parallelStart = clock();
    pthread_t threadIds[2];
    HeatProblemArgs thread1Args;
    HeatProblemArgs thread2Args;
    
    thread1Args.startRows = 0;
    thread1Args.startCols = 0;
    thread1Args.endRows = rows;
    
    thread2Args.startRows = 0;
    thread2Args.endRows = rows;
    thread2Args.endCols = cols;
    
    // Columns are even
    if (cols % 2 == 0) {
        thread1Args.endCols = cols/2;
        thread2Args.startCols = (cols/2);
    } else {
        thread1Args.endCols = floor(cols/2);
        thread2Args.startCols = (floor(cols/2));
    }
    
    int thread1 = pthread_create(&threadIds[0], NULL, parallelSolution, (void*)&thread1Args);
    int thread2 = pthread_create(&threadIds[1], NULL, parallelSolution, (void*)&thread2Args);
    
    if (thread1 | thread2) {
        printf("ERROR while creating threads! \n");
        return -1;
    }
    
    for (int i = 0; i < 2; i++) {
        pthread_join(threadIds[i], NULL);
    }
    
    clock_t parallelEnd = clock();
    double parallelTimeSpent = ((double) parallelEnd - parallelStart)/CLOCKS_PER_SEC;
    print2DArray(rows, cols, a);
    printf("Time Spent for parallized solution is: %f seconds \n", parallelTimeSpent);
    return 0;
}

static int executeSequentialSolution() {
    clock_t sequentialStart = clock();
    sequentialSolution(rows, cols, a);
    print2DArray(rows, cols, a);
    clock_t sequentialEnd = clock();
    double sequentialTimeSpent = ((double) sequentialEnd - sequentialStart)/CLOCKS_PER_SEC;
    printf("Time Spent for sequential solution is: %f seconds \n", sequentialTimeSpent);
    return 0;
}

int main()
{
    int t1 = 100;
    int t2 = 200;
    int t3 = 300;
    int t4 = 400;
    
    
    for (int i = 0; i < rows; i++) {
        a[i][0] = t4;
        a[i][cols-1] = t2;
    }
    for (int j = 0; j < cols; j++) {
        a[0][j] = t1;
        a[rows-1][j] = t3;
    }
    for (int i = 1; i<rows-1; i++) {
        for (int j=1; j<cols-1; j++) {
            a[i][j] = 0;
        }
    }
    
    return executeSequentialSolution();
    
    
//    return executeParallelSolution();
    
    return 0;
}
