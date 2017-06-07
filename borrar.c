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

//Variables Globales-----------------------------------------------
char** peticion=NULL;

//Catalogo:---------------------------------------------------------
//Estructuras para recorrer Catalogo.dat
struct BloqueC{
	char tipo;
	char datos[25];
	struct BloqueC* sig;
};

struct Tabla{
	char nombre[30];                  // T1
	unsigned long int primer_Bloque;  // 0
	unsigned long int ultimo_Bloque;  // 0
	unsigned int numColumnas;         // 0
	struct Tabla* sig;                // NULL
};

//User:-------------------------------------------------------------
//Estructuras para recorrer Users.dat
struct Bloque {
	struct Bloque * sig;  //      -> 01
	char tipo[7];  		  //INT   -> TEXTO
	char columna[25];     //EDAD  -> NOMBRE
	char datos[30];       //20    -> ANDRES
	unsigned long int anterior;
};

// Metodos str: ---------------------------------------------------
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
	int i=0;
	while(cadena[i]!='\0'){
		peso+=1;
		i++;
	}
	return peso;
}

//Borrar bloques:----------------------------------------------
//Metodos para borrar bloques

//Verifica si la columna a eliminar exista en esa tabla
bool verificaColumnas(struct BloqueC* bloq,int tamC){
	bool error=true;
	int i=0,j=0;
	int tamP = pesoCadena(peticion)-2;
	for(i=2; i< tamP;i=2+i){
		for(j=1; j < tamC;j=2+j){
			if(strcmp(peticion[i],bloq[j].datos)==0)
				error=false;
		}
	}
	if(error==true)
		return false;
	else
		error=true;

	return true;
}

//Mueve el puntero del archivo al primer elemento de la tupla a eliminar
long int primeroTupla(FILE* archivo,struct Bloque* bloq1,char* princT){
	do{
		if(strcmp(princT,bloq1->columna)==0){
			return (ftell(archivo)-sizeof(struct Bloque));
		}
		fseek(archivo,sizeof(struct Bloque)*(bloq1->anterior),SEEK_SET);
		fread(bloq1,sizeof(struct Bloque),1,archivo);
	}while(bloq1->anterior!=-1);
	return -2;
}

//Borra los datos en el archivo usuarios.dat
void borrarDatos(struct Tabla* tabla,char* princT){
	FILE* archivo = fopen("usuarios.dat","r+b");
	struct Bloque bloq1;
	struct Bloque bloq2;
	bool primera=true;
	int inicio = tabla->ultimo_Bloque , borrarPos=0, posAnt=-2;
	if(archivo){
		fseek(archivo,sizeof(struct Bloque)*inicio,SEEK_SET);
		fread(&bloq1,sizeof(struct Bloque),1,archivo);
		if(tabla->ultimo_Bloque>=tabla->primer_Bloque){
			if(!feof(archivo)){
				while(bloq1.anterior!=-1||primera){
					if(bloq1.anterior==-1 && primera==true)primera=false;
					if(strcmp(bloq1.columna,peticion[2])==0){
						if(strcmp(bloq1.datos,peticion[3])==0){
							borrarPos=primeroTupla(archivo,&bloq1,princT);
							if(posAnt!=-2){
								fseek(archivo,posAnt,SEEK_SET);
								fread(&bloq2,sizeof(struct Bloque),1,archivo);
								bloq2.anterior=bloq1.anterior;
								fseek(archivo,posAnt,SEEK_SET);
								fwrite(&bloq2,sizeof(struct Bloque),1,archivo);
								if(bloq2.anterior!=-1){
									fseek(archivo,sizeof(struct Bloque)*(bloq2.anterior),SEEK_SET);
									fread(&bloq1,sizeof(struct Bloque),1,archivo);
								}
							}else {
								borrarPos=primeroTupla(archivo,&bloq1,princT);
								tabla->ultimo_Bloque=bloq1.anterior;
								fseek(archivo,sizeof(struct Bloque)*(bloq1.anterior),SEEK_SET);
								fread(&bloq1,sizeof(struct Bloque),1,archivo);
							}
						}else{
							goto sig;
						}
					}else{
					sig:if(strcmp(bloq1.columna,princT)==0)
							posAnt=ftell(archivo)-sizeof(struct Bloque);
						fseek(archivo,sizeof(struct Bloque)*(bloq1.anterior),SEEK_SET);
						fread(&bloq1,sizeof(struct Bloque),1,archivo);
					}
					if(tabla->ultimo_Bloque==-1){tabla->ultimo_Bloque=0;tabla->primer_Bloque=1;break;}
				}

			}else{printf("Error al leer el archivo\n");}
		}else{printf("Error al leer el archivo(de caballos)\n");}
	}
	fclose(archivo);
}

//Encuentra, Cambia la tabla seleccionada y llama a borrarDatos()
void borrar(){
	FILE* archivo = fopen("catalogo.dat","r+b");
	int pos=0;
	if(archivo){
		struct Tabla tabla;
		struct BloqueC bloq;
		fseek(archivo,sizeof(struct BloqueC)*25,SEEK_SET);
		pos=ftell(archivo);
		fread(&tabla,sizeof(struct Tabla),1,archivo);
		while(!feof(archivo)){
			if(strcmp(tabla.nombre,peticion[1])==0){
				fseek(archivo,sizeof(struct BloqueC)*(1+(tabla.numColumnas-1)*2),SEEK_CUR);
				fread(&bloq,sizeof(struct BloqueC),1,archivo);
				borrarDatos(&tabla,bloq.datos);
				fseek(archivo,pos,SEEK_SET);
				fwrite(&tabla,sizeof(struct Tabla),1,archivo);
				break;
			}else{
				fseek(archivo,sizeof(struct BloqueC)*(tabla.numColumnas*2),SEEK_CUR);
			}
			pos=ftell(archivo);
			fread(&tabla,sizeof(struct Tabla),1,archivo);
		}
	}else{printf("Error al leer el archivo\n");}
	fclose(archivo);
}

//Parsing:---------------------------------------------------------
//Metodos para la verificacion de la gramatica

//Comprueba que la petición esté bien escrita
int verificaSintaxis(char *comando) {
	int fin = 0, tamano= 0;
	bool signo= true, letra= false, coma= false, punto= false, seguir= true;
	ourToLower(comando);

	while(comando[++fin]!='\0' && seguir) {
		switch(comando[fin]) {
		case ' ':
			if (comando[fin - 1] != ' ' && comando[fin - 1] != ',' && comando[fin - 1] != '@' && comando[fin - 1] != '#')
				++tamano;
			break;

		case ',':
			if (coma || signo || !letra) {
				printf("Error: ',' \n");
				return -1;
			}
			if(comando[fin - 1] != ' ')
				++tamano;
			coma= true;
			break;

		case '@':
			if (!signo || coma) {
				printf("Error: '@' \n");
				return -1;
			}
			if(comando[fin - 1] != ' ')
				++tamano;
			signo= false;
			break;

		case '.':
			if(punto || signo || coma || !isdigit(comando[fin - 1]) || !isdigit(comando[fin + 1])) {
				printf("Error: '.' \n");
				return -1;
			}
			punto= true;
			break;
		case '#':
			if(signo || coma || punto) {
				printf("Error: '#' \n");
				return -1;
			}
			if(comando[fin - 1] != ' ')
				++tamano;
			seguir= false;
			signo= true;
			break;
		default:
			if((tamano==2 && signo) || (comando[fin]!='-' && !isalpha(comando[fin]) && !isdigit(comando[fin]))) {
				printf("Error: En signo \n");
				return -1;
			}
			if(isdigit(comando[fin]))
				punto= false;
			if(!signo)
				letra= true;
			coma= false;
			break;
		}
	}
	if(tamano%2!=0 || !signo)
		return -1;
	else
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
char** cargarColumnas(char* _nombre){
	FILE* archivo = fopen("catalogo.dat","rb");
	int i = 0,tam;
	struct BloqueC *bloq;
	char** respuesta = NULL;
	if(archivo){
		struct Tabla tabla;
		fseek(archivo,sizeof(struct BloqueC)*25,SEEK_SET);
		fread(&tabla,sizeof(struct Tabla),1,archivo);
		while(!feof(archivo)){
			if(strcmp(tabla.nombre,_nombre)==0){
				tam = tabla.numColumnas;
				respuesta = (char **)calloc(tam*2+1, sizeof(char *));
				bloq = (struct BloqueC*)calloc(tam*2+1, sizeof(struct BloqueC));
				fread(bloq,sizeof(struct BloqueC),tam*2,archivo);
				for(i=0;i<tam*2;i++){
					respuesta[i]=substr(bloq[(tam*2-1)-i].datos,0,strlen(bloq[(tam*2-1)-i].datos));
				}
				break;
			}else {
				fseek(archivo,sizeof(struct BloqueC)*(tabla.numColumnas*2),SEEK_CUR);
			}
			fread(&tabla,sizeof(struct Tabla),1,archivo);
		}
	}else{printf("Error al leer el archivo\n");fclose(archivo);return NULL;}
	fclose(archivo);
	free(bloq);
	return respuesta;
}

//Verifica si el dato es del tipo correcto segun la tabla
bool verificaTipo(char* instruccion, char* columna){
	int iterador= 0;
	switch(columna[0]){
	case 'c':
		if(strlen(instruccion)!=1) {
			printf("Error en char");
			return false;
		}
		break;
	case 'i':
		if(instruccion[iterador]=='-')
			++iterador;
		if(instruccion[iterador]=='\0')
			return false;
		while(instruccion[iterador]!='\0') {
			if(!isdigit(instruccion[iterador]))
				return false;
			else
				++iterador;
		}
		break;
	case 'f':
		if(instruccion[iterador]=='-')
			++iterador;
		if(instruccion[iterador]=='\0')
			return false;
		while(instruccion[iterador]!='\0') {
			if(!isdigit(instruccion[iterador]) && instruccion[iterador]!='.')
				return false;
			++iterador;
		}
		break;
	case 'b':
		if(strlen(instruccion)!=1 || (instruccion[iterador]!='1' && instruccion[iterador]!='0'))
			return false;
		break;
	default:break;
	}
	return true;
}

//Comprueba que el orden lógico de las palabras extraidas y valida que no hayan errores
bool verificaSemantica () {
	char ** columna; int verificaPar= 2, iterador; bool ausente;
	columna = cargarColumnas(peticion[1]);
	if(columna==NULL) {
		printf("Error: No existe esta tabla \n");
		return false;
	}
	do{
		iterador= 0; ausente= true;
		while(columna[iterador]!='\0' && ausente) {
			if(strcmp(peticion[verificaPar], columna[iterador])==0) {
				ausente= false;
				if(!verificaTipo(peticion[verificaPar+1], columna[iterador+1])) {
					printf("Error: Criterio invalido en columna: %s \n", peticion[verificaPar]);
					return false;
				}
			}
			iterador= iterador+2;
		}
		if(ausente) {
			printf("Error: Columna inexistente: %s \n", peticion[verificaPar]);
			return false;
		}
		//if(peticion[++verificaPar]=='\0')
		//	return false;
		verificaPar+=2;
	}while(peticion[verificaPar]!='\0');//++
	return true;
}

//Hace uso de varios metodos para realizar el parsing
bool parsing (char *comando) {
	int tamano= verificaSintaxis(comando);
	if(tamano != 4){
		printf("\n Error de sintaxis \n");
		return false;
	}else{
		peticion = detectaPalabras(comando, tamano);

		if(verificaSemantica()){
			printf("\n Correcto \n");
			return true;
		}else{
			printf("\n Error de semantica \n");
			return false;
		}
	}
}

//Main: principal
int main(int argc, char* argv[]) {
	int i=0,tamargc=0;
	char* temp;

	while(argv[i]!='\0') {
        tamargc+=strlen(argv[i])+1;
		i++;
	}

	temp=(char *)calloc(tamargc +1, sizeof(char ));
	i=0;
	while(argv[i]!='\0') {
        strcat(temp,argv[i]);
        strcat(temp," ");
		i++;
	}

	if(parsing(temp)){
		borrar();
	}
	if(peticion!=NULL)
		free(peticion);

	free(temp);
    return (EXIT_SUCCESS);
}
