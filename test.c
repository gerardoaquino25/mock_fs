#include "ABM.h"

typedef struct readThreadParams {
    char* nombre;
    int escritura;
} t_auxiliar;

void prueba_ABM_directorio() {
	txt_write_in_stdout("CREACION DE CARPETAS\n");
	crear_directorio("", "hola");
	crear_directorio("", "chau");
	crear_directorio("/hola/como", "estas");
	crear_directorio("/hola/como", "va");
	crear_directorio("/hola/como", "andas");
	crear_directorio("/hola/como", "caminas");
	crear_directorio("/hola/como/estas", "vos");
	crear_directorio("/hola/como/estas", "usted");
	crear_directorio("/hola/como/estas", "chabon");
	crear_directorio("/hola/como/andas", "vos");
	crear_directorio("/hola/como/andas", "usted");
	crear_directorio("/hola/como/andas", "chabon");
	crear_directorio("/chau", "hasta");
	crear_directorio("/eso/es", "nuevo");
	listar_directorios();

	txt_write_in_stdout("\n");

	txt_write_in_stdout("RENOMBRAMIENTO DE CARPETAS\n");
	renombrar_directorio("/hola", "holaAAA");
	renombrar_directorio("/chau", "chauAAA");
	renombrar_directorio("/holaAAA/como/estas", "estasAAA");
	renombrar_directorio("/chauAAA/hasta", "hastaAAA");
	renombrar_directorio("/eso/es/nuevo", "nuevoAAA");
	listar_directorios();

	txt_write_in_stdout("\n");

	txt_write_in_stdout("MOVER CARPETAS\n");
	mover_directorio("/holaAAA", "/hola");
	listar_directorios();
	txt_write_in_stdout("\n");
	mover_directorio("/chauAAA", "/chau");
	listar_directorios();
	txt_write_in_stdout("\n");
	mover_directorio("/hola/holaAAA/como/estasAAA", "/hola/holaAAA/como/estas");
	listar_directorios();
	txt_write_in_stdout("\n");
	mover_directorio("/chau/chauAAA/hastaAAA", "/chau/chauAAA/hasta");
	listar_directorios();
	txt_write_in_stdout("\n");
	mover_directorio("/eso/es/nuevoAAA", "/eso/es/nuevo");
	listar_directorios();

	txt_write_in_stdout("\n");

	txt_write_in_stdout("VOLVER CARPETAS\n");
	mover_directorio("/hola/holaAAA", " ");
	listar_directorios();
	txt_write_in_stdout("\n");
	mover_directorio("/chau/chauAAA", "/");
	listar_directorios();
	txt_write_in_stdout("\n");
	mover_directorio("/holaAAA/como/estas/estasAAA", "/holaAAA/como");
	listar_directorios();
	txt_write_in_stdout("\n");
	mover_directorio("/chauAAA/hasta/hastaAAA", "/chauAAA");
	listar_directorios();
	txt_write_in_stdout("\n");
	mover_directorio("/eso/es/nuevo/nuevoAAA", "/eso/es");
	listar_directorios();

	txt_write_in_stdout("MOVER CARPETAS\n");
	mover_directorio("/holaAAA", "/chauAAA");
	listar_directorios();
	mover_directorio("/chauAAA", "/chauAAA/como/estasAAA");
	listar_directorios();

	txt_write_in_stdout("\n");
}

void bloquear_archivo2(void *context){
	t_auxiliar *readParams = context;
	bloquear_archivo(readParams->nombre, readParams->escritura);
}

void desbloquear_archivo2(void *context){
	t_auxiliar *readParams = context;
	desbloquear_archivo(readParams->nombre, readParams->escritura);
}

void prueba_copia_archivo() {
	alta_nodo("A");
	alta_nodo("B");
	alta_nodo("C");

	copiar_archivo_a_mdfs(ARCHIVO_NUEVO, "/home/utnso/Escritorio/");

	copiar_archivo_a_local(ARCHIVO_NUEVO, "/home/utnso/Escritorio/Pruebas");
}

int es_ruta(char* ruta){
	if (ruta == NULL)
		return 0;

	if(string_length(ruta) < 1)
		return 0;

	return string_starts_with(ruta, "/") == true;
}

char* get_ruta(char * ruta){

}

void* consola() {
	char * word = string_new();
	char * ruta = string_new();
	char * ruta_aux = string_new();
	string_append(&ruta, "/");

	while (1) {
		char word_aux[256];

		txt_write_in_stdout("Ingrese una instruccion:\n");
		fgets(word_aux, sizeof(word_aux), stdin);

		word = string_duplicate(word_aux);
		string_trim(&word);
		strcpy(word, string_substring(word, 0, string_length(word)));

		char** comandos = string_split(word, " ");

		int i = 0;
		while (comandos[i] != NULL) {
			i++;
		}

		if (strcmp(comandos[0], "dir") == 0) {
			txt_write_in_stdout(ruta);
			txt_write_in_stdout("\n");
		}

		if (strcmp(comandos[0], "cd") == 0) {
			if (comandos[1] != NULL) {
				if (strcmp(comandos[1], "..") == 0) {
					if (strcmp(ruta, "/") != 0) {
						char** split_ruta = string_split(ruta, "/");
						int j = 0;
						while (split_ruta[j] != NULL) {
							j++;
						}
						int lenght = string_length(ruta) - string_length(split_ruta[j - 1]);

						ruta = string_substring(ruta, 0, lenght);
					}
				} else {
					if (es_ruta(comandos[1])) {
						ruta = string_duplicate(comandos[1]);
					} else {
						ruta_aux = string_new();
						ruta_aux = string_duplicate(ruta);
						string_append(&ruta_aux, comandos[1]);
						string_append(&ruta_aux, "/");

						if (get_directorio_by_ruta(ruta_aux) != NULL)
							ruta = string_duplicate(ruta_aux);
						else
							txt_write_in_stdout("No existe la ruta indicada.\n");
					}
				}
			}
		}

		if (strcmp(comandos[0], "start") == 0) {
			if (strcmp(comandos[1], "nodo") == 0) {
				if (comandos[2] != NULL) {
					alta_nodo(comandos[2]);
				} else {
					txt_write_in_stdout("Debe indicar un nombre para el nodo.\n");
				}
			}
		}

		if (strcmp(comandos[0], "stop") == 0) {
			if (strcmp(comandos[1], "nodo") == 0) {
				if (comandos[2] != NULL) {
					baja_nodo(comandos[2]);
				} else {
					txt_write_in_stdout("Debe indicar un nombre para el nodo.\n");
				}
			}
		}

		if (strcmp(comandos[0], "copy") == 0) {
			if (strcmp(comandos[1], "file") == 0)
				if (comandos[2] != NULL)
					copiar_archivo_a_mdfs(comandos[2], comandos[3]);

			if (strcmp(comandos[1], "bloque") == 0)
				if (comandos[2] != NULL && comandos[3])
					copiar_bloque(comandos[2], (int) strtol(comandos[3], (char **) NULL, 10), comandos[4],
							(int) strtol(comandos[5], (char **) NULL, 10));
		}

		if (strcmp(comandos[0], "export") == 0)
			if (strcmp(comandos[1], "file") == 0)
				if (comandos[2] != NULL)
					copiar_archivo_a_local(comandos[2], comandos[3]);

		if (strcmp(comandos[0], "create") == 0)
			if (strcmp(comandos[1], "dir") == 0)
				if (comandos[2] != NULL)
					crear_directorio(comandos[3], comandos[2]);

		if (strcmp(comandos[0], "rename") == 0)
			if (strcmp(comandos[1], "dir") == 0)
				if (comandos[2] != NULL)
					renombrar_directorio(comandos[3], comandos[2]);

		if (strcmp(comandos[0], "get") == 0)
			if (strcmp(comandos[1], "bloque") == 0)
				if (comandos[2] != NULL && comandos[3] != NULL)
					ver_bloque(comandos[2], (int) strtol(comandos[3], (char **) NULL, 10));

		if (strcmp(comandos[0], "delete") == 0)
			if (strcmp(comandos[1], "bloque") == 0) {
				if (comandos[2] != NULL && comandos[3] != NULL)
					borrar_bloque(comandos[2], (int) strtol(comandos[3], (char **) NULL, 10));
			} else if (strcmp(comandos[1], "dir") == 0) {
				if (comandos[2] != NULL)
					eliminar_directorio(comandos[2]);
			}

		if (strcmp(comandos[0], "ls") == 0) {
			if (comandos[1] == NULL)
				listar_todo();
			else {

				if (strcmp(comandos[1], "dirs") == 0)
					listar_directorios();

				if (strcmp(comandos[1], "files") == 0)
					listar_archivos();

				if (strcmp(comandos[1], "bloques") == 0)
					if (comandos[2] != NULL)
						listar_bloques_archivo(comandos[2]);

				if (strcmp(comandos[1], "nodos") == 0)
					listar_nodos();
			}
		}

		if (strcmp(comandos[0], "agregar") == 0) {
			if (comandos[1] != NULL) {
				agregar_semaforo_archivo(comandos[1]);
			}
		}

		if (strcmp(comandos[0], "bloquear") == 0) {
			if (comandos[1] != NULL) {
				pthread_t hilo_uno;
				pthread_t hilo_dos;
				int escritura = comandos[2] != NULL;
				t_auxiliar readParams;
			    readParams.nombre = string_duplicate(comandos[1]);
			    readParams.escritura = escritura;

				pthread_create(&hilo_uno, NULL, (void*) bloquear_archivo2, &readParams);
			}
		}

		if (strcmp(comandos[0], "desbloquear") == 0){
			if (comandos[1] != NULL) {
				pthread_t hilo_uno;
				int escritura = comandos[2] != NULL;
				t_auxiliar readParams;
			    readParams.nombre = comandos[1];
			    readParams.escritura = escritura;

				pthread_create(&hilo_uno, NULL, (void*) desbloquear_archivo2, &readParams);
			}
		}

		if (strcmp(comandos[0], "exit") == 0)
			exit(1);
	}
}

void iniciar() {
	semaforos_archivo = malloc(sizeof(t_control_self));
	semaforos_archivo->lectura = dictionary_create();
	semaforos_archivo->escritura = dictionary_create();
	semaforos_archivo->intermedio = dictionary_create();

	semaforos_bloque = malloc(sizeof(t_control_self));
	semaforos_bloque->lectura = dictionary_create();
	semaforos_bloque->escritura = dictionary_create();
	semaforos_bloque->intermedio = dictionary_create();

	semaforos_nodo = malloc(sizeof(t_control_self));
	semaforos_nodo->lectura = dictionary_create();
	semaforos_nodo->escritura = dictionary_create();
	semaforos_nodo->intermedio = dictionary_create();

	directorios = malloc(sizeof(t_directorios_self));
	directorios->directorios = list_create();
	directorios->contador = 0;
	directorios->contador_directorios = 0;

	archivos = list_create();
	nodos = list_create();
	bloques_nodos_archivos = dictionary_create();
	bloques_nodo_disponible = dictionary_create();

	contador_archivos = 0;
	contador_nodo = 0;
}

void inicio_mock(){
//	alta_nodo("J");
//	alta_nodo("H");
//	alta_nodo("L");
//
//	copiar_archivo_a_mdfs("/home/utnso/Escritorio/Nuevo.txt", "/prueba");
//	listar_bloques_archivo("/prueba/Nuevo.txt");
//
//	copiar_bloque("H", 1, "L", 2);
//	listar_bloques_archivo("/prueba/Nuevo.txt");
//
//	copiar_archivo_a_mdfs("/home/utnso/Escritorio/Nuevo.txt", "/prueba/hola");
//	listar_bloques_archivo("/prueba/hola/Nuevo.txt");
//
//	copiar_bloque("L", 1, "L", 3);
//	listar_bloques_archivo("/prueba/Nuevo.txt");
//	listar_bloques_archivo("/prueba/hola/Nuevo.txt");
//
//	borrar_bloque("J", 2);
//	listar_bloques_archivo("/prueba/Nuevo.txt");
//
//	baja_nodo("H");

	crear_directorio(
			"/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola1/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola2/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola3/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola4/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola5/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola6/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola7/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola8/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola9/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola10/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola11/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola12/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola13/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola14/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola15/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola16/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola17/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola18/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
	crear_directorio(
			"/hola19/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola/hola", "hola");
}



int main(int argc, char *argv[]) {
	txt_write_in_stdout("Bienvenido!\n");
	iniciar();
//	inicio_mock();
	consola();
}
