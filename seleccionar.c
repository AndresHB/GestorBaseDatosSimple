/*
Autores:
	Carlos Arroyo Villalobos
	Andrés Hernández Bravo
	Andrés Navarro Durán
	Karina Rivera Solano

	Marzo, 2016
*/

//Librerias
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

//Definicion de tipo boolean
typedef int bool;
#define true 1
#define false 0

//Variables Globales----------------------------------------
struct ListaTabla tablasG;
char** peticion = NULL;

// Metodos str: --------------------------------------------
// Metodos para manejo de char*

//Crea una subcadena:
char* substr(char *cadena, int comienzo, int longitud) {
	if (longitud == 0) longitud = strlen(cadena)-comienzo-1;
	char* nuevo=(char*)calloc(longitud+2,sizeof(char));
	strncpy(nuevo, cadena + comienzo, longitud);
	return nuevo;
}

//Funciona como un toLower en C
void ourToLower(char *comando) {
	int iterador=0;
    while(comando[iterador]!='\0') {
        comando[iterador]=tolower(comando[iterador]);
        iterador++;
	}
}

//Retorna el peso del array de char*
int pesoCadena(char** cadena){
	int peso=0;
	if(cadena==NULL)return 0;
	while(cadena[peso]!='\0')
		peso++;
	return peso;
}

//User:-------------------------------------------------------------
//Estructuras para recorrer Users.dat
struct Bloque {
	struct Bloque * sig;
	char tipo[7];
	char columna[25];
	char datos[30];
	unsigned long int anterior;
};

struct ListaBloques{
	struct Bloque* primero;
	struct Bloque* actual;
	struct Bloque* ultimo;
};

//Constructor de la lista de bloques de datos
void initLB(struct ListaBloques * lisB){
	lisB->primero = NULL;
	lisB->actual = NULL;
	lisB->ultimo = NULL;
}

//Retorna true si la lista esta vacia
bool vacioLB(struct ListaBloques * lis){
	if(lis->primero != NULL)
		return true;
	else
		return false;
}

//Inserta bloques en la lista
void insertarLB(struct Bloque* nuevo, struct ListaBloques*  lis){
		if(!(lis->primero)){
			lis->primero=nuevo;
			nuevo->sig=NULL;
			lis->ultimo=lis->primero;
		}else {
			lis->ultimo->sig=nuevo;
			nuevo->sig=NULL;
			lis->ultimo=nuevo;
		}
}

//Destructor de la lista de bloques de datos
void destructorLB(struct ListaBloques * lisB){
	if(vacioLB(lisB)){
		lisB->actual = lisB->primero;
		while(lisB->actual->sig != NULL){
			lisB->actual = lisB->primero->sig;
			free(lisB->primero);
			lisB->primero=lisB->actual;
		}
		free(lisB->actual);
	}
}

//Catalogo:---------------------------------------------------------
//Estructuras para recorrer Catalogo.dat
struct BloqueC {
	char tipo;
	char datos[25];
	struct BloqueC* sig;
};

struct Tabla {
	char nombre[30];
	unsigned long int primer_Bloque;
	unsigned long int ultimo_Bloque;
	unsigned int numColumnas;
	struct Tabla* sig;
};

struct Nodo{
	struct Nodo * sig;
	struct ListaBloques* datos;
	struct BloqueC* columnas;
	struct Tabla* tabla;
};

struct ListaTabla{
	struct Nodo* primero;
	struct Nodo* actual;
};

//Constructor de la lista de tablas
void initLT(struct ListaTabla* list){
	list->primero=NULL;
	list->actual=NULL;
}

//Retorna true si la lista esta vacia
bool vacioLT(struct ListaTabla * lisT){
	if(lisT->primero != NULL)
		return true;
	else
		return false;
}

//Inserta nodos en la lista
void insertarLT(struct Nodo* nuevo, struct ListaTabla* lis){
		if(!(lis->primero)){
			lis->primero=nuevo;
			nuevo->sig=NULL;
		}else {
			nuevo->sig=lis->primero;
			lis->primero=nuevo;
		}
}

//Destructor
void destructorLT(struct ListaTabla * lisT){
	if(vacioLT(lisT)){
		lisT->actual = lisT->primero;
		while(lisT->actual->sig != NULL){
			lisT->actual = lisT->primero->sig;
			free(lisT->primero->columnas);
			free((lisT->primero->tabla));
			destructorLB(lisT->primero->datos);
			free(lisT->primero);
			lisT->primero=lisT->actual;
		}
		free(lisT->primero->columnas);
		free((lisT->primero->tabla));
		destructorLB(lisT->primero->datos);
		free(lisT->primero);

	}
}

//Crea un nodo
struct Nodo* crearNodo(struct Tabla* tabla){
	struct Nodo* nuevo= (struct Nodo*)calloc(1,sizeof(struct Nodo));
	nuevo->datos=(struct ListaBloques*)calloc(1,sizeof(struct ListaBloques));
	initLB(nuevo->datos);
	nuevo->sig=NULL;
	nuevo->tabla=tabla;
	nuevo->columnas=(struct BloqueC*)calloc((tabla->numColumnas)*2,sizeof(struct BloqueC));
}

//Imprime la lista
void imprimeListaT(bool parametros){
	if(vacioLT(&tablasG)){
		tablasG.actual = tablasG.primero;
		int i=0,j=0,tam=0;
		while(tablasG.actual!=NULL){
			printf("-> Tabla: %s \n",tablasG.actual->tabla->nombre);
			printf(" -------------------------------------------------------------------\n");

			tam=tablasG.actual->tabla->numColumnas;
			for(i=0,j=2; i< tam*2;i+=2,j=2){
				if(parametros){
					while(peticion[j]!='\0'){
						if(strcmp(peticion[j++],tablasG.actual->columnas[i+1].datos)==0){
							printf("\t%s",tablasG.actual->columnas[i+1].datos);
							break;
						}
					}
				}else{
					printf("\t%s",tablasG.actual->columnas[i+1].datos);
				}
			}
			tablasG.actual->datos->actual=tablasG.actual->datos->primero;

			printf("\n -------------------------------------------------------------------\n");
			i=0;
			if(parametros)tam=pesoCadena(peticion)-2;
				while(tablasG.actual->datos->actual!=NULL){
						if(i%tam==0){printf("\n"); i=0;}
						printf("\t%s",tablasG.actual->datos->actual->datos);
						tablasG.actual->datos->actual=tablasG.actual->datos->actual->sig;
						i++;
				}
			printf("\n\n <----------------------------------------------------------------->\n\n");
			tablasG.actual=tablasG.actual->sig;
		}
	}
}

//Seleccionar bloques:----------------------------------------------
//Metodos para seleccionar bloques

//Recorre la lista para cargar los datos
void cargaDatos(bool parametros){
	FILE* archivo = fopen("usuarios.dat","rb");
	unsigned long int inicio=0, i=0;
	if(archivo){
		tablasG.actual=tablasG.primero;
		while(tablasG.actual != NULL){ //lee las tablas en la lista
			inicio = tablasG.actual->tabla->ultimo_Bloque;
			if(inicio>=tablasG.actual->tabla->primer_Bloque){ //si la tabla tiene datos
				fseek(archivo,sizeof(struct Bloque)*inicio,SEEK_SET);
				struct Bloque* bloq= (struct Bloque*)calloc(1,sizeof(struct Bloque));
				fread(bloq,sizeof(struct Bloque),1,archivo);
				while(!feof(archivo)){
					if(parametros==false){ // si tiene o no parametros
						insertarLB(bloq,tablasG.actual->datos);
					}else{
						i=2;
						while(peticion[i]!='\0')//compara con los parametros los datos en el archivo
							if(strcmp(peticion[i++],bloq->columna)==0){
								insertarLB(bloq,tablasG.actual->datos);
								break;
							}
					}
					if(bloq->anterior!=-1){//siguiente dato
						fseek(archivo,sizeof(struct Bloque)*(bloq->anterior),SEEK_SET);
						bloq= (struct Bloque*)calloc(1,sizeof(struct Bloque));
						fread(bloq,sizeof(struct Bloque),1,archivo);
					}else break;
				}
			}tablasG.actual = tablasG.actual->sig;
		}
	}fclose(archivo);
}

//Carga todas las columnas de un tabla
void seleccionarTodo(){
	FILE* archivo = fopen("catalogo.dat","r+b");
	if(archivo){
		fseek(archivo,sizeof(struct BloqueC)*25,SEEK_SET);
		struct Tabla* tabla=(struct Tabla*)calloc(1,sizeof(struct Tabla));
		struct Nodo* nodo;
		initLT(&tablasG);
		fread(tabla,sizeof(struct Tabla),1,archivo);
		while(!feof(archivo)){
			nodo = crearNodo(tabla);
			fread(nodo->columnas,sizeof(struct BloqueC)*((tabla->numColumnas)*2),1,archivo);
			insertarLT(nodo,&tablasG);
			tabla=(struct Tabla*)calloc(1,sizeof(struct Tabla));
			fread(tabla,sizeof(struct Tabla),1,archivo);
		}
		free(tabla);
	}else{ printf("Error al leer el archivo \n");fclose(archivo); }
	fclose(archivo);
	cargaDatos(false);
}

//Carga la tabla y llama a cargar datos
void seleccionarTabla(char* nombre,bool parametros){
	FILE* archivo = fopen("catalogo.dat","r+b");
	bool entro=false;
	if(archivo){
		fseek(archivo,sizeof(struct BloqueC)*25,SEEK_SET);
		struct Tabla* tabla=(struct Tabla*)calloc(1,sizeof(struct Tabla));
		struct Nodo* nodo;
		initLT(&tablasG);
		fread(tabla,sizeof(struct Tabla),1,archivo);
		while(!feof(archivo)){
			if(strcmp(tabla->nombre,nombre)==0){
				nodo = crearNodo(tabla);
				fread(nodo->columnas,sizeof(struct BloqueC)*((tabla->numColumnas)*2),1,archivo);
				insertarLT(nodo,&tablasG);
				cargaDatos(parametros);
				entro=true;
				break;
			}
			fseek(archivo,sizeof(struct BloqueC)*(tabla->numColumnas)*2,SEEK_CUR);
			fread(tabla,sizeof(struct Tabla),1,archivo);
		}
		entro==false?free(tabla):NULL;
	}else{ printf("Error al leer el archivo \n"); }
	fclose(archivo);
}

//Decide que seleccionar aplicar y destruye la lista
void seleccionar(){
	int cant=pesoCadena(peticion);
	switch(cant){
		case 1:
			seleccionarTodo();
			imprimeListaT(false);
			break;
		case 2:
			seleccionarTabla(peticion[1],false);
			imprimeListaT(false);
			break;
		default:
			seleccionarTabla(peticion[1],true);
			imprimeListaT(true);
			break;
	}
	destructorLT(&tablasG);
}

//Parsing:---------------------------------------------------------
//Metodos para la verificacion de la gramatica

//Comprueba que la petición esté bien escrita
int verificaSintaxis(char *comando) {
	int fin = 0, tamano= 0;
	bool signo= true, letra= false, coma= false, cierre= false;
	ourToLower(comando);

	while(comando[++fin]!='\0') {
		switch(comando[fin]) {
		case ' ':
			if (comando[fin - 1] != ' ' && comando[fin - 1] != ',' && comando[fin - 1] != '@' && comando[fin - 1] != '#')
				++tamano;
			break;

		case ',':
			if (cierre || coma || signo || !letra) {
				printf("Error: ',' \n");
				return -1;
			}
			if(comando[fin - 1] != ' ')
				++tamano;
			coma= true;
			break;

		case '@':
			if (cierre || !signo || coma) {
				printf("Error: '@' \n");
				return -1;
			}
			if(comando[fin - 1] != ' ')
				++tamano;
			signo= false;
			break;

		case '#':
			if(cierre || signo || coma) {
				printf("Error: '#' \n");
				return -1;
			}
			if(comando[fin - 1] != ' ')
				++tamano;
			cierre= true;
			signo= true;
			break;
		default:
			if(cierre || (tamano==2 && signo) || (!isalpha(comando[fin]) && !isdigit(comando[fin])) || (!signo && isdigit(comando[fin]))) {
				printf("Error: En signo \n");
				return -1;
			}
			if(!signo)
				letra= true;
			coma= false;
			break;
		}
	}
	if(!signo ||(!cierre && tamano>1)) {
		printf("Error: Parametros abiertos \n");
		return -1;
	}

	return tamano;
}

//Extrae de la petición las palabras clave
char ** detectaPalabras (char *comando, int tamano) {
	char ** instrucciones = (char **)calloc(tamano + 1, sizeof(char *));
	int inicio = 0, fin = 0, contador=0;
	while(comando[++fin]!='\0') {
			if(comando[fin] == ' ' || comando[fin] == ',' || comando[fin] == '@' || comando[fin] == '#') {
				if (comando[fin - 1] == ' ' || comando[fin - 1] == ',' || comando[fin - 1] == '@' || comando[fin - 1] == '#')
						inicio = fin + 1;
				else {
					instrucciones[contador]= substr(comando,inicio, fin-inicio);
					inicio = fin + 1;
					++contador;
				}
			}
	}
	return instrucciones;
}

//Extrae el nombre de las columnas de una tabla
char ** cargarColumnas(char* _nombre) {
	FILE* archivo = fopen("catalogo.dat","rb");
	int i = 0,tam=0;
	struct BloqueC *bloq=NULL;
	char** respuesta = NULL;
	if(archivo){

		struct Tabla tabla;
		fseek(archivo,sizeof(struct BloqueC)*25,SEEK_SET);
		fread(&tabla,sizeof(struct Tabla),1,archivo);

		while(!feof(archivo)){
			if(strcmp(tabla.nombre,_nombre)==0){
				tam = tabla.numColumnas;
				respuesta = (char **)calloc(tam+1, sizeof(char *));
				bloq = (struct BloqueC*)calloc(tam*2, sizeof(struct BloqueC));
				fread(bloq,sizeof(struct BloqueC),tam*2,archivo);
				for(i=tam*2-1;0<=i;i=i-2){
					respuesta[tam-(i+1)/2]=substr(bloq[i].datos,0,strlen(bloq[i].datos));
				}
				break;
			}else{
				fseek(archivo,sizeof(struct BloqueC)*(tabla.numColumnas*2),SEEK_CUR);
			}
			fread(&tabla,sizeof(struct Tabla),1,archivo);
		}
	}else{printf("Error al leer el archivo(de caballos)\n");}
	fclose(archivo);
	if(bloq!=NULL)
		free(bloq);
	return respuesta;
}

//Verifica que no se repitan los nombres de columnas
bool repiteNombre() {
	int cursor, iterador;
	for(cursor= 2; peticion[cursor]!='\0' ; cursor++)
		for(iterador= 2; peticion[iterador]!='\0'; iterador++)
			if(cursor!=iterador)
				if(strcmp(peticion[cursor], peticion[iterador])==0)
					return true;
	return false;
}

//Comprueba que el orden lógico de las palabras extraidas y valida que no hayan errores
bool verificaSemantica () {
	char ** columna; int cursor= 2, iterador; bool seguir;
	if(peticion[1]!='\0') {
		columna = cargarColumnas(peticion[1]);
		if(columna==NULL) {
			printf("Error: No existe esta tabla \n");
			return false;
		}
	} else return true;

	if(peticion[2]=='\0')
		return true;

	do {
		iterador= 0; seguir= false;
		while(columna[iterador]!='\0') {
			if(strcmp(peticion[cursor], columna[iterador])==0){
				seguir= true;
				break;
			}
			++iterador;
		}
		if(!seguir) {
			printf("Error: Columna inexistente: %s \n", peticion[cursor]);
			return false;
		}
	} while(peticion[++cursor]!='\0');
	if(repiteNombre()) {
		printf("Se repiten nombres de columnas \n");
		return false;
	}
	return true;
}

//Hace uso de varios metodos para realizar el parcing
bool parsing (char *comando) {
	int tamano= verificaSintaxis(comando);
	if(tamano == -1){
		printf("Error de sintaxis \n");
		return false;
	}
	else{

		peticion = detectaPalabras(comando, tamano);

		if(verificaSemantica()){
			printf("Correcto \n");
			return true;
		}else{
			printf("Error de semantica \n");
			return false;
		}
	}
}

//Main: Principal
int main(int argc, char* argv[]) {
	int i=0,tamargc=0;
	char* temp;

	while(argv[i]!='\0'){
        tamargc+=strlen(argv[i])+1;
		i++;
	}

	temp=(char *)calloc(tamargc+1, sizeof(char ));
	i=0;

	while(argv[i]!='\0'){
        strcat(temp,argv[i]);
        strcat(temp," ");
		i++;
	}

	if(parsing(temp))
		seleccionar();
	free(peticion);
    return (EXIT_SUCCESS);
}
