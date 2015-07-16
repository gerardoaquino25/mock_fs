#include "ABM.h"

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

void prueba_copia_archivo() {
	alta_nodo("A");
	alta_nodo("B");
	alta_nodo("C");

	copiar_archivo_a_mdfs(ARCHIVO_NUEVO, "/home/utnso/Escritorio/");

	copiar_archivo_a_local(ARCHIVO_NUEVO, "/home/utnso/Escritorio/Pruebas");
}

void* consola() {
	char * word = string_new();
	while (1) {
		char word_aux[256];

		txt_write_in_stdout("Ingrese una instruccion:\n");
		/* the leading space before the %c ignores space characters in the input */
		fgets(word_aux, sizeof(word_aux), stdin);

		word = string_duplicate(word_aux);
		string_trim(&word);
		strcpy(word, string_substring(word, 0, string_length(word)));

		char** comandos = string_split(word, " ");

		int i = 0;
		while (comandos[i] != NULL) {
			i++;
		}

		if (strcmp(comandos[0], "start") == 0) {
			if (strcmp(comandos[1], "nodo") == 0) {
				alta_nodo(comandos[2]);
			}
		}

		if (strcmp(comandos[0], "stop") == 0) {
			if (strcmp(comandos[1], "nodo") == 0) {
				baja_nodo(comandos[2]);
			}
		}

		if (strcmp(comandos[0], "copy") == 0) {
			if (strcmp(comandos[1], "file") == 0) {
				copiar_archivo_a_mdfs(comandos[2], comandos[3]);
			}
		}

		if (strcmp(comandos[0], "export") == 0) {
			if (strcmp(comandos[1], "file") == 0) {
				copiar_archivo_a_local(comandos[2], comandos[3]);
			}
		}

		if (strcmp(comandos[0], "create") == 0) {
			if (strcmp(comandos[1], "dir") == 0) {
				if (comandos[2] != NULL) {
					crear_directorio(comandos[3], comandos[2]);
				}
			}
		}

		if (strcmp(comandos[0], "rename") == 0) {
			if (strcmp(comandos[1], "dir") == 0) {
				if (comandos[2] != NULL) {
					renombrar_directorio(comandos[3], comandos[2]);
				}
			}
		}

		if (strcmp(comandos[0], "get") == 0) {
			if (strcmp(comandos[1], "file") == 0) {
				if (strcmp(comandos[2], "bloque") == 0) {
					ver_bloque(comandos[3], comandos[4]);
				}
			}
		}

		if (strcmp(comandos[0], "ls") == 0) {

			if (comandos[1] == NULL) {
				listar_todo();
			} else {

				if (strcmp(comandos[1], "dirs") == 0) {
					listar_directorios();
				}

				if (strcmp(comandos[1], "files") == 0) {
					listar_archivos();
				}

				if (strcmp(comandos[1], "file") == 0) {
					if (strcmp(comandos[2], "blocks") == 0) {
						listar_bloques_archivo(comandos[3]);
					}
				}

				if (strcmp(comandos[1], "nodos") == 0) {
					listar_nodos();
				}
			}
		}

		if (strcmp(comandos[0], "exit") == 0) {
			exit(1);
		}
	}
}

void iniciar() {
	contador_archivos = 0;
	contador_nodo = 0;
	archivos = list_create();
	nodos = list_create();
	bloques_nodos_archivos = dictionary_create();
	bloques_nodo_disponible = dictionary_create();
	directorios = malloc(sizeof(t_directorios_self));
	directorios->directorios = list_create();
	directorios->contador = 0;
}

int main(int argc, char *argv[]) {
	txt_write_in_stdout("Bienvenido!\n");

	iniciar();

	consola();
}
