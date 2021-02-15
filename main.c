/* 
Authors : Koray Emre SENEL  - 150117037
          Ahmet TURGUT      - 150117046
          Mehmet Etka UZUN  - 150118504
*/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <semaphore.h>
#include <pthread.h>

//input arguments
int publisherType ;
int publisherCount ;
int packagerCount ;
int bookCount ;
int packageSize ;
int bufferSize ;

int *packagedBookCount ;//array of book numbers in a buffer
int *bufferBookCount;//array of book numbers in a buffer
int *bufferSizeArr;//array of buffer sizes 

typedef struct book 
{

    int bookID; //ID of book
    int type; //Type of publisher that creadted 

}book;
typedef struct buffer
{

    int type; //Type of buffer
    struct book *book; //Book that buffer contains
    struct buffer *next;  

}buffer;

typedef struct buffer* bufferPtr ;
typedef struct buffer** bufferPtrPtr ;
bufferPtrPtr bufferHeaderArr; //global buffer headers 

typedef struct publisher 
{

    int publisherID; //Publisher id 
    int publishedBookCount; //The count of published books 
    int type; //Publisher type

}publisher;



typedef struct packager
{
    int id ; //Packager id
    struct book **bookArr; //Package of packager
    int num; //How many books package has 

}packager;

//Mutexes 
pthread_mutex_t *pubMutex;//Array for different buffer types
pthread_mutex_t pacMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t exitMutex = PTHREAD_MUTEX_INITIALIZER;

void insert(bufferPtrPtr head,int type,int id){ 
    
    //Creating the buffer that will be inserted to buffers linked list
    bufferPtr insertingBuffer=malloc(sizeof(buffer));
    insertingBuffer->type = type;
    insertingBuffer->book = malloc(sizeof(book));
    insertingBuffer->book->type = type;
    insertingBuffer->book ->bookID = bufferBookCount[type]+1;
    insertingBuffer->next = NULL;
    bufferBookCount[type]++;
    //Count for how many books this buffer has
    int koray=0;
    if(*head == NULL){
        *head=malloc(sizeof(buffer*));
        *head = insertingBuffer;
        printf("Publisher %d of type %d     Book%d_%d  is published and put into the buffer %d.\n",id,type+1,(*head)->book->type,(*head)->book->bookID,type+1);
        
        return ;
    }
    else{
        bufferPtr temp = *head;
        koray++;
        //Buffers size check to double if it is filled
        if(koray == bufferSizeArr[type]){
            printf("Publisher %d of type %d     Buffer is full. Resizing the buffer. %d \n",id,type+1,bufferSizeArr[type]);
            bufferSizeArr[type] = 2*(bufferSizeArr[type]);
        }
        while(temp->next != NULL){
            koray++;
            //Buffers size check to double if it is filled
            if(koray == bufferSizeArr[type]){
                printf("Publisher %d of type %d     Buffer is full. Resizing the buffer. %d \n",id,type+1,bufferSizeArr[type]);
                bufferSizeArr[type] = 2*(bufferSizeArr[type]);
            }
            temp = temp->next;
        }
        temp->next = malloc(sizeof(buffer*));
        temp->next = insertingBuffer;
        printf("Publisher %d of type %d     Book%d_%d  is published and put into the buffer %d.\n",id,type+1,temp->next->book->type+1,temp->next->book->bookID,type+1);
       
        
        return ;
    }

}

void delete (bufferPtrPtr header){

    buffer *temp = *header;
     // If head node itself holds the key to be deleted 
    if (temp != NULL ) 
    { 
        
        if(temp->next != NULL){

        *header = temp->next;  

        }
        else
        {
            *header = NULL;
        }
         // Changed head 
        if(temp != NULL)
        free(temp);

    }

}
//Method that publisher threads will use.
void* bookPublishment(void *a){

    int x = 0;
    publisher *pub = malloc(sizeof(publisher*));
    pub = a;
    int i = pub->type;
    for(x = 0; x < bookCount ; x++){
        //Locking the buffer that book will be published.
        pthread_mutex_lock(&pubMutex[i]);
        insert(&bufferHeaderArr[i],i,pub->publisherID);
        //Unlocking the buffers lock.
        pthread_mutex_unlock(&pubMutex[i]);
        fflush(stdout);
        //Incrementing the publishers published book number
        pub->publishedBookCount++;
    }
    printf("Publisher %d of type %d     Finished publishing %d books.Exiting the system.\n",pub->publisherID,i+1,bookCount);
    pthread_exit(NULL);

}

void* bookPackagment(void *a){

    packager *pac = malloc(sizeof(packager));
    pac = a;
    pac->bookArr= (book**)malloc(sizeof(book*)*100);
    int c = 0;
    int randBuffer = rand() % publisherType;
    for(c = 0; c < publisherType*bookCount*publisherCount*2 ; c++){
        //Random selecting the buffer
        
        //Locking the mutexes
        pthread_mutex_lock(&pacMutex);
        pthread_mutex_lock(&pubMutex[randBuffer]);
        //Checking if the buffer is empty
        while(bufferHeaderArr[randBuffer] == NULL){
            //Checking if the buffer has any books going to publish in it
            if(bufferBookCount[randBuffer] == publisherCount*bookCount){
                int i ;
                int all = 0;
                for(i = 0 ; i < publisherType ; i++){
                    all += packagedBookCount[i];
                }
                //Checking if the publishers published all the books and they are packaged
                if(publisherType*bookCount*publisherCount == all){
                    //Locking the mutex that used for packager exit
                    pthread_mutex_lock(&exitMutex);
                    printf("Packager%d      There are no publishers left in the system. Only %d of %d number of books could be packaged.",pac->id,pac->num,packageSize);
                    //if the package is empty it will exit
                    if(pac->num == 0){
                        printf("Exiting system.\n");      
                        //unlocking every lock before exiting
                        pthread_mutex_unlock(&exitMutex);
                        pthread_mutex_unlock(&pubMutex[randBuffer]);
                        pthread_mutex_unlock(&pacMutex);
                    if(pac != NULL){
                        free(pac);
                    }
                        
                        pthread_exit(NULL);
                    }
                    //Printing the packages content before exiting
                    printf("\n The package contains :");
                    fflush(stdout);
                    int k ;
                    for(k =0 ; k < pac->num-1 ; k++){
                        printf("Book%d_%d,",pac->bookArr[k]->type+1,pac->bookArr[k]->bookID);
                    }
                    printf("Book%d_%d.      Exiting system.\n",pac->bookArr[pac->num-1]->type+1,pac->bookArr[pac->num-1]->bookID);
                    //unlocking every lock before exiting
                    pthread_mutex_unlock(&exitMutex);
                    pthread_mutex_unlock(&pubMutex[randBuffer]);
                    pthread_mutex_unlock(&pacMutex);
                    if(pac != NULL)
                        free(pac);
                    pthread_exit(NULL);
                }

            }
            //If there are still books waiting to get published randomly selecting another buffer and unlocking mutex.
            pthread_mutex_unlock(&pubMutex[randBuffer]);
            randBuffer = rand() % publisherType;
        }
    //Adding the head of the buffers head to the package
    fflush(stdout);
    pac->bookArr[pac->num]  = bufferHeaderArr[randBuffer]->book;
    //Deleting the buffer
    delete(&bufferHeaderArr[randBuffer]);
    //Incrementing the buffers packaged book count
    packagedBookCount[randBuffer]++;
    printf("Packager %d     Put Book%d_%d into the package (%d/%d).\n",pac->id,pac->bookArr[pac->num]->type+1,pac->bookArr[pac->num]->bookID,pac->num+1,packageSize);
    fflush(stdout);
    //Incrementing the packagers book count
    pac->num++;
    //Unlocking mutexes
    pthread_mutex_unlock(&pacMutex);
    pthread_mutex_unlock(&pubMutex[randBuffer]);
    //If package is full prints the content and NULLS the array.
        if(pac->num == packageSize){

            printf("Packager%d      Finished preparing one package. The package contains:\n          ",pac->id);
            fflush(stdout);
            int k = 0;
            for(k =0 ; k < packageSize ; k++){

                printf("Book%d_%d,",pac->bookArr[k]->type+1,pac->bookArr[k]->bookID);
                pac->bookArr[k] = NULL;

            }  
            printf("\n");
            pac->num = 0;
        }
    }
    pthread_mutex_lock(&exitMutex);
                    printf("Packager%d      There are no publishers left in the system. Only %d of %d number of books could be packaged.",pac->id,pac->num,packageSize);
                    //if the package is empty it will exit
                    if(pac->num == 0){
                        printf("Exiting system.\n");      
                        //unlocking every lock before exiting
                        pthread_mutex_unlock(&exitMutex);
                        pthread_mutex_unlock(&pubMutex[randBuffer]);
                        pthread_mutex_unlock(&pacMutex);
                    if(pac != NULL){
                        free(pac);
                    }
                        
                        pthread_exit(NULL);
                    }
                    //Printing the packages content before exiting
                    printf("\n The package contains :");
                    fflush(stdout);
                    int k ;
                    for(k =0 ; k < pac->num-1 ; k++){
                        printf("Book%d_%d,",pac->bookArr[k]->type+1,pac->bookArr[k]->bookID);
                    }
                    printf("Book%d_%d.      Exiting system.\n",pac->bookArr[pac->num-1]->type+1,pac->bookArr[pac->num-1]->bookID);
                    //unlocking every lock before exiting
                    pthread_mutex_unlock(&exitMutex);
                    pthread_mutex_unlock(&pubMutex[randBuffer]);
                    pthread_mutex_unlock(&pacMutex);
                    if(pac != NULL)
                        free(pac);
                    pthread_exit(NULL);

}

int main(int argc,char* argv[]){

   if(argc == 0)return 0;
	
   if(argc == 10){
        if(strcmp(argv[1],"-n") != 0) {
        printf("Invalid Argument!");
        return 0;
        }
        if(strcmp(argv[5],"-b") != 0) {
            printf("Invalid Argument!");
            return 0;
        }

        if(strcmp(argv[7],"-s") != 0) {
            printf("Invalid Argument!");
            return 0;
        }
   }else {
       printf("Invalid Argument!");
       return 0;
   }


    publisherType = atoi(argv[2]);
    publisherCount = atoi(argv[3]);
    packagerCount = atoi(argv[4]);
    bookCount = atoi(argv[6]);
    packageSize = atoi(argv[8]);
    bufferSize = atoi (argv[9]);

    int i ;
    int type ;

    //Creating buffers and filling the arrays
	buffer *buffersArrTemp[publisherType];
    int bufferSizeArrTemp[publisherType];
    int bufferBookCountTemp[publisherType];
    int packagedBookCountTemp[publisherType];
	
    for (i = 0 ; i < publisherType ; i++ ){

        bufferPtr header = NULL;
        buffersArrTemp[i] = header;
        bufferSizeArrTemp[i] = bufferSize;
        bufferBookCountTemp[i] = 0;
        packagedBookCountTemp[i] = 0;
    
    }

    bufferBookCount = bufferBookCountTemp;
    bufferSizeArr = bufferSizeArrTemp;
    bufferHeaderArr = buffersArrTemp;
    packagedBookCount = packagedBookCountTemp;
	
	//Creating publisher structs
    publisher *publisherArr [publisherType*publisherCount];
	
    for (type = 0 ; type < publisherType ; type++){    
        for (i = 0 ; i < publisherCount ; i++){

            publisherArr[type*publisherCount+i] = malloc(sizeof(publisher*));
            publisher *publisherTemp = malloc(sizeof(publisher));
            publisherTemp->publisherID = i+1;
            publisherTemp->type = type;
            publisherTemp->publishedBookCount = 0;
            publisherArr[type*publisherCount+i] = publisherTemp;
            

        }

    }   
	//Creating package structs
    packager *packagerArr[packagerCount];
    
    for (i = 0 ; i < packagerCount ; i++){

        packagerArr[i] = malloc(sizeof(packager*));
        book *tempxdArr[packageSize];
        packager *packagerTemp = malloc(sizeof(packager));
        packagerTemp->id = i+1;
        packagerTemp->bookArr = tempxdArr;
        packagerTemp->num = 0;
        packagerArr[i] = packagerTemp;

    }
	//Creating and initilazing mutexes
    pthread_mutex_t typeSemArr[publisherType];
    pubMutex = typeSemArr;

    for(i = 0 ; i<publisherType;i++){
    
        pthread_mutex_init(&pubMutex[i],NULL);
    
    }
	//Creating threads and joining 
    pthread_t pacTreads[packagerCount];
    pthread_t pubTreads[publisherType*publisherCount];

    for (i = 0 ; i < publisherType*publisherCount ; i++){
    
        pthread_create(&(pubTreads[i]), NULL, &bookPublishment ,(void*) publisherArr[i]);  
    
    }

    for (i = 0 ; i < packagerCount ; i++){
    
        pthread_create(&(pacTreads[i]), NULL, &bookPackagment ,(void*) packagerArr[i]);
    
    }
    
    for (i = 0 ; i < packagerCount ; i++){
    
        pthread_join(pacTreads[i], NULL);
    
    }

    for (i = 0 ; i < publisherType*publisherCount ; i++){
    
        pthread_join(pubTreads[i], NULL);
    
    }

}