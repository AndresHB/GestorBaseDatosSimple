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

//User:-------------------------------------------------------------
//Estructuras para recorrer Users.dat

struct Bloque {
	struct Bloque * sig;
	char tipo[7];
	char columna[25];
	char datos[30];
	unsigned long int anterior;
};

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
	if(cadena==NULL)return 0;
	while(cadena[i]!='\0'){
		peso+=1;
		i++;
	}
	return peso;
}

//Modificar bloques:----------------------------------------------
//Metodos para modificar bloques

//Verifica si la columna a modificar exista en esa tabla
bool verificaColumnas(struct BloqueC* bloq,int tamC){
	bool error=true;
	int i=0,j=0;
	int tamP= pesoCadena(peticion)-2;
	for(i=2; i<tamP;i=3+i){
		for(j=1;j<tamC;j=2+j){
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

//Copia los datos de un bloque en otro
void copiarBloque(struct Bloque* des,struct Bloque* origen){
	des->sig=NULL;
	strcpy(des->tipo,origen->tipo);
	strcpy(des->columna,origen->columna);
	strcpy(des->datos,"  ");
	des->anterior=origen->anterior;
}

// Cambia los bloques de los datos en usuarios.dat
void cambiarDatos(unsigned long int inicio){
	FILE* archivo = fopen("usuarios.dat","r+b");
	struct Bloque bloq1;
	struct Bloque bloq2;
	int i=0,tam = pesoCadena(peticion)-2;
	bool primera=true;
	bool cambiar=false;
	if(archivo){
		fseek(archivo,sizeof(struct Bloque)*inicio,SEEK_SET);
		fread(&bloq1,sizeof(struct Bloque),1,archivo);
		while(bloq1.anterior!=-1||primera){
			if(bloq1.anterior==-1 && primera==true)primera=false;
			for(i=2;i<tam; i=i+3){
				if(strcmp(bloq1.columna,peticion[i])==0)
					if(strcmp(bloq1.datos,peticion[i+1])==0){
						cambiar = true;
						break;
					}
			}
			if(cambiar){
				copiarBloque(&bloq2,&bloq1);
				strcpy(bloq2.datos,peticion[i+2]);
				fseek(archivo,sizeof(struct Bloque)*-1,SEEK_CUR);
				fwrite(&bloq2,sizeof(struct Bloque),1,archivo);
				cambiar=false;
				inicio=bloq1.anterior;
			}

			fseek(archivo,sizeof(struct Bloque)*(bloq1.anterior),SEEK_SET);
			fread(&bloq1,sizeof(struct Bloque),1,archivo);

		}
	}else{printf("Error al leer el archivo\n");}
	fclose(archivo);
}

//Busca y modifica la tabla a cambiar y llama a cambiarDatos()
bool modificar() {
	FILE* archivo = fopen("catalogo.dat","rb");
	struct BloqueC * bloq=NULL;
	if(archivo){
		struct Tabla tabla;
		fseek(archivo,sizeof(struct BloqueC)*25,SEEK_SET);
		fread(&tabla,sizeof(struct Tabla),1,archivo);
		while(!feof(archivo)){
			if(strcmp(tabla.nombre,peticion[1])==0){
				bloq=(struct BloqueC*)calloc(tabla.numColumnas * 2,sizeof(struct BloqueC));
				fread(bloq,sizeof(struct BloqueC),tabla.numColumnas*2,archivo);
				if(!verificaColumnas(bloq,tabla.numColumnas *2))
					return false;
				else{
					cambiarDatos(tabla.ultimo_Bloque);
					return true;
				}
			}else{
				fseek(archivo,sizeof(struct BloqueC)*(tabla.numColumnas*2),SEEK_CUR);
			}
			fread(&tabla,sizeof(struct Tabla),1,archivo);
		}
	}else{printf("Error al leer el archivo\n");}
	fclose(archivo);
	if(bloq!=NULL)
		free(bloq);
	return true;

}

//Parsing:---------------------------------------------------------
//Metodos para la verificacion de la gramatica

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
char ** cargarColumnas(char* _nombre){
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
				printf("Signo: %d, Coma: %d, Punto: %d \n", signo, coma, punto);
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
	if(!signo)
		return -1;
	return tamano;
}

//verifica si los datos insertados como parametros son tres digitos
bool verificaTamano(int cursor) {
	int contador= 0;
	if(peticion[cursor]!='\0') {
		++contador;
		if(peticion[cursor+1]!='\0') {
			++contador;
			if(peticion[cursor+2]!='\0')
				++contador;
		}
	}
	return contador;
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

//Verifica que los parametros se adecuen a las  reglas y a los valores de la tabla
bool verificaParametros(int cursor, char** columna) {
	int iterador; bool ausente= true;
	for(iterador= 0; columna[iterador]!='\0'; iterador+=2) {
		if(strcmp(peticion[cursor], columna[iterador])==0)	{
			ausente= false;
			if(!(verificaTipo(peticion[cursor+1], columna[iterador+1])) || !(verificaTipo(peticion[cursor+2], columna[iterador+1])))
				return false;
		}
	}
	if(ausente)
		return false;
	return true;
}

//Comprueba que el orden lógico de las palabras extraidas y valida que no hayan errores
bool verificaSemantica () {
	char ** columna = cargarColumnas(peticion[1]);
	int cursor= 2, tres; bool seguir;
	if(columna==NULL) {
		printf("Error: No existe esta tabla \n");
		return false;
	}
	do {
		if(!verificaParametros(cursor, columna)) {
			printf("Error: En parametros \n");
			return false;
		}
		cursor= cursor+3;
		tres = verificaTamano(cursor);
		if(0<tres && tres<3) {
			printf("Error: Parametros insuficientes \n");
			return false;
		}
		if(tres==0)
			seguir= false;
		else
			seguir= true;
	} while(seguir);
	return true;
}

//Hace uso de varios metodos para realizar el parsing
bool parsing (char *comando) {
	int tamano= verificaSintaxis(comando);
	if(tamano < 5){
		printf("Error de sintaxis \n");
		return false;
	}else{
		peticion= detectaPalabras(comando, tamano);
		if(verificaSemantica()) {
			printf("Correcto \n");
			return true;
		}else{
			printf("Error de semantica \n");
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

	if(parsing(temp)==true)
		modificar();

	free(temp);
    return (EXIT_SUCCESS);
}
