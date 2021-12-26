
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define MAX_STACK 20
#define MAX_CITY 10
#define MAX_LENGTH 10


struct node {
  char city[MAX_LENGTH];
  int id;
  int hour;
  int min;
  int price;
  struct node* nextHead;
  struct node* next;
};
typedef struct node node;


typedef struct{
  int numVertices;
  node* listHead;
}Graph;

struct stack{
	int top;
	node* elements[MAX_STACK];
};
typedef struct stack stack;


struct flight{
	char route[250];
	int hour;
	int min;
	int cost;
	int stops;
};

typedef struct flight flight;


node* createNode(char city[MAX_LENGTH], int hour, int min, int price);
node* createHead(int size ,char cities [size][MAX_LENGTH]);
Graph* createGraph(int size ,char cities [size][MAX_LENGTH], char* fileName);

void addConnection(struct Graph* graph, char* source,node* destinaitonNode);
void printGraph(Graph* graph) ;
node* searchNode(Graph* graph,char* source);
void InsertNode(node* sourceNode,node* newNode);

int DFS(Graph* g, node* sourceHead, char destination[MAX_LENGTH], int* i, int k, node* path[MAX_CITY], int visited[MAX_CITY], int* found, FILE* fp, int* n, flight flights[*n] );
void printresult(int size , flight flights[size], int order);

/**/
void mappingFile(char * filename, char cities[MAX_CITY][MAX_LENGTH], int* size );
int addIfUnique(int n, char  cities[n][MAX_LENGTH], char city[MAX_LENGTH]);
void userInterface(Graph* g);
char* lowercase(char* s);

void merge(flight arr[],int start, int mid, int end, int order);
void sort(flight arr[], int start, int end, int order ); 
int main() {
	int numberofCities;
	char cities[MAX_CITY][MAX_LENGTH];
	mappingFile("sample.txt",cities, &numberofCities);
   	struct Graph* graph = createGraph(numberofCities,cities, "sample.txt");
   	printGraph(graph);
  	while(1){
  		userInterface(graph);
	}
   	free(graph);
  return 0;
}
void userInterface(Graph* g){
	
	/*These variables are used to read data from files*/
	char source[MAX_LENGTH], destination[MAX_LENGTH], sourceCache[MAX_LENGTH], destinationCache[MAX_LENGTH], start[3];
	char line[256], line2[256];
	int k,  found=0, i=0, j=0, end=0, m, order; 
	/*
	k     -> maximum number of stops
	found -> if the flight information exist before in the cache
	i     -> to iterate the route of one flight (iterator for cities)
	j     -> to iterate the array of all available flights (iterator for flights)
	end	  -> Keep track of the flights that belong to same source and destination
	order -> Either to sort results by price or by duration
	*/
	int visited[g->numVertices]; /*Keep track of visited vertices of the graph for DFS*/
	node* paths[10];	/*save all cities of same flight*/
	node* sourceHead, *destinationHead; 
	flight flights[10];	/*save all flights of same source and destination*/
	FILE *fp;
	
   	printf("\n********** Welcome to the Flight Management System **************** \n");
   	printf("\nPlease enter the source: ");
   	scanf("%s",&source);
	
	printf("\nPlease Enter the destination: ");
	scanf("%s",&destination);
   	
	strcpy(source, lowercase(source));
   	strcpy(destination, lowercase(destination));
   	if(strcmp(source,destination)==0){
		printf("\nError: Departure city cannot be the same as destination\n");
		return;
	}
	/*Check if the cities exist in the system*/
	sourceHead = searchNode(g,source);
	if(sourceHead == -1){
		printf("\nUnfortunately we do not have any flights from the city %s \n", source);
		return;
	}
	destinationHead = searchNode(g,destination);
	if(destinationHead==-1){
		printf("\nUnfortunately we do not have any flights to the city %s \n", destination);
		return;
	}

	printf("\nPlease Enter maximum number of stops: ");
	scanf("%d",&k);
	
	printf("\nTo sort results by price enter 0, to sort them by duration enter 1: ");
	scanf("%d",&order);
	
	/******************************* Search in the cache **********************************/
	fp = fopen("cache.txt", "r");
	if (fp != NULL){
	
		while(fgets(line, sizeof(line),fp) && !found){
			sscanf(line, "%s %s %s\n", &start, &sourceCache, &destinationCache);
			if(strcmp(start, "***")==0){ /*A new flight*/
				if(strcmp(sourceCache,source)==0 && strcmp(destinationCache, destination)==0){
					found=1;
				}	
			}	
		}
		
		if(found){ /*Flight exists in the cache*/
			printf("\nFlights retrieved from the cache: \n");
			fgets(line2, sizeof(line2),fp);
			sscanf(line2, "%d ", &m);
			if(m<=k){
				/*Pick the flight*/		
				strcpy(flights[j].route,line); /*Read first route*/
				sscanf(line2, "%d %d %d %d\n", &(flights[j].stops), &(flights[j].hour), &(flights[j].min), &(flights[j].cost));
				j++;
			}
			
			/*Check for other routes*/
			while(fgets(line, sizeof(line),fp) && !end){
				sscanf(line, "%s ", &start);
				if(strcmp(start, "***")==0){ /*This is the begining of another flight so end the search*/
					end=1;	
				}
				else{
					
					fgets(line2, sizeof(line2),fp);
					sscanf(line2, "%d ", &m);
					if(m<=k){
						/*Pick the flight*/	
						strcpy(flights[j].route,line); /*Read first option*/
						sscanf(line2, "%d %d %d %d\n", &(flights[j].stops), &(flights[j].hour), &(flights[j].min), &(flights[j].cost));
						j++;
					}
				}	
			}
			fclose(fp);
			if(j){ /*j is number of flights found*/
				printresult(j,flights,order);
			}
			else{
				if(k==0){
					printf("\nUnfortunately, there are no direct flights. Try searching for indirect flights\n");
				}
				else{
					printf("\nNo flights with %d stops found. Please try bigger values of stops\n",k);
				}
			}
			return;
		}
	}
	/*No previous data matching in the cache so start a new search */
	fclose(fp);
	/*Set the array of visited for (DFS) to zero*/
	memset(visited, 0 , g->numVertices * sizeof(int) );
	paths[i]= sourceHead; /*The begining of the route is the source city*/
	i++; /*To iterate the route*/
	fp = fopen("cache.txt", "a");
	if (fp == NULL){
		printf("\nFile couldn't be created\n");
		return -1;
	}
	fprintf(fp,"*** %s %s\n",sourceHead->city,destination);
	printf("\n ***************** DFS start *************** \n");
	visited[sourceHead->id]=1; /*set the departure city as visited*/
	DFS(g,sourceHead,destination,&i,k, paths,visited,&found,fp, &j,flights);
	if(found){
		if(j){ /*j is number of flights found*/
			printresult(j,flights,order);
		}
		else{
			printf("No flights with %d stops found",k);
		}
	}
	else{
		printf("\nUnfortunately, there are no flights available ");
		 
	}
   	fclose(fp);
}


node* createNode(char city[MAX_LENGTH], int hour, int min, int price) {
  node* newCity = malloc(sizeof(node));
  strcpy(newCity->city, city);
  newCity->hour = hour;
  newCity->min = min;
  newCity->price = price;
  newCity->next = NULL;
  newCity->nextHead = NULL;
  return newCity;
}

Graph* createGraph(int size ,char cities [size][MAX_LENGTH], char* fileName) {
  Graph* graph = malloc(sizeof(Graph));
  graph->numVertices = size;
  /*Creating the list of heads*/
  graph->listHead = createHead(size, cities);
  /*Adding the flights*/
  char line[256];
  char source[MAX_LENGTH];
  node* NewNode, *NewNode2, * sourceNode;
  FILE *fp;
  fp = fopen(fileName, "r");
  if (fp == NULL){
	printf("The file does not exist");
	return -1;
  }
  while(fgets(line, sizeof(line),fp)){
	NewNode  = malloc(sizeof(node));
	NewNode2 = malloc(sizeof(node));
	NewNode->next=NULL; NewNode->nextHead=NULL;
	NewNode2->next=NULL; NewNode2->nextHead=NULL;
	sscanf(line, "%s %s %d %d %d\n", &source, &(NewNode->city), &(NewNode->hour), &(NewNode->min), &(NewNode->price));
	strcpy(source, lowercase(source));
	strcpy(NewNode->city,lowercase(NewNode->city));
	/*Add edge from source to destination*/
	sourceNode = searchNode(graph,source); 
	if(sourceNode== -1){
		printf("\nError: node %s not found\n", source);
		return -1;
	}
	NewNode2->id = sourceNode->id;
	InsertNode(sourceNode,NewNode);
	/*Add edge from destination to source*/
	sourceNode = searchNode(graph,NewNode->city);
	if(sourceNode== -1){
		printf("\nError: node %s not found\n", NewNode->city);
		return -1;
	}
	NewNode->id = sourceNode->id;
	strcpy(NewNode2->city, source);
	NewNode2->hour =NewNode->hour;
	NewNode2->min =NewNode->min;
	NewNode2->price = NewNode->price;
	InsertNode(sourceNode,NewNode2);	
			
}
	
	fclose(fp);
  return graph;
}

node* createHead(int size ,char cities [size][MAX_LENGTH]){
	int i, id=0;
	node* newHead = malloc(sizeof(node));
	strcpy(newHead->city,lowercase(cities[0]));
	newHead->id= id;
	id++;
	newHead->nextHead= NULL;
	newHead->next = NULL;
	newHead->hour =0;
	newHead->min=0;
	newHead->price=0;
	node* q, *p = newHead;
	for(i=1;i<size;i++){
		q = malloc(sizeof(node));
		strcpy(q->city, lowercase(cities[i]));
		q->id = id;
		q->next= NULL;
		q->nextHead=NULL;
		q->hour= 0;
		q->min=0;
		q->price=0;
		p->nextHead = q;
		p =q;
		id++;
	}
	return newHead;
}

node* searchNode(Graph* graph,char* source){
	node* p = graph->listHead;
	if(p == NULL){
		return -1;	
	}
	while( strcmp(p->city,source) != 0 && p->nextHead != NULL ){
		p = p->nextHead;
	}
	if(strcmp(p->city,source) == 0){
		return p;
	}
	else{
		return -1;
	}
	
}

void InsertNode(node* sourceNode,node* newNode){
	node* p;
	if(sourceNode->next == NULL){
		sourceNode->next = newNode;
		return;
	}
	p = sourceNode->next;
	while(p->next !=NULL){
		p = p-> next;
	}
	p->next = newNode;
	return;
	
}

void printGraph(Graph* graph) {
  node* tempHead = graph->listHead;
  if(tempHead == NULL){
  	printf("\nError: The graph is empty\n");
  	return;
  }
  printf("\nPrinting the graph: \n");
  node* tempNode;
  while(tempHead!= NULL){
    printf("%s", tempHead->city);
	tempNode=tempHead->next;
    while(tempNode!= NULL){
    	printf(" -> %s id= %d (%d,%d,%d)", tempNode->city, tempNode->id, tempNode->hour, tempNode->min, tempNode->price);
    	tempNode = tempNode->next;
	}
    printf("\n|\n");
    tempHead = tempHead->nextHead;
  }
}


void mappingFile(char * filename, char cities[10][MAX_LENGTH], int* size ){
	
	int j,i=0;
	char line[256];
	char source[MAX_LENGTH], destination[MAX_LENGTH];
	FILE *fp;
	fp = fopen(filename, "r");
	if (fp == NULL){
		printf("The file does not exist");
		return -1;
	}
	while(fgets(line, sizeof(line),fp)){
		sscanf(line, "%s %s ", &source, &destination);
		if(addIfUnique(i,cities, source)){
			i++;
		}
		if(addIfUnique(i, cities ,destination)){
			i++;
		}		
	}
	
	*size= i;
	fclose(fp);
	return cities;
}



int addIfUnique(int n, char  cities[n][MAX_LENGTH], char city[MAX_LENGTH]){
	if(n==0){
		strcpy (cities[0], city); 
		return 1;            
	}
	int j=0, unique=1;
	while(j<n && unique){
		
		if(strcmp(cities[j], city)==0){
			unique = 0;
		}	
		j++;
	}
	if(unique){
		strcpy (cities[j], city);
	}
	return unique;
}


char* lowercase(char* s) {
  char *p;
  for(p=s; *p; p++) *p=tolower(*p);
  return s;
}


int DFS(Graph* g, node* sourceHead, char destination[MAX_LENGTH], int* i, int k ,node* path[MAX_CITY], int visited[MAX_CITY], int *found , FILE* fp, int * n , flight flights[*n]){
	
	int j, hour= 0, min=0, cost=0;
	char route[250];
	if(strcmp(sourceHead->city, destination)==0){
		/*Add route to file*/	
		*found=1;
		fprintf(fp, "%s-> ",path[0]->city);
		strcpy(route, path[0]->city);
		strcat(route, "->");
		for(j=1;j<*i; j++){
			fprintf(fp, "%s-> ",path[j]->city);
			strcat(route, path[j]->city);
			strcat(route, "->");
			cost = cost + path[j]->price;
			hour = hour + path[j]->hour;
			min = min + path[j]->min;
		}
		/*Computing hour:min*/
		hour = hour + (min/60) + (*i-2) ;
		min = min % 60;
		fprintf(fp, "\n");
		fprintf(fp,"%d %d %d %d\n",*i-2,hour,min,cost);
		if(*i-2 <= k){
			strcpy(flights[*n].route,route);
			flights[*n].stops = *i-2;
			flights[*n].hour = hour;
			flights[*n].min=min;
			flights[*n].cost = cost;
			*n = *n + 1; /*iterate the flights table*/
		}
		
		*i = *i - 1;
		
		return 0;
	}
	node* neighbour;
	neighbour = sourceHead->next; 
    /*Check all neighbours*/
	while(neighbour != NULL){
		if(visited[neighbour->id] != 1){
				visited[neighbour->id] = 1;
				path[*i]= neighbour;
				*i= *i + 1;
				DFS(g, searchNode(g,neighbour->city), destination ,i,k,path,visited,found,fp, n , flights);
                if(found){
					/*If found do not move to other nodes linked after it*/
                	neighbour = path[*i];
				}
				visited[neighbour->id] = 0;	
		}	
		neighbour = neighbour->next;
		
	} 
	*i = *i -1;
}

void printresult(int size , flight flights[size], int order){
	int i;	
	sort(flights, 0, size-1, order );
	for(i=0;i<size;i++){
		printf("\nFlight no: %d\n",i+1);
		printf("%s\n",flights[i].route);
		printf("number of stops: %d . Total duration: %d hour %d min \nTotal cost: %d\n ",flights[i].stops, flights[i].hour, flights[i].min, flights[i].cost);
		
		
	}
}

void sort(flight arr[], int start, int end, int order ){
	if (start<end){
		int mid= start + (end -start)/2;
		sort(arr, start, mid,order); /*split left array*/
		sort(arr, mid+1, end,order); /*split right array*/
		merge(arr,start, mid, end, order); /*merge the two arrays*/
	}
	
}

void merge(flight arr[],int start, int mid, int end, int order){
	int i,j,k;
	int size1= mid -start +1; 
	int size2= end - mid;
	flight leftArr[size1], rightArr[size2];
	/*Copy both arrays*/
	for(i=0;i<size1;i++){
		leftArr[i]= arr[start+i];
	}
	for(j=0; j<size2; j++){
		rightArr[j]= arr[mid+j+1];
	}
	
	
	i=0;j=0;k=start;
	if( order == 0){ /*Order by price*/	
		while(i<size1 && j< size2){
			if(leftArr[i].cost < rightArr[j].cost){
			arr[k]= leftArr[i];
			i++;
			}
			else{
				arr[k]= rightArr[j];
				j++;
			}
			k++;
		}
	}
	else{
		/*Order by duration*/
		while(i<size1 && j< size2){
			if((leftArr[i].hour*60 + leftArr[i].min ) < (rightArr[j].hour * 60 + rightArr[j].min)){
			arr[k]= leftArr[i];
			i++;
			}
			else{
				arr[k]= rightArr[j];
				j++;
			}
			k++;
		}
	}

	while(i<size1){
		arr[k]=leftArr[i];
		k++;
		i++;
	}
	while(j<size2){
		arr[k]= rightArr[j];
		k++;
		j++;
	}
	return;
}
