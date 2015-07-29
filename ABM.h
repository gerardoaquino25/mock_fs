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
#include <semaphore.h>
#include <mongoc.h>

#define TAMANIO_MAXIMO_DIRECTORIOS 1024
#define CANTIDAD_BLOQUES_NODO_DEFAULT 50
#define TAMANIO_BLOQUE_NODO 20971520
#define VALIDO 1
#define INVALIDO 0
#define MONGO_DB "utnso"
#define MONGO_SERVER "mongodb://localhost:27017/"

typedef struct {
	t_list* directorios;
	unsigned long contador;
	int contador_directorios;
} t_directorios_self;

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
	char* archivo_id;
	int numero;
} t_copia_self;

typedef struct {
	char* archivo_id;
	int archivo_bloque;
} t_bloque_archivo_control_self;

typedef struct {
	sem_t* semaforo;
	int contador;
} t_semaforo_self;

typedef struct {
	t_dictionary* lectura;
	t_dictionary* escritura;
	t_dictionary* intermedio;
} t_control_self;

typedef struct {
	sem_t* lectura;
	sem_t* escritura;
} t_control_semaforo_self;

t_list * archivos; //Lista de t_archivos
t_list * nodos; //Lista de t_nodos
t_dictionary * bloques_nodos_archivos;
t_dictionary * bloques_nodo_disponible;
unsigned long contador_archivos;
unsigned long contador_nodo;

t_control_self* semaforos_archivo;
t_control_self* semaforos_bloque;
t_control_self* semaforos_nodo;

//t_control_contador_self* mutex_archivos_id;
//t_control_contador_self* mutex_nodos_id;
//t_control_contador_self* mutex_directorios_id;
t_control_semaforo_self* semaforo_control_directorio;

sem_t * m_archivos_id;
sem_t * m_control_directorios;
sem_t * m_directorios_id;
sem_t * m_nodos_id;

t_directorios_self* directorios;
t_archivo_datos_self * datos; //Estructura temporal para simular la obtencion de datos del nodo.

/*
 * Busca un t_archivo en una lista de archivos.
 */
t_archivo_self* find_archivo(char* nombre, t_list* archivosAux);

char* generate_unique_id_to_nodo();

char* generate_unique_id_to_directorio();

char* generate_unique_id_to_archivo();

/*
 * Busca un t_archivo en una lista de archivos.
 */
t_archivo_self* find_archivo_by_id(char* archivo_id, t_list* archivosAux);

int no_tiene_copia(t_archivo_self* archivo, int numero_bloque,
		t_nodo_self* nodo);

/*
 * Busca un t_nodo_tp que sea el que mas espacio disponible tenga en la lista de nodos.
 */
t_nodo_self* find_nodo_disponible(int numero_bloque, t_archivo_self* archivo);

/*
 * Devuelve que bloque de nodo esta disponible y lo quita de la lista
 * de nodos disponibles.
 */
int get_bloque_disponible(t_nodo_self* nodo);

/*
 * Busca un t_nodo_tp en la lista de nodos.
 */
t_nodo_self* find_nodo(char* nombre);

/*
 *Agrega un t_nodo_tp nuevo a la lista de nodos.
 */
void add_nodo_to_nodos(t_nodo_self* nodo, int existente);

/*
 * Inicializa la lista de control de bloques por nodo.
 */
void iniciar_disponibles(t_nodo_self* nodo);

/*
 * Setea como NO disponible a un archivo.
 */
t_archivo_self* archivo_no_disponible(t_archivo_self* archivo);

/*
 * Valida si un archivo que estaba inactivado, pasa a estar activo
 * gracias a la validacion de una copia de un bloque dado.
 */
void chequear_alta_archivo(t_archivo_self* archivo, int numero_bloque);

/*
 * Activa el nodo y busca los archivos utilizados por el nodo reactivado y
 * si estan invalidos verifica si deben ser dados de alta.
 */
void reactivar_nodo(t_nodo_self* nodo);

/*
 * Agrega o reactiva un nodo que ingreso al FS.
 */
void alta_nodo(char* nombre);

/*
 * Dado un archivo y un bloque determinados, verifica si el bloque es invalido
 * y lo setea. En el caso de que lo sea, invalida tambien al archivo.
 */
void chequear_baja_archivo(t_archivo_self* archivo, int archivo_bloque);

/*
 * Desactiva un nodo.
 */
void baja_nodo(char* nombre);

char* create_directorio(char* nombre, int nivel, char* padre, char* ruta);

char * get_directorio_numero(char* nombre, int nivel, char* padre);

char* set_directorio(char* ruta, int extension);

/*
 * Crea un archivo nuevo.
 */
t_archivo_self* create_archivo(long tamanio, char* nombre);

/*
 * Crea un bloque nuevo.
 */
t_bloque_archivo_self* create_bloque(int numero);

/*
 * Crea una copia de un bloque determinado de un archivo y lo asigna a un bloque de un nodo.
 */
t_copia_self* create_copia(int bloque, t_nodo_self* nodo, char* archivo_id, int nodo_bloque_destino, int nodo_ocupado);

/*
 * Asigna un bloque de t_nodo_tp a una t_copia.
 */
void asignar_bloque_nodo_a_copia(t_copia_self* copia, t_nodo_self* nodo, int nodo_bloque_destino, int nodo_ocupado);

/*
 * Asigna una t_copia a las copias de un t_bloque_archivo.
 */
void asignar_copia_a_bloque_archivo(t_bloque_archivo_self* bloque,
		t_copia_self* copia);

void asignar_bloque_archivo_a_archivo(t_archivo_self* archivo,
		t_bloque_archivo_self* bloque);

void add_archivo_to_archivos(t_archivo_self* archivo);

t_archivo_datos_self* dividir_archivo(char* nombre);

void borrado_fisico_nodo(char* nodo);

void formatear_mdfs();

void mover_archivo(char* nombre, char* destino);

void renombrar_archivo(char * ruta, char* nombre);

void eliminar_archivo(char * ruta);

void crear_directorio(char* ruta, char* nombre);

t_directorio_self* get_directorio_by_id(char* archivo_id);

char * get_ruta_directorio(t_directorio_self* directorio);

t_directorio_self* get_directorio_by_ruta(char* ruta);

void renombrar_directorio(char* ruta, char* nombre);

char* get_nombre_archivo(char* ruta);

void copiar_archivo_a_mdfs(char* ruta, char* destino_aux);

void listar_nodos();

void listar_bloques_archivo(char* nombre);

void listar_todo();

void listar_archivos();

void listar_directorios();

void eliminar_directorio(char* ruta);

int get_cantidad_niveles(char* ruta);

int es_subdirectorio_by_ruta(char* ruta, char* destino);

int es_subdirectorio_by_directorio(t_directorio_self* ruta,
		t_directorio_self* destino);

void mover_directorio(char* ruta, char* destino);

int reconstruir_archivo(t_list* bloques, char * ruta, char* nombre);

char* get_data_from_nodo(char* nombre, t_copia_self * bloque, int campo_temporal);

int copiar_archivo_a_local(char* nombre, char* ruta);

void get_MD5(char* nombre);

void ver_bloque(char* nombre, int bloque_id);

void borrar_bloque(char* nombre, int bloque_id);

void copiar_bloque(char* nombre_nodo_origen, int bloque_origen, char* nombre_nodo_destino, int bloque_destino);

void mongo_delete(char* key, char* value, char* collection_name);

void mongo_update_string(char* key_to_search, char* value_to_search, char* key_to_update, char* value_to_update,
		char* collection_name);

void mongo_update_integer(char* key_to_search, char* value_to_search, char* key_to_update, int value_to_update,
		char* collection_name);

void mongo_update_long(char* key_to_search, char* value_to_search, char* key_to_update, long value_to_update,
		char* collection_name);

void mongo_insert(char* key, char* value, char* collection_name);

long mongo_get_long(char* key, char* compare_value, char* key_result, char* collection_name);

int mongo_get_integer(char* key, char* compare_value, char* key_result, char* collection_name);

#endif /* ABM_H_ */
