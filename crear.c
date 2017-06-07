/*
Autores:
	Carlos Arroyo Villalobos
	Andrés Hernández Bravo
	Andrés Navarro Durán
	Karina Rivera Solano

	Marzo, 2016
*/

//Librerias ------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//Definicion de tipo boolean--------------------------------
typedef int bool;
#define false 0
#define true 1

//Variables Globales----------------------------------------
char** peticion=NULL;


// Catalogo:  ---------------------------------------------
//Estructuras para recorrer Catalogo.dat

struct BloqueC{
	char tipo;
	char datos[25];
	struct BloqueC* sig;
};

struct Tabla{
	char nombre[30];
	unsigned long int primer_Bloque;
	unsigned long int ultimo_Bloque;
	unsigned int numColumnas;
	struct Tabla* sig;
};

// Metodos str: --------------------------------------------
// Metodos para manejo de char*

//Crea una subcadena:
char* substr(char *cadena, int comienzo, int longitud){
	if (longitud == 0) longitud = strlen(cadena)-comienzo-1;
	char* nuevo=(char*)calloc(longitud+1,sizeof(char)); // cambie el 2 por el 1
	strncpy(nuevo, cadena + comienzo, longitud);
	return nuevo;
}

//Retorna el tamaño del array de char*
int pesoCadena(char** cadena){
	int peso=0;
	while(cadena[peso]!='\0'){
		peso++;
	}return peso;
}

//Crear tablas:----------------------------------------------
//Metodos para crear y guardar tablas

//Crea Catalogo
void crearCatalogo(){
	 int i=0;
	FILE* archivo = fopen("catalogo.dat","wb");
	if(archivo){
		 char* tipos[]= {"int", "string", "char", "bool", "float"};
		 char* SQL[]= {"crear", "insertar", "eliminar", "seleccionar", "actualizar"};
		 struct BloqueC bloq[25];

		 for(i=0; i<25;i++){
			bloq[i].tipo = '\0';
			strcpy(bloq[i].datos,"NULL");
			bloq[i].sig = NULL;
			if(i<5){
				strcpy(bloq[i].datos,tipos[i]);
				bloq[i].tipo='T';
				strcpy(bloq[i+5].datos,SQL[i]);
				bloq[i+5].tipo='S';
			}
		}
		 fwrite(&bloq,sizeof(struct BloqueC),25,archivo);
	}else printf("Error al crear Catalogo.dat");
	fclose(archivo);
}

//Carga los tipos de datos aceptados
char** cargaTipo(){
	char** carac = NULL;
	FILE* archivo=NULL;
otraV:archivo = fopen("catalogo.dat","rb");
	if(archivo){
		int i = 0, cont=0;
		carac=(char **)calloc(6, sizeof(char *));
		struct BloqueC bloq[10];
		fread(&bloq,sizeof(struct BloqueC),10,archivo);
		for(i=0; i<5;i++)
			if(bloq[i].tipo=='T') {
				carac[cont++]=substr(bloq[i].datos,0,strlen(bloq[i].datos));
			}
	}else{printf("Creando archivo...\n");crearCatalogo();goto otraV;}
	fclose(archivo);
	return carac;
}

//Crea las columnas despues de las tablas
bool crearColumnas(unsigned int tam){
	FILE* archivo = fopen("catalogo.dat","ab+");
	tam=2*tam+2;
	if(archivo){
		int i = 0;
		struct BloqueC bloq;
		for(i=2;i<tam;i++){
			strcpy(bloq.datos,peticion[i]);
			bloq.tipo = (i%2==0)?'T':'C';
			fwrite(&bloq,sizeof(struct BloqueC),1,archivo);
		}
	}else{printf("Error al abrir el archivo\n");fclose(archivo); return false;}
	fclose(archivo);
	return true;
}

//Retorna si la Tabla existe o no
bool TablaExiste(char* _nombre) {
	FILE* archivo = fopen("catalogo.dat","rb");
	if(archivo){
		struct Tabla tabla;
		fseek(archivo,sizeof(struct BloqueC)*25,SEEK_SET);
		fread(&tabla,sizeof(struct Tabla),1,archivo);
		while(!feof(archivo)){
			if(strcmp(tabla.nombre,_nombre)==0){
				fclose(archivo);
				return true;
			}
			fseek(archivo,sizeof(struct BloqueC)*tabla.numColumnas*2,SEEK_CUR);
			fread(&tabla,sizeof(struct Tabla),1,archivo);
		}
	}else{printf("Error al leer el archivo\n");}
	fclose(archivo);
	return false;
}

//Crea las Tablas en el archivo junto con sus columnas
bool crearTabla(){
	FILE* archivo = fopen("catalogo.dat","ab+");
	struct Tabla tabla;
	if(archivo){
		strcpy(tabla.nombre,peticion[1]);
		tabla.primer_Bloque=1;
		tabla.ultimo_Bloque=0;
		tabla.sig=NULL;
		tabla.numColumnas=(pesoCadena(peticion)-2)/2;
		fwrite(&tabla,sizeof(struct Tabla),1,archivo);
	}else{printf("Creando Catalogo... \n");fclose(archivo); return false;}
	fclose(archivo);
	if(crearColumnas(tabla.numColumnas))
		return true;
	return false;
}


void crear(){
	if(crearTabla())
		printf("Tabla creada\n");
	else
		printf("Error al crear tabla\n");
}

//Parsing:---------------------------------------------------
//Metodos para la verificacion de la gramatica

//Funciona como toLower en C
void ourToLower(char *COMANDO){
	int i=0;
	while(COMANDO[i]!='\0') {
	        COMANDO[i]=tolower(COMANDO[i]);
	        i++;
	}
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

//Verifica que no se repitan los nombres de columnas
bool repiteNombre() {
	int cursor, iterador;
	for(cursor= 3; peticion[cursor]!='\0' ; cursor++) {
		for(iterador= 3; peticion[iterador]!='\0'; iterador++) {
			if(cursor!=iterador)
				if(strcmp(peticion[cursor], peticion[iterador])==0)
					return true;

			if(peticion[iterador+1]=='\0') break;
			else iterador++;
		}
		if(peticion[cursor+1]=='\0') break;
		else cursor++;
	}
	return false;
}

//Comprueba que el orden lógico de las palabras extraidas y valida que no hayan errores
bool verificaSemantica () {
	char ** tipos= cargaTipo(); bool error;
	int verificaPar= 1, iterador;
	if(TablaExiste(peticion[verificaPar])) {
		tipos==NULL?free(tipos):NULL;
		printf("Error: Nombre de tabla en uso \n");
		return false;
	}
	while(peticion[++verificaPar]!='\0') {
		error= true;
		for(iterador= 0;  iterador < 5; iterador++)
			if(strcmp(peticion[verificaPar], tipos[iterador])==0)
				error= false;

		if(error){
			tipos==NULL?free(tipos):NULL;
			printf("Error: Tipo de columna invalido \n");
			return false;
		}
		++verificaPar;
		for(iterador= 0;  iterador < 5; iterador++)
			if(strcmp(peticion[verificaPar], tipos[iterador])==0)
				error= true;
		if(error){
			tipos==NULL?free(tipos):NULL;
			printf("Error: Nombre de columna invalido \n");
			return false;
		}
	}
	if(repiteNombre()) {
		printf("Se repiten nombres de columnas \n");
		return false;
	}
	tipos==NULL?free(tipos):NULL;
	return true;
}

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
			if(cierre || (tamano==2 && signo) || (signo && !isalpha(comando[fin]) && !isdigit(comando[fin]))) {
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

//Hace uso de varios metodos para realizar el parcing
bool parsing (char *comando) {
	int tamano= verificaSintaxis(comando);
	int resp=0;
	if(tamano < 4){
		printf("Error de sintaxis \n");
		return false;
	}else{
		peticion = detectaPalabras(comando, tamano);
		resp=verificaSemantica();
		if(resp==1){
			return true;
		}else{
			if(resp==0){
				printf("Error de semantica \n");
				return false;
			}
			else{
				printf("Tabla ya Existe \n");
				return false;
			}
		}
	}return false;
}

//Main: Principal
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

	if(parsing(temp)) {
		crear();
		peticion==NULL?free(peticion):NULL;
	}
	if(temp!=NULL)
		free(temp);
    return (EXIT_SUCCESS);
}
