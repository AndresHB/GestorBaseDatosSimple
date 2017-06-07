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
char** columnaG=NULL;
char** datos=NULL;

// Catalogo:  ---------------------------------------------
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

//User:-------------------------------------------------------------
//Estructuras para recorrer Users.dat

struct Bloque {
	struct Bloque * sig;
	char tipo[7];
	char columna[25];
	char datos[30];
	unsigned long int anterior;
};

// Metodos str: --------------------------------------------
// Metodos para manejo de char*

//Funciona como toLower en C
void ourToLower(char *comando) {
	int iterador=0;
    while(comando[iterador]!='\0') {
        comando[iterador]=tolower(comando[iterador]);
        iterador++;
	}
}

//Crea una subcadena:
char* substr(char *cadena, int comienzo, int longitud) {
	if (longitud == 0) longitud = strlen(cadena)-comienzo-1;
	char* nuevo=(char*)calloc(longitud+2,sizeof(char));
	strncpy(nuevo, cadena + comienzo, longitud);
	return nuevo;
}

//Retorna el peso del array de char*
int pesoCadena(char** cadena) {
	int peso=0;
	while(cadena[peso]!='\0')
		peso++;
	return peso;
}

//Insertar bloques:----------------------------------------------
//Metodos para insertar y guardar tablas

//Insertar tablas: Metodos para insertar y guardar datos
void insertaDatos(unsigned long int * primero, unsigned long int * ultimo){
	FILE* archivoU = fopen("usuarios.dat","ab");
	FILE* archivoUR = fopen("usuarios.dat","rb");
	unsigned long int pos=0, ant=0;
	struct Bloque* bloq=NULL;
	bool primV=false,Nvacio=false;
	int i=0,j=0,e=0,tamC = pesoCadena(columnaG),tamD=pesoCadena(datos)-2;
	bloq=(struct Bloque*)calloc(tamC/2,sizeof(struct Bloque));
	if(archivoU){
		fseek(archivoUR,sizeof(struct Bloque)*-1,SEEK_END);
		pos = ftell(archivoUR);
		if(*primero>*ultimo){
			primV=true;
			if(pos==0){ //Listo
				*primero=0;
				*ultimo=((tamC)/2)-1;
				 ant=0;
			}else{
				Nvacio=true;
				fseek(archivoUR,0,SEEK_END);
				pos = ftell(archivoUR);
				*primero=(pos/sizeof(struct Bloque));
				*ultimo=*primero+(tamC)/2 -1;
				ant=*primero;
			}
		}else{
			fseek(archivoUR,0,SEEK_END);
			pos = ftell(archivoUR);
			ant = (pos/sizeof(struct Bloque));
		}

		for(i=0,j=0;j<tamC/2;i=i+2,j++){
				bloq[j].sig=NULL;
				strcpy(bloq[j].columna,columnaG[i]);
				strcpy(bloq[j].tipo,columnaG[i+1]);
				strcpy(bloq[j].datos,"NULL");
				if(primV){
					if(Nvacio){
						bloq[j].anterior= -1;
						Nvacio=false;
					}else {
						bloq[j].anterior= ant+(j-1);
					}
				}else {
					bloq[j].anterior=*ultimo;
					*ultimo=(pos/sizeof(struct Bloque))+((tamC)/2 -1);
					primV=true;
				}
				for(e=2;e<tamD+2;e=e+2){
					if(strcmp(datos[e],columnaG[i])==0){
						strcpy(bloq[j].datos,datos[e+1]);
						break;
					}
				}
		}fwrite(bloq,sizeof(struct Bloque),(tamC)/2,archivoU);
	}else{printf("Error al leer el archivo\n");}
	fclose(archivoU);
	fclose(archivoUR);
}

//Modifica los datos de las tablas despues de insertar en ellas
void insertar(){
	FILE* archivoC = fopen("catalogo.dat","r+b");
	if(archivoC){
		struct Tabla tabla;
		struct Tabla tabla1;
		fseek(archivoC,sizeof(struct BloqueC)*25,SEEK_SET);
		fread(&tabla,sizeof(struct Tabla),1,archivoC);
		while(!feof(archivoC)){
			if(strcmp(tabla.nombre,datos[1])==0){
				insertaDatos(&(tabla.primer_Bloque),&(tabla.ultimo_Bloque));
				tabla1.sig=NULL;
				tabla1.numColumnas = tabla.numColumnas;
				tabla1.primer_Bloque=tabla.primer_Bloque;
				tabla1.ultimo_Bloque=tabla.ultimo_Bloque;
				strcpy(tabla1.nombre, tabla.nombre);
				fseek(archivoC,(sizeof(struct Tabla)*-1),SEEK_CUR);
				fwrite(&tabla1,sizeof(struct Tabla),1,archivoC);
				break;
			}else{
				fseek(archivoC,sizeof(struct BloqueC)*(tabla.numColumnas*2),SEEK_CUR);
			}
			fread(&tabla,sizeof(struct Tabla),1,archivoC);
		}
	}else{printf("Error al leer el archivo\n");}
	fclose(archivoC);
}

//Parsing:---------------------------------------------------
//Metodos para la verificacion de la gramatica

//Comprueba que la petición esté bien escrita
int verificaSintaxis(char *comando) {
	int fin = 0, tamano= 0;
	bool signo= true, letra= false, coma= false, punto= false, cierre= false;
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

		case '.':
			if(cierre || punto || signo || coma || !isdigit(comando[fin - 1]) || !isdigit(comando[fin + 1])) {
				printf("Error: '.' \n");
				return -1;
			}
			punto= true;
			break;

		case '#':
			if(cierre || signo || coma || punto) {
				printf("Error: '#' \n");
				return -1;
			}
			if(comando[fin - 1] != ' ')
				++tamano;
			cierre= true;
			signo= true;
			break;

		default:
			if(cierre || (tamano==2 && signo) || (signo && comando[fin]!='-' && !isalpha(comando[fin]) && !isdigit(comando[fin]))) {
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
	if(tamano%2!=0 || !signo || !cierre) {
		printf("Error: Faltan argumentos \n");
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
char** cargarColumnas(char* _nombre){
	FILE* archivo = fopen("catalogo.dat","rb");
	int i = 0,tam;
	struct BloqueC *bloq=NULL;
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
			}else{
				fseek(archivo,sizeof(struct BloqueC)*(tabla.numColumnas*2),SEEK_CUR);
			}
			fread(&tabla,sizeof(struct Tabla),1,archivo);
		}
	}else{printf("Error al leer el archivo\n");}
	fclose(archivo);
	if(bloq!=NULL)
		free(bloq);
	return respuesta;
}

//Verifica si el valor por ingresar corresponde a la columna
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

//Verifica que no se repitan los nombres de columnas
bool repiteNombre() {
	int cursor, iterador;
	for(cursor= 2; datos[cursor]!='\0' ; cursor++) {
		for(iterador= 2; datos[iterador]!='\0'; iterador++) {
			if(cursor!=iterador)
				if(strcmp(datos[cursor], datos[iterador])==0)
					return true;

			if(datos[iterador+1]=='\0') break;
			else iterador++;
		}
		if(datos[cursor+1]=='\0') break;
		else cursor++;
	}
	return false;
}

//Comprueba que el orden lógico de las palabras extraidas y valida que no hayan errores
bool verificaSemantica () {
	int verificaPar= 2, iterador; bool ausente;
	columnaG = cargarColumnas(datos[1]);
	if(columnaG==NULL) {
		printf("Error: No existe esta tabla \n");
		return false;
	}
	do{
		iterador= 0; ausente= true;
		while(columnaG[iterador]!='\0' && ausente) {
			if(strcmp(datos[verificaPar], columnaG[iterador])==0) {
				ausente= false;
				if(!verificaTipo(datos[verificaPar+1], columnaG[iterador+1])) {
					printf("Error: Insercion invalida en columna: %s \n", datos[verificaPar]);
					return false;
				}
			}
			iterador= iterador+2;
		}
		if(ausente) {
			printf("Error: Columna inexistente: %s \n", datos[verificaPar]);
			return false;
		}
		verificaPar+=2;
	}while(datos[verificaPar]!='\0');
	if(repiteNombre()) {
		printf("Se repiten nombres de columnas \n");
		return false;
	}
	return true;
}

//Hace uso de varios metodos para realizar el parsing
bool parsing (char *comando) {
	int tamano= verificaSintaxis(comando);
	if(tamano < 4)
		printf("Error de sintaxis \n");
	else
	{
		datos= detectaPalabras(comando, tamano);

		if(verificaSemantica()){
			printf("Insercion realizada \n");
			return true;
		}else{
			printf("Error de semantica \n");
			return false;
		}
	}
	return false;
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


	if(parsing(temp)){
		insertar();
	}
	datos!=NULL?free(datos):NULL;
	columnaG!=NULL?free(columnaG):NULL;
	free(temp);
    return (EXIT_SUCCESS);
}
