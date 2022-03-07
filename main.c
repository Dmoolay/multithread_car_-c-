#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

//==================================================================

#define N 5
#define FIN_PROB 0.1
#define MIN_INTER_ARRIVAL_IN_NS 8000000
#define MAX_INTER_ARRIVAL_IN_NS 9000000
#define INTER_MOVES_IN_NS 100000
#define SIM_TIME 2
#define SCRINSHOTS 10
#define MAX_CAR ((SIM_TIME*1000000000)/MIN_INTER_ARRIVAL_IN_NS)*4

//===================================================================

pthread_t cararray[N][N]; // array that represent the matrix
pthread_mutex_t Movelock[N][N]; // array for mutex for the cararray
pthread_mutex_t lockCountUp;				//mutex for increment counter
pthread_t gen[4];  //array of generators threads
pthread_t prnscr[1];
pthread_t carss[MAX_CAR];
pthread_mutex_t pthlock[MAX_CAR];
int count = 0;   // counter for cars.

//===================================================================

//func declaration:
void * Generate();
_Noreturn void Car(pthread_t ID);
_Noreturn void *enter_Car(void * stationNum);
_Noreturn void * print_screen();
void find_id(pthread_t ID);
void Free();
void handle_error_x(int x, char* msg);

//===================================================================

int main(){
    //init
    int check;
    srand(time(NULL)); // for random seed
    for (int i = 0; i < 4; ++i) {gen[i] = -1;}
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            if ((i==0) || (i == N-1) || (j==0) || (j == N-1)){cararray[i][j] = -1;}
            else{cararray[i][j] = 11111;}
            check = pthread_mutex_init(&(Movelock[i][j]), NULL);
            if (check != 0 ){ handle_error_x(check, "pthread_mutex_init");}
        }
    }
    for (int i = 0; i < MAX_CAR; ++i){
        check = pthread_mutex_init(&(pthlock[i]), NULL);
        if (check != 0 ){ handle_error_x(check, "pthread_mutex_init");}
    }
    check = pthread_mutex_init(&(lockCountUp), NULL);
    if (check != 0 ){ handle_error_x(check, "pthread_mutex_init");}

    int Generator_idx[4]; // save the generators index
    //creating the 4 generators threads
    for (int i = 0; i < 4; i++) {
        Generator_idx[i] = i;
        check = pthread_create(gen +i , NULL, (void*)&Generate, (void*)(Generator_idx+i));
        if (check != 0 ){ handle_error_x(check, "pthread_create");}
    }
    //create the print screen thread(prints SCRINSHOTS times in the simulation)
    check = pthread_create(prnscr, NULL, (void*)&print_screen, NULL);
    if (check != 0 ){ handle_error_x(check, "pthread_create");}
    //waits until the end of the simulation then free all memory and exit all the threads.
    sleep(SIM_TIME);
    Free();
    return 0;
}

_Noreturn void * Generate(void *number){
    //There are 4 generators of cars
    //the function starts new car thread every random time.
    double random;						//get the random time
    int stationNum = *((int *)number);
    stationNum++;
    int check;
    while (1){
        random = (rand() % (MAX_INTER_ARRIVAL_IN_NS - MIN_INTER_ARRIVAL_IN_NS)) + MIN_INTER_ARRIVAL_IN_NS; //random time in range
        usleep((double)random / (double)1000); // sleep random
        check = pthread_mutex_lock(&lockCountUp);
        if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock_count");}
        check = pthread_mutex_lock((&pthlock[count]));
        if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}

        check = pthread_create(carss + count, NULL, (void*)&enter_Car, (void *)(&stationNum));
        if (check != 0 ){ handle_error_x(check, "pthread_create");}

        pthread_mutex_unlock((&pthlock[count]));
        count++;
        pthread_mutex_unlock(&lockCountUp);
    }
}

_Noreturn void * enter_Car(void * stationNum1){
    int flagCar = 0, check;
    int stationNum = *((int *)stationNum1);
    while (1){
        if (flagCar == 0){
            //random = rand() % INTER_MOVES_IN_NS;
            //printf("%f\n", random);
            usleep(INTER_MOVES_IN_NS / 1000);
            //station number 1
            if (stationNum == 1){
                if ((cararray[0][N - 1] == -1) && (cararray[1][N - 1] == -1)){  // check if can generate a car
                    // lock the cell and the one before it
                    check = pthread_mutex_lock((&Movelock[1][N - 1]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                    check = pthread_mutex_lock((&Movelock[0][N - 1]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}

                    cararray[0][N - 1] = pthread_self();

                    pthread_mutex_unlock((&Movelock[0][N - 1]));
                    pthread_mutex_unlock((&Movelock[1][N - 1]));

                    flagCar = 1;
                }
            }
            //station number 2
            if (stationNum == 2){
                if ((cararray[0][0] == -1) && (cararray[0][1] == -1)){  // check if can generate a car
                    // lock the cell and the one before it
                    check = pthread_mutex_lock((&Movelock[0][1]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                    check = pthread_mutex_lock((&Movelock[0][0]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}

                    cararray[0][0] = pthread_self();

                    pthread_mutex_unlock((&Movelock[0][0]));
                    pthread_mutex_unlock((&Movelock[0][1]));

                    flagCar = 1;
                }
            }
            //station number 3
            if (stationNum == 3){
                if ((cararray[N - 2][0] == -1) && (cararray[N - 1][0] == -1)){  // check if can generate a car
                    // lock the cell and the one before it
                    check = pthread_mutex_lock((&Movelock[N - 2][0]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                    check = pthread_mutex_lock((&Movelock[N - 1][0]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}

                    cararray[N - 1][0] = pthread_self();

                    pthread_mutex_unlock((&Movelock[N - 1][0]));
                    pthread_mutex_unlock((&Movelock[N - 2][0]));

                    flagCar = 1;
                }
            }
            //station number 4
            if (stationNum == 4){
                if ((cararray[N - 1][N - 1] == -1) && (cararray[N - 1][N - 2] == -1)){  // check if can generate a car
                    // lock the cell and the one before it
                    check = pthread_mutex_lock((&Movelock[N - 1][N - 2]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                    check = pthread_mutex_lock((&Movelock[N - 1][N - 1]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}

                    cararray[N - 1][N - 1] = pthread_self();

                    pthread_mutex_unlock((&Movelock[N - 1][N - 1]));
                    pthread_mutex_unlock((&Movelock[N - 1][N - 2]));

                    flagCar = 1;
                }
            }
        }
        else{Car(pthread_self());}
    }
}

_Noreturn void Car(pthread_t ID){
    //a car function, there is thread that runs this function, we call this func only after the car enter the square.
    int flag=0,flagb=0, check;
    double random;
    int i = 0,j = 0;
    while (1){
        usleep(INTER_MOVES_IN_NS / 1000);
        // find where is the car
        for (i = 0; i < N; i++){
            for (j = 0; j < N; j++){
                if (cararray[i][j] == ID){
                    flagb = 1;
                    break;
                }
            }
            if(flagb){break;}
        }
        flagb = 0;
        //move left
        if ((i==0) && (j!=0)){
            if ((j == N-1) && (flag == 1)){
                random = ((rand() % 10000) / (double)10000);
                //by the probabilty need to exit
                if (random < (double)FIN_PROB) {
                    check = pthread_mutex_lock((&Movelock[i][j]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                    cararray[i][j] = -1;
                    pthread_mutex_unlock((&Movelock[i][j]));
                    find_id(ID);
                    pthread_exit(NULL);
                }
                else{ // move if next cell is free
                    if (cararray[i][j-1] == -1){ // next cell free
                        check = pthread_mutex_lock((&Movelock[i][j]));
                        if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                        check = pthread_mutex_lock((&Movelock[i][j-1]));
                        if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}

                        cararray[i][j] = -1;
                        cararray[i][j-1] = ID;
                        flag = 1;

                        pthread_mutex_unlock((&Movelock[i][j - 1]));
                        pthread_mutex_unlock((&Movelock[i][j]));
                    }
                }
            }
            else{   // move if next cell is free
                if (cararray[i][j - 1] == -1){ // next cell free
                    check = pthread_mutex_lock((&Movelock[i][j]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                    check = pthread_mutex_lock((&Movelock[i][j - 1]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}

                    cararray[i][j] = -1;
                    cararray[i][j - 1] = ID;
                    flag = 1;

                    pthread_mutex_unlock((&Movelock[i][j - 1]));
                    pthread_mutex_unlock((&Movelock[i][j]));
                }
            }
        }
            //move down
        else if ((j == 0) && (i != N-1)){
            if ((i == 0) && (flag == 1)){
                random = ((rand() % 10000) / (double)10000);
                if (random < (double)FIN_PROB) {  //by the probabilty need to exit
                    check = pthread_mutex_lock((&Movelock[i][j]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                    cararray[i][j] = -1;
                    pthread_mutex_unlock((&Movelock[i][j]));
                    find_id(ID);
                    pthread_exit(NULL);
                }
                else{ // move if next cell is free
                    if (cararray[i+1][j] == -1){ // next cell free
                        check = pthread_mutex_lock((&Movelock[i][j]));
                        if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                        check = pthread_mutex_lock((&Movelock[i+1][j]));
                        if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}

                        cararray[i][j] = -1;
                        cararray[i+1][j] = ID;
                        flag = 1;

                        pthread_mutex_unlock((&Movelock[i+1][j]));
                        pthread_mutex_unlock((&Movelock[i][j]));
                    }
                }
            }
            else{   // move if next cell is free
                if (cararray[i+1][j] == -1){ // next cell free
                    check = pthread_mutex_lock((&Movelock[i][j]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                    check = pthread_mutex_lock((&Movelock[i+1][j]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}

                    cararray[i][j] = -1;
                    cararray[i+1][j] = ID;
                    flag = 1;
                    pthread_mutex_unlock((&Movelock[i+1][j]));
                    pthread_mutex_unlock((&Movelock[i][j]));
                }
            }
        }
            //move right
        else if ((i == N-1) && (j != N - 1)){
            if ((j == 0) && (flag == 1)){
                random = ((rand() % 10000) / (double)10000);
                if (random < (double)FIN_PROB){ //by the probabilty need to exit
                    check = pthread_mutex_lock((&Movelock[i][j]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                    cararray[i][j] = -1;
                    pthread_mutex_unlock((&Movelock[i][j]));
                    find_id(ID);
                    pthread_exit(NULL);
                }
                else{ // move if next cell is free
                    if (cararray[i][j + 1] == -1){ // next cell free
                        check = pthread_mutex_lock((&Movelock[i][j]));
                        if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                        check = pthread_mutex_lock((&Movelock[i][j + 1]));
                        if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}

                        cararray[i][j] = -1;
                        cararray[i][j + 1] = ID;
                        flag = 1;

                        pthread_mutex_unlock((&Movelock[i][j + 1]));
                        pthread_mutex_unlock((&Movelock[i][j]));
                    }
                }
            }
            else{   // move if next cell is free
                if (cararray[i][j + 1] == -1){ // next cell free
                    check = pthread_mutex_lock((&Movelock[i][j]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                    check = pthread_mutex_lock((&Movelock[i][j + 1]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}

                    cararray[i][j] = -1;
                    cararray[i][j + 1] = ID;
                    flag = 1;
                    pthread_mutex_unlock((&Movelock[i][j + 1]));
                    pthread_mutex_unlock((&Movelock[i][j]));
                }
            }
        }
            //move up
        else if ((j == N-1) && (i != 0)){
            if ((i == N - 1) && (flag == 1)){
                random = ((rand() % 10000) / (double)10000);
                if (random < (double)FIN_PROB){ //by the probabilty need to exit
                    check = pthread_mutex_lock((&Movelock[i][j]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                    cararray[i][j] = -1;
                    pthread_mutex_unlock((&Movelock[i][j]));
                    find_id(ID);
                    pthread_exit(NULL);
                }
                else{ // move if next cell is free
                    if (cararray[i-1][j] == -1){ // next cell free
                        check = pthread_mutex_lock((&Movelock[i][j]));
                        if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                        check = pthread_mutex_lock((&Movelock[i - 1][j]));
                        if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}

                        cararray[i][j] = -1;
                        cararray[i - 1][j] = ID;
                        flag = 1;

                        pthread_mutex_unlock((&Movelock[i - 1][j]));
                        pthread_mutex_unlock((&Movelock[i][j]));
                    }
                }
            }
            else{   // move if next cell is free
                if (cararray[i - 1][j] == -1){ // next cell free
                    check = pthread_mutex_lock((&Movelock[i][j]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}
                    check = pthread_mutex_lock((&Movelock[i - 1][j]));
                    if (check != 0 ){ handle_error_x(check, "pthread_mutex_lock");}

                    cararray[i][j] = -1;
                    cararray[i - 1][j] = ID;
                    flag = 1;
                    pthread_mutex_unlock((&Movelock[i - 1][j]));
                    pthread_mutex_unlock((&Movelock[i][j]));
                }
            }
        }
    }
}

_Noreturn void * print_screen(){
    double sss = (SIM_TIME / (double)(SCRINSHOTS +1) * 1000000);
    for (int m = 0; m < SCRINSHOTS; ++m){
        usleep(sss); //sleep
        for (int i = 0; i < N; i++){
            for (int j = 0; j < N; j++){
                if ((i == 0) || (i == N-1)){
                    if (cararray[i][j] == -1){printf(" ");}
                    else{printf("*");}
                }
                else{
                    if ((j == 0) || (j == N-1)){
                        if (cararray[i][j] == -1){printf(" ");}
                        else{printf("*");}
                    }
                    else{printf("@");}
                }
            }
            printf("\n");
        }
        printf("\n");
        printf("\n");
    }
}

void Free(){ // exit all the thread and destroy all mutex
    pthread_cancel(&prnscr);
    for (int i = 0; i < count+1; ++i){
        if (carss[i] != -1){pthread_cancel(carss+i);}
    }
    for (int i = 0; i < 4; ++i){
        if (gen[i] != -1){pthread_cancel(gen+i);}
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {pthread_mutex_destroy(&(Movelock[i][j]));}
    }
    for (int i = 0; i < MAX_CAR; ++i) {pthread_mutex_destroy(&(pthlock[i]));}
    pthread_mutex_destroy(&(lockCountUp));
}

void find_id(pthread_t ID){ //find car in array and mark when exit
    for (int i = 0; i < count+1; ++i){
        if (carss[i] == ID){
            pthread_mutex_lock((&pthlock[i]));
            carss[i] = -1;
            pthread_mutex_unlock((&pthlock[i]));
            break;
        }
    }
}

void handle_error_x(int x, char* msg){
    errno = x;
    perror(msg);
    Free();
    exit(EXIT_FAILURE);
}
