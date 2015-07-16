#ifndef ABM_H_

#define ABM_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#include <commons/collections/list.h>
#include <mensajeria/mensajes.h>

t_list * archivos; //Lista de t_archivos
t_list * nodos; //Lista de t_nodos
t_dictionary * bloques_nodos_archivos;
t_dictionary * bloques_nodo_disponible;
unsigned long contador_archivos;
unsigned long contador_nodo;

typedef struct {
	t_list* directorios;
	unsigned long contador;
} t_directorios_self;

t_directorios_self* directorios;

typedef struct {
	char* nombre;
	char* ruta;
	char* padre;
	int nivel;
	char* id;
} t_directorio_self;

typedef struct {
	int estado;
	t_list * copias; //Lista de t_copia
	int numero;
} t_bloque_archivo_self;

typedef struct {
	int estado;
	long tamanio;
	t_dictionary * bloques; //Lista de t_bloque_archivo
	char* nombre;
	char* id;
	int cantidad_bloques;
	int bloques_invalidos; //Lista de t_bloque_archivo
	char* ruta_id; // Es un
} t_archivo_self;

typedef struct {
	t_list* bloques;
	long tamanio;
} t_archivo_datos_self;

t_archivo_datos_self * datos; //Estructura temporal para simular la obtencion de datos del nodo.

typedef struct {
	int estado;
	char * nombre;
	char * nodo_id;
	int bloques_disponibles;
	t_dictionary * bloques;
} t_nodo_self;

typedef struct {
	t_nodo_self * nodo;
	int bloque_nodo;
} t_nodo_bloque_self;

typedef struct {
	t_nodo_bloque_self * nodo_bloque;
	int archivo_bloque;
} t_copia_self;

typedef struct {
	char* archivo_id;
	int archivo_bloque;
} t_bloque_archivo_control_self;

#define CANTIDAD_BLOQUES_NODO_DEFAULT 50
#define TAMANIO_BLOQUE_NODO 20971520

#define ARCHIVO_NUEVO "/home/utnso/Escritorio/Nuevo.txt"
#define ARCHIVO_NUEVO2 "/home/utnso/Escritorio/Nuevo2.txt"

#endif /* ABM_H_ */
