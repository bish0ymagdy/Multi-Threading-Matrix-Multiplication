#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#define  SIZE     20
#define  M1FILE  "c_per_matrix.txt"
#define  M2FILE  "c_per_row.txt"
#define  M3FILE  "c_per_element.txt"

// variables decleration & initialization
int a_row, a_col, b_row, b_col;
int **A,  **B, **C_mat, **C_row, **C_ele;

struct pos{
    int row;
    int col;
};

// Function Prototypes
void Read(int ***mat, int  *row, int *col, char *file); // read the matrix from the given file name and put it in the given matrix pointer
void Write(int **mat, int row, int col, char *file);    // write the output matrix in the given text file
void GetInput(int argc, char *argv[]);                  // get the text file name which contain the matix
void GarbageCollector(int argc, char *argv[]);          // free the allocated space in the memory by the malloc function
void CreateMatrix(int ***mat, int  row, int col);       // allocate the necessary space for the given matrix pointer
void *MulPerMatrix();                                   // thread function mutiple the two matrices 
void Method1(int argc, char *argv[]);                   // method 1 function
void *MulPerRow(void *row);                             // thread function calculate each row in the matrix multiplication
void Method2(int argc, char *argv[]);                   // method 2 function
void *MulPerEle(void *ele);                             // thread function calculate each element in the matrix multiplication
void Method3(int argc, char *argv[]);                   // method 3 function
void SelectMethod(int argc, char *argv[]);              // select the method of calculation

void Read(int ***mat, int  *row, int *col, char *file){
    FILE *f = fopen(file,"r");

    if (f == NULL){
        printf("Error opening the file.\n");
        exit(EXIT_FAILURE);
    }

    if (fscanf(f, "row=%d col=%d", row, col) != 2){
        printf("Error reading row and column sizes from file.\n");
        return;
    } 

    *mat = (int **)malloc(*row * sizeof(int *));
    if (mat == NULL) {
        printf("Error allocating memory.\n");
        free(mat);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < *row; i++) {
        (*mat)[i] = (int *)malloc(*col * sizeof(int));
        if ((*mat)[i] == NULL) {
            printf("Error allocating memory.\n");
            return;
        }
    }

    for (int i = 0; i < *row; i++) {
        for (int j = 0; j < *col; j++) {
            if (fscanf(f, "%d", &((*mat)[i][j])) != 1) {
                printf("Error reading from file.\n");
                return;
            }
        }
    }

    fclose(f);
}

void Write(int **mat, int row, int col, char *file){
    FILE *f = fopen(file, "w");

    if (f == NULL) {
        printf("Error opening the file for writing.\n");
        exit(EXIT_FAILURE);
    }

    fprintf(f, "row=%d col=%d\n", row, col);

    int max_digits = 0;

    // determine the maximum number of digits among all elements of the matrix to make the output column alined
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            int digits = snprintf(NULL, 0, "%d", mat[i][j]);
            if (digits > max_digits) {
                max_digits = digits;
            }
        }
    }

    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            fprintf(f, "%*d\t\t", max_digits, mat[i][j]);
        }
        fprintf(f, "\n");
    }

    fclose(f);
}

void CreateMatrix(int ***mat, int  row, int col){
    *mat = (int **)malloc(row * sizeof(int *));
    if (mat == NULL) {
        printf("Error allocating memory.\n");
        return;
    }

    for (int i = 0; i < row; i++) {
        (*mat)[i] = (int *)malloc(col * sizeof(int));
        if ((*mat)[i] == NULL) {
            printf("Error allocating memory.\n");
            return;
        }

        for (int j = 0; j < col; j++) {
            (*mat)[i][j] = 0;
        }
    }
}

void GetInput(int argc, char *argv[]){

    if (argc < 3) {
        Read(&A, &a_row, &a_col,  "a.txt");
        Read(&B, &b_row, &b_col,  "b.txt");
    }
    else {
        char a[SIZE], b[SIZE];
        strcpy(a, argv[1]);
        strcpy(b, argv[2]);
        strcat(a, ".txt");
        strcat(b, ".txt");
        Read(&A, &a_row, &a_col, a);
        Read(&B, &b_row, &b_col, b);
        if(a_col != b_row){
            printf("Error in dimensions\n");
            GarbageCollector(argc, argv);
            exit(EXIT_FAILURE);
        }
    }
}

void *MulPerMatrix(){
    for (int i = 0; i < a_row; i++) {
        for (int j = 0; j < b_col; j++) {
            C_mat[i][j] = 0;
 
            for (int k = 0; k < b_row; k++) {
                C_mat[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    pthread_exit(NULL);
}

void Method1(int argc, char *argv[]){
    struct timeval start, stop;
    CreateMatrix(&C_mat, a_row, b_col);
    gettimeofday(&start , NULL);
    pthread_t M1;
    if(pthread_create(&M1, NULL, &MulPerMatrix, NULL) != 0){
        printf("Error creating thread\n");
        GarbageCollector(argc, argv);
        exit(EXIT_FAILURE);
    }
    pthread_join(M1, NULL);
    gettimeofday(&stop, NULL);
    for (int i = 0;  i < a_row; i++){
        for (int j = 0; j < b_col; j++){
            printf("%d\t", C_mat[i][j]);
        }
        printf("\n");
    }
    printf("Method 1 created 1 thread\n");
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
}

void *MulPerRow(void *row){
    int r = *((int*) row);
    for (int j = 0; j < b_col; j++) {
        C_row[r][j] = 0;

        for (int k = 0; k < b_row; k++) {
            C_row[r][j] += A[r][k] * B[k][j];
        }
    }
    pthread_exit(NULL);
}

void Method2(int argc, char *argv[]){
    struct timeval start, stop;
    CreateMatrix(&C_row, a_row, b_col);
    gettimeofday(&start , NULL);
    pthread_t M2[a_row];
    int thread_args[a_row];
    for (int i = 0; i < a_row; i++){
        thread_args[i] = i;
        if(pthread_create(&M2[i], NULL, &MulPerRow, (void *)&thread_args[i]) != 0){
            printf("Error creating thread\n");
            GarbageCollector(argc, argv);
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < a_row; i++){
        pthread_join(M2[i], NULL);
    }
    gettimeofday(&stop, NULL);
    for (int i = 0;  i < a_row; i++){
        for (int j = 0; j < b_col; j++){
            printf("%d\t", C_row[i][j]);
        }
        printf("\n");
    }
    printf("Method 2 created %d thread\n", a_row);
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
}

void *MulPerEle(void* ele){
    struct pos *element = (struct pos*) ele;
    for (int k = 0; k < b_row; k++) {
        C_ele[element -> row][element -> col] += A[element -> row][k] * B[k][element -> col];
    }

    pthread_exit(NULL);
}

void Method3(int argc, char *argv[]){
    struct timeval start, stop;
    CreateMatrix(&C_ele, a_row,  b_col);
    gettimeofday(&start , NULL);
    pthread_t M3[a_row][b_col];
    struct pos thread_args[a_row][b_col];
    for (int i = 0; i < a_row; i++){

        for (int j = 0; j < b_col; j++){
            thread_args[i][j].row = i;
            thread_args[i][j].col = j;
            if(pthread_create(&M3[i][j], NULL, &MulPerEle, (void *)&thread_args[i][j]) != 0){
                printf("Error creating thread\n");
                GarbageCollector(argc, argv);
                exit(EXIT_FAILURE);
            }
        }
    }
    gettimeofday(&stop, NULL);
    for (int i = 0;  i < a_row; i++){
        for (int j = 0; j < b_col; j++){
            printf("%d\t", C_ele[i][j]);
        }
        printf("\n");
    }
    printf("Method 2 created %d thread\n", a_row * b_col);
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
}

void GarbageCollector(int argc, char *argv[]){
    for(int i = 0; i < a_row; i++){
        free(A[i]);
        if (argc == 4){
            if (strcmp(argv[3], "1") == 0){
                free(C_mat[i]);
            }
            else if (strcmp(argv[3], "2") == 0){
                free(C_row[i]);
            }
            else if (strcmp(argv[3], "3") == 0){
                free(C_ele[i]);
            }
        }
        else if (argc == 2){
            if (strcmp(argv[1], "1") == 0){
                free(C_mat[i]);
            }
            else if (strcmp(argv[1], "2") == 0){
                free(C_row[i]);
            }
            else if (strcmp(argv[1], "3") == 0){
                free(C_ele[i]);
            }
        }
        else {
            free(C_mat[i]);
            free(C_row[i]);
            free(C_ele[i]);
        } 
    }
    

    for (int i = 0; i < b_row; i++){
        free(B[i]);
    }

    free(A);
    free(B);
    if (argc == 4){
        if (strcmp(argv[3], "1") == 0){
            free(C_mat);
        }
        else if (strcmp(argv[3], "2") == 0){
            free(C_row);
        }
        else if (strcmp(argv[3], "3") == 0){
            free(C_ele);
        }
    }
    else if (argc == 2){
        if (strcmp(argv[1], "1") == 0){
            free(C_mat);
        }
        else if (strcmp(argv[1], "2") == 0){
            free(C_row);
        }
        else if (strcmp(argv[1], "3") == 0){
            free(C_ele);
        }
    }
    else{
        free(C_mat);
        free(C_row);
        free(C_ele);
    }    
}

void SelectMethod(int argc, char *argv[]){
    if (argc == 4){
        if (strcmp(argv[3], "1") == 0){
            GetInput(argc, argv);
            Method1(argc, argv);
            Write(C_mat, a_row, b_col, M1FILE);
        }
        else if (strcmp(argv[3], "2") == 0){
            GetInput(argc, argv);
            Method2(argc, argv);
            Write(C_row, a_row, b_col, M2FILE);
        }
        else if (strcmp(argv[3], "3") == 0){
            GetInput(argc, argv);
            Method3(argc, argv);
            Write(C_ele, a_row, b_col, M3FILE);
        }
        else {
            printf("wrong argument\n");
            GarbageCollector(argc, argv);
            exit(EXIT_FAILURE);
        }
    }
    else if (argc == 2){
        if (strcmp(argv[1], "1") == 0){
            GetInput(argc, argv);
            Method1(argc, argv);
            Write(C_mat, a_row, b_col, M1FILE);
        }
        else if (strcmp(argv[1], "2") == 0){
            GetInput(argc, argv);
            Method2(argc, argv);
            Write(C_row, a_row, b_col, M2FILE);
        }
        else if (strcmp(argv[1], "3") == 0){
            GetInput(argc, argv);
            Method3(argc, argv);
            Write(C_ele, a_row, b_col, M3FILE);
        }
        else {
            printf("wrong argument\n");
            GarbageCollector(argc, argv);
            exit(EXIT_FAILURE);
        }
    }
    else if (argc > 4){
        printf("wrong argument\n");
        GarbageCollector(argc, argv);
        exit(EXIT_FAILURE);
    }
    else{
        GetInput(argc, argv);
        Method1(argc, argv);
        Method2(argc, argv);
        Method3(argc, argv);
        Write(C_mat, a_row, b_col, M1FILE);
        Write(C_row, a_row, b_col, M2FILE);
        Write(C_ele, a_row, b_col, M3FILE);
    }
}

int main(int argc, char *argv[]){
    SelectMethod(argc, argv);
    GarbageCollector(argc, argv);
    return 0;
}