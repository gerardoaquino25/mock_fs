#include "ABM.h"

#define filename "/home/utnso/Escritorio/Nuevo2.txt"

#define VALIDO 1
#define INVALIDO 0

/*
 * Busca un t_archivo en una lista de archivos.
 */
t_archivo_self* find_archivo(char* nombre, t_list* archivosAux) {
	int i;
	for (i = 0; i < list_size(archivosAux); i += 1) {
		t_archivo_self * archivo = list_get(archivosAux, i);
		if (strcasecmp(archivo->nombre, nombre) == 0) {
			return archivo;
		}
	}
	return NULL;
}

char* generate_unique_id_to_directorio() {
	directorios->contador++;
	return string_itoa(directorios->contador);
}

char* generate_unique_id_to_archivo() {
	directorios->contador++;
	return string_itoa(directorios->contador);
}

/*
 * Busca un t_archivo en una lista de archivos.
 */
t_archivo_self* find_archivo_by_id(char* archivo_id, t_list* archivosAux) {
	int i;
	for (i = 0; i < list_size(archivosAux); i += 1) {
		t_archivo_self * archivo = list_get(archivosAux, i);
		if (strcasecmp(archivo->id, archivo_id) == 0) {
			return archivo;
		}
	}
	return NULL;
}

/*
 * Busca un t_nodo_tp que sea el que mas espacio disponible tenga en la lista de nodos.
 * TODO: Falta verificar que una copia del mismo bloque de archivo no este en ese nodo.
 */
t_nodo_self* find_nodo_disponible(char* nombre, int numero_bloque1) {
	int espacio_disponible = 0;
	int i;
	t_nodo_self* selecto = NULL;
	for (i = 0; i < list_size(nodos); i++) {
		t_nodo_self * nodo = list_get(nodos, i);
		if (nodo->bloques_disponibles > espacio_disponible
				&& nodo->estado == 1) {
			espacio_disponible = nodo->bloques_disponibles;
			selecto = nodo;
		}
	}

	return selecto;
}

/*
 * Devuelve que bloque de nodo esta disponible y lo quita de la lista
 * de nodos disponibles.
 */
int get_bloque_disponible(t_nodo_self* nodo) {
	t_list* lista = ((t_list*) dictionary_get(bloques_nodo_disponible,
			nodo->nombre));

	if (list_size(lista)) {
		int disponible = list_get(lista, 0);
		list_remove(lista, 0);
		return disponible;
	}

	return -1;
}

/*
 * Busca un t_nodo_tp en la lista de nodos.
 */
t_nodo_self* find_nodo(char* nombre) {
	int i;
	for (i = 0; i < list_size(nodos); i += 1) {
		t_nodo_self * nodo = list_get(nodos, i);
		if (nodo->nombre == nombre) {
			return nodo;
		}
	}
	return NULL;
}

/*
 *Agrega un t_nodo_tp nuevo a la lista de nodos.
 */
void add_nodo_to_nodos(t_nodo_self* nodo) {
	list_add(nodos, nodo);
	iniciar_disponibles(nodo->nombre);
	t_list* bloque_archivo_control = list_create();
	dictionary_put(bloques_nodos_archivos, nodo->nombre,
			bloque_archivo_control);
}

/*
 * Inicializa la lista de control de bloques por nodo.
 */
void iniciar_disponibles(char* nombre) {
	int i;
	t_list * lista = list_create();
	for (i = 1; i <= CANTIDAD_BLOQUES_NODO_DEFAULT; i++) {
		int* numero;
		numero = malloc(sizeof(int));
		numero = (int) i;
		list_add(lista, numero);
	}

	dictionary_put(bloques_nodo_disponible, nombre, lista);
}

/*
 * Setea como NO disponible a un archivo.
 */
t_archivo_self* archivo_no_disponible(t_archivo_self* archivo) {
	return archivo->estado == INVALIDO;
}

//t_list* ArchivosNoDisponibles(t_list* archivosAux) {
//	t_list* noDisponibles = list_create();
//	int i;
//	for (i = 0; i < list_size(archivosAux); i++) {
//		if (ArchivoNoDisponible(list_get(archivosAux, i))) {
//			list_add(noDisponibles);
//		}
//	}
//	return noDisponibles;
//}

/*
 * Valida si un archivo que estaba inactivado, pasa a estar activo
 * gracias a la validacion de una copia de un bloque dado.
 */
void chequear_alta_archivo(t_archivo_self* archivo, int numero_bloque) {
	if (archivo->estado == INVALIDO) {
		t_bloque_archivo_self* bloque = dictionary_get(archivo->bloques,
				string_itoa(numero_bloque));

		if (bloque->estado == INVALIDO) {
			t_list* copias = bloque->copias;

			int i;
			for (i = 0; i < list_size(copias); i++) {
				t_copia_self* copia = list_get(copias, i);
				if (copia->nodo_bloque->nodo->estado == VALIDO) {
					bloque->estado = VALIDO;
					archivo->bloques_invalidos -= 1;
					break;
				}
			}

			if (archivo->bloques_invalidos == archivo->cantidad_bloques) {
				archivo->estado = VALIDO;
			}
		}
	}
}

/*
 * Activa el nodo y busca los archivos utilizados por el nodo reactivado y
 * si estan invalidos verifica si deben ser dados de alta.
 */
void reactivar_nodo(t_nodo_self* nodo) {
	if (nodo != NULL) {
		int i;
		nodo->estado = 1;
		t_list* archivos_nodo = dictionary_get(bloques_nodos_archivos,
				nodo->nombre);

		for (i = 0; i < list_size(archivos_nodo); i++) {
			t_bloque_archivo_control_self* bloque_archivo_control = list_get(
					archivos_nodo, i);
			t_archivo_self * archivo = find_archivo_by_id(
					bloque_archivo_control->archivo_id, archivos);
			chequear_alta_archivo(archivo,
					bloque_archivo_control->archivo_bloque);
		}
	}
}

/*
 * Agrega o reactiva un nodo que ingreso al FS.
 */
void alta_nodo(char* nombre) {
	t_nodo_self* nodoAux = find_nodo(nombre);
	if (nodoAux == NULL) {
		t_nodo_self* nodo;
		nodo = malloc(sizeof(t_nodo_self));
		nodo->estado = 1;
		nodo->nombre = nombre;
		nodo->bloques_disponibles = CANTIDAD_BLOQUES_NODO_DEFAULT;
		add_nodo_to_nodos(nodo);
	} else {
		reactivar_nodo(nodoAux);
	}
}

/*
 * Dado un archivo y un bloque determinados, verifica si el bloque es invalido
 * y lo setea. En el caso de que lo sea, invalida tambien al archivo.
 */
void chequear_baja_archivo(t_archivo_self* archivo, int archivo_bloque) {
	t_bloque_archivo_self* bloque = dictionary_get(archivo->bloques,
			string_itoa(archivo_bloque));

	if (bloque->estado == VALIDO) {
		int i;
		t_list* copias = bloque->copias;
		int desactivados = 0;

		for (i = 0; i < list_size(copias); i++) {
			t_copia_self* copia = list_get(copias, i);
			if (copia->nodo_bloque->nodo->estado == INVALIDO) {
				desactivados++;
			}
		}

		bloque->estado = desactivados != list_size(copias)
				&& list_size(copias) != 0;

		if (bloque->estado == INVALIDO) {
			archivo->bloques_invalidos += 1;
			archivo->estado = INVALIDO;
		}
	}
}

/*
 * Desactiva un nodo.
 */
void baja_nodo(char* nombre) {
	t_nodo_self* nodo = find_nodo(nombre);
	if (nodo != NULL) {
		int i;
		nodo->estado = 0;
		t_list* archivos_nodo = dictionary_get(bloques_nodos_archivos, nombre);

		if (archivos_nodo != NULL) {
			for (i = 0; i < list_size(archivos_nodo); i++) {
				t_bloque_archivo_control_self* bloque_archivo_control =
						list_get(archivos_nodo, i);
				t_archivo_self * archivo = find_archivo_by_id(
						bloque_archivo_control->archivo_id, archivos);
				chequear_baja_archivo(archivo,
						bloque_archivo_control->archivo_bloque);
			}
		}
	}
}

char* create_directorio(char* nombre, int nivel, char* padre, char* ruta){
	t_directorio_self* directorio;
	directorio = malloc(sizeof(t_directorio_self));
	directorio->nombre = string_duplicate(nombre);
	directorio->padre = string_duplicate(padre);
	directorio->nivel = nivel;
	directorio->ruta = string_duplicate(ruta);
	directorio->id = generate_unique_id_to_directorio();
	list_add(directorios->directorios, directorio);

	return directorio->id;
}

char * get_directorio_numero(char* nombre, int nivel, char* padre) {
	int i;
	char* res = "-1";
	for (i = 0; i < list_size(directorios->directorios); i++) {
		t_directorio_self* directorio = list_get(directorios->directorios, i);

		if (directorio->nivel == nivel) {
			if (strcasecmp(directorio->padre, padre) == 0){
				if(strcasecmp(directorio->nombre, nombre) == 0)
					return directorio->id;
			}
		}
	}

	return res;
}

char* set_directorio(char* ruta, int extension) {
	int i = 0;
	int encontrado = 1;
	char* padre = "0";
	char** directorios_aux = string_split(ruta, "/");
	int sum = extension ? 1 : 0;
	char* ruta_aux = string_new();

	while (1) {
		if (directorios_aux[i + sum] != NULL) {
			string_append(&ruta_aux, "/");
			string_append(&ruta_aux, directorios_aux[i]);
			if (encontrado) {
				char* padre_aux = get_directorio_numero(directorios_aux[i], i, padre);
				if (!(strcasecmp(padre_aux, "-1") == 0)) {
					padre = padre_aux;
				} else {
					encontrado = 0;
					padre = create_directorio(directorios_aux[i], i, padre, ruta_aux);
				}
			} else {
				padre = create_directorio(directorios_aux[i], i, padre, ruta_aux);
			}
			i++;
		}else{
			break;
		}
	}

	return padre;
}

/*
 * Crea un archivo nuevo.
 */
t_archivo_self* create_archivo(int tamanio, char* nombre) {
	t_archivo_self* archivo;
	archivo = malloc(sizeof(t_archivo_self));
	archivo->estado = 0;
	archivo->bloques = dictionary_create();
	archivo->tamanio = tamanio;
	archivo->nombre = nombre;
	archivo->bloques_invalidos = 0;
	archivo->id = generate_unique_id_to_archivo();
	archivo->ruta_id = set_directorio(nombre, true);

	return archivo;
}

/*
 * Crea un bloque nuevo.
 */
t_bloque_archivo_self* create_bloque(int numero) {
	t_bloque_archivo_self* bloque;
	bloque = malloc(sizeof(t_bloque_archivo_self));
	bloque->copias = list_create();
	bloque->estado = 1;
	bloque->numero = numero;

	return bloque;
}

/*
 * Crea una copia de un bloque determinado de un archivo y lo asigna a un bloque de un nodo.
 */
t_copia_self* create_copia(int bloque, t_nodo_self* nodo, int archivo_id) {
	t_copia_self* copia;
	copia = malloc(sizeof(t_copia_self));
	copia->archivo_bloque = bloque;
	asignar_bloque_nodo_a_copia(copia, nodo);

	// Agrega el nodo_bloque usado a la lista de control.
	t_bloque_archivo_control_self* control;
	control = malloc(sizeof(t_bloque_archivo_control_self));
	control->archivo_bloque = 1;
	control->archivo_id = archivo_id;
	t_list* lista = dictionary_get(bloques_nodos_archivos, nodo->nombre);
	list_add(lista, control);

	return copia;
}

/*
 * Asigna un bloque de t_nodo_tp a una t_copia.
 */
void asignar_bloque_nodo_a_copia(t_copia_self* copia, t_nodo_self* nodo) {
	t_nodo_bloque_self *nodo_bloque;
	nodo_bloque->nodo = nodo;
	nodo_bloque->bloque_nodo = get_bloque_disponible(nodo);
	copia->nodo_bloque = nodo_bloque;
	nodo->bloques_disponibles -= 1;
}

/*
 * Asigna una t_copia a las copias de un t_bloque_archivo.
 */
void asignar_copia_a_bloque_archivo(t_bloque_archivo_self* bloque,
		t_copia_self* copia) {
	list_add(bloque->copias, copia);
}

void asignar_bloque_archivo_a_archivo(t_archivo_self* archivo,
		t_bloque_archivo_self* bloque) {
	dictionary_put(archivo->bloques, string_itoa(bloque->numero), bloque);
}

void add_archivo_to_archivos(t_archivo_self* archivo) {
	list_add(archivos, archivo);
}

t_archivo_datos_self* dividir_archivo(char* nombre) {
	t_archivo_datos_self* archivo;
	archivo = malloc(sizeof(archivo));

	FILE *file;
	long filelen;
	int i;
	archivo->bloques = list_create();
	int max = 20000000;
	int maxRead = max - sizeof(char);

	file = fopen(nombre, "rb");
	if (file == NULL){
		printf("No se encontro el archivo.");
		exit(1);
	}

	fseek(file, 0, SEEK_END);
	filelen = ftell(file);
	archivo->tamanio = filelen;
	rewind(file);

	int cantidadBloques = filelen / maxRead;
	int resto = filelen % maxRead;

	for (i = 0; i < cantidadBloques; i++) {
		char* segmento;
		segmento = (char *) malloc((max) * sizeof(char));
		fread(*&segmento, maxRead, 1, file);
		segmento = strcat(segmento, "\0");
		list_add(archivo->bloques, segmento);
	}

	if (resto != 0) {
		char* segmento;
		segmento = malloc(max);
		fread(*&segmento, max, 1, file);

		for (i = 0; i < resto; i++) {
			segmento = strcat(segmento, "\0");
		}
		list_add(archivo->bloques, segmento);
	}

	fclose(file);

	return archivo;
}

void borrado_fisico_nodo(char* nodo){
	//TODO: Solicitud de borrado de informacion de nodo.
}

void formartear_mdfs() {
	//TODO: Test.
	int i, j;
	for (i = 0; i < list_size(nodos); i++) {
		t_nodo_self* nodo = list_get(nodos, i);
		borrado_fisico_nodo(nodo->nombre);
		t_list * bna = dictionary_get(bloques_nodos_archivos, nodo->nombre);
		t_list * bnd = dictionary_get(bloques_nodo_disponible, nodo->nombre);

		list_clean(bna);
		list_clean(bnd);

		iniciar_disponibles(nodo->nombre);
	}

	for (i = 0; i < list_size(archivos); i++) {
		t_archivo_self* archivo = list_get(archivos, i);
		t_list* bloques = archivo->bloques;
		for (i = 0; j < list_size(bloques); j++) {
			t_bloque_archivo_self* bloque = list_get(bloques, i);
			list_clean(bloque->copias);
		}
		list_clean(bloques);
	}
	list_clean(archivos);

	list_clean(directorios->directorios);
}

void mover_archivo(char* nombre, char* destino){
	t_archivo_self* archivo = find_archivo(nombre, archivos);
	archivo->ruta_id = set_directorio(destino, true);
}

void renombrar_archivo(char * ruta, char* nombre){
	t_archivo_self* archivo = find_archivo(ruta, archivos);
	int i = 0;
	char** ruta_aux = string_split(archivo->nombre, "/");
	char* destino = "/";

	while(ruta_aux[i + 1] != NULL){
		string_append(&destino, ruta_aux[i]);
		string_append(&destino, "/");
		i++;
	}

	string_append(&destino, nombre);
	archivo->nombre = set_directorio(destino, true);
}

void eliminar_archivo(char * ruta) {
	int indice, i, j, k;
	t_archivo_self * archivo = NULL;
	for (indice = 0; indice < list_size(archivos); indice += 1) {
		t_archivo_self * archivo_aux = list_get(archivos, indice);
		if (strcasecmp(archivo_aux->nombre, ruta) == 0) {
			t_archivo_self * archivo = archivo_aux;
			break;
		}
	}

	if (archivo != NULL) {
		for (i = 0; i < dictionary_size(archivo->bloques); i++) {
			t_bloque_archivo_self* bloque = dictionary_get(archivo->bloques,
					string_itoa(i));
			for (j = 0; j < dictionary_size(bloque->copias); j++) {
				t_copia_self* copia = list_get(bloque->copias, i);
				t_nodo_bloque_self* bloque_nodo = copia->nodo_bloque;
				t_nodo_self * nodo = bloque_nodo->nodo;
				//TODO: Si el nodo no esta disponible, no se puede borrar los datos.
				nodo->bloques_disponibles++;
				dictionary_put(bloques_nodo_disponible, nodo->nombre,
						bloque_nodo->bloque_nodo);

				t_list* archivos_nodo = dictionary_get(bloques_nodos_archivos,
						nodo->nombre);

				for (k = list_size(archivos_nodo) - 1; k >= 0; k--) {
					t_bloque_archivo_control_self* bloque_archivo_control =
							list_get(archivos_nodo, i);
					if (strcasecmp(
							bloque_archivo_control->archivo_id, archivo->id) == 0)
						list_remove(archivos_nodo, k);
				}

			}
			list_destroy(bloque->copias);
		}

		list_remove(archivos, indice);
	}
}

void crear_directorio(char* ruta, char* nombre){
	char * destino = string_new();
	if (ruta != NULL)
		string_append(&destino, ruta);

	string_append(&destino,"/");
	string_append(&destino,nombre);
	set_directorio(destino, false);
}

t_directorio_self* get_directorio_by_id(char* archivo_id){
	int i;
	for (i = 0; i < list_size(directorios->directorios); i++){
		t_directorio_self* directorio_aux = list_get(directorios->directorios, i);
		if (strcasecmp(archivo_id, directorio_aux->id) == 0)
			return directorio_aux;
	}

	return NULL;
}

char * get_ruta_directorio(t_directorio_self* directorio){
	char* padre = directorio->padre;
	char* ultimo = strdup("0");
	char* ruta = string_new();
	string_append(&ruta, "/");
	string_append(&ruta, directorio->nombre);

	while(strcasecmp(padre, ultimo) != 0){
		t_directorio_self* directorio_aux = get_directorio_by_id(padre);
		char* ruta_aux = strdup("/");
		string_append(&ruta_aux, directorio_aux->nombre);
		string_append(&ruta_aux, ruta);
		ruta = string_duplicate(ruta_aux);
		padre = string_duplicate(directorio_aux->padre);
	}

	return ruta;
}

t_directorio_self* get_directorio_by_ruta(char* ruta){
	char** ruta_aux = string_split(ruta, "/");
	int i = 0;
	int j;
	char * padre = string_new();
	string_append(&padre, "0");
	t_directorio_self * resultado;
	resultado = malloc(sizeof(t_directorio_self));

	while(ruta_aux[i] != NULL){
		resultado = NULL;
		for (j = 0; j < list_size(directorios->directorios); j++) {
			t_directorio_self * directorio = list_get(directorios->directorios,
					j);
			if(strcasecmp(directorio->nombre, ruta_aux[i]) == 0){
				if (directorio->nivel == i) {
					if (strcasecmp(directorio->padre, padre) == 0){
						padre = directorio->id;
						resultado = list_get(directorios->directorios,j);
						break;
					}
				}
			}
		}
		i++;
	}

	return resultado;
}

void renombrar_directorio(char* ruta, char* nombre){
	char* ruta_aux = string_new();

	if(strncmp(ruta, "/", 1) == 0){

	}else{
		string_append(&ruta_aux, "/");
	}
	string_append(&ruta_aux, ruta);

	t_directorio_self* directorio = get_directorio_by_ruta(ruta_aux);
	directorio->nombre = nombre;

	char** directorios_aux = string_split(ruta_aux, "/");
	int i = 0;
	char * ruta_final = string_new();

	while(i < directorio->nivel){
		string_append(&ruta_final, "/");
		string_append(&ruta_final, directorios_aux[i]);
		i++;
	}

	string_append(&ruta_final, "/");
	string_append(&ruta_final, nombre);
	directorio->ruta = ruta_final;

	free(ruta_aux);
	free(directorios_aux);
}

char* get_nombre_archivo(char* ruta) {
	char** directorio = string_split(ruta, "/");
	int i = 0;

	while (directorio[i + 1] != NULL)
		i++;

	return directorio[i];
}

void copiar_archivo_a_mdfs(char* ruta, char* destino_aux) {
	t_archivo_datos_self* archivo_datos = dividir_archivo(ruta);
	char * destino = string_new();
	string_append(&destino, destino_aux);

	//TEMPORAL
	datos = archivo_datos;
	//

	char * nombre_archivo = get_nombre_archivo(ruta);
	if(!string_ends_with(destino, "/"))
		string_append(&destino, "/");
	string_append(&destino, nombre_archivo);

	t_archivo_self* archivo = create_archivo(archivo_datos->tamanio, destino);

	int i;
	for (i = 0; i < list_size(archivo_datos->bloques); i++) {
		t_bloque_archivo_self* bloque = create_bloque(i);

		t_copia_self* copia1 = create_copia(i, find_nodo_disponible(destino, i),
				archivo->id);
		t_copia_self* copia2 = create_copia(i, find_nodo_disponible(destino, i),
				archivo->id);
		t_copia_self* copia3 = create_copia(i, find_nodo_disponible(destino, i),
				archivo->id);

		asignar_copia_a_bloque_archivo(bloque, copia1);
		asignar_copia_a_bloque_archivo(bloque, copia2);
		asignar_copia_a_bloque_archivo(bloque, copia3);

		asignar_bloque_archivo_a_archivo(archivo, bloque);
	}
	archivo->estado = VALIDO;

	add_archivo_to_archivos(archivo);

//	list_destroy(archivo_datos);
}

void listar_directorios() {
	int i;
	int j;
	int cola = list_size(directorios->directorios);
	int encontrado = false;

	for (j = 0; j < cola; j++) {
		for (i = 0; i < list_size(directorios->directorios); i++) {
			t_directorio_self * directorio = list_get(directorios->directorios,
					i);
			if (directorio->nivel == j) {
				txt_write_in_stdout(string_itoa(directorio->nivel));
				txt_write_in_stdout(": ");
				txt_write_in_stdout(directorio->id);
				txt_write_in_stdout(" ");
				txt_write_in_stdout(directorio->padre);
				txt_write_in_stdout(" ");
				txt_write_in_stdout(directorio->nombre);
				txt_write_in_stdout(": ");
				txt_write_in_stdout(get_ruta_directorio(directorio));
				txt_write_in_stdout(".\n");
				encontrado = true;
			}
		}
		if (encontrado)
			encontrado = false;
		else
			break;
	}
}

void eliminar_directorio(char* ruta){
	//TODO: Eliminar archivos contenidos en los directorios.
	t_directorio_self* directorio = get_directorio_by_ruta(ruta);
	t_list* directorios_a_eliminar = list_create();
	t_list* directorios_actuales = list_create();
	list_add(directorios_actuales, directorio);
	int i, j;

	for (i = list_size(directorios_actuales) - 1; i >= 0; i--) {
		t_directorio_self* directorio_padre = list_get(directorios_actuales, i);
		for (j = 0; j < list_size(directorios->directorios); j++) {
			t_directorio_self* directorio_aux = list_get(
					directorios->directorios, j);
			if (strcasecmp(directorio_aux->padre, directorio_padre->id) == 0) {
				list_add(directorios_a_eliminar, directorio_aux);
				list_add(directorios_actuales, directorio_aux);
			}
		}
		list_remove(directorios_actuales, i);
		i = list_size(directorios_actuales);
	}
	list_destroy(directorios_actuales);
	list_add(directorios_a_eliminar, directorio);

	for (i = 0; i < list_size(directorios_a_eliminar); i++) {
		t_directorio_self* directorio_a_eliminar = list_get(directorios_a_eliminar, i);
		for (j = 0; j < list_size(directorios->directorios); j++) {
			t_directorio_self* directorio_aux = list_get(
					directorios->directorios, j);
			if (strcasecmp(directorio_aux->id, directorio_a_eliminar->id) == 0) {
				list_remove(directorios->directorios, j);
				break;
			}
		}
	}

	list_destroy(directorios_a_eliminar);
}

int get_cantidad_niveles(char* ruta) {
	char* ruta_aux2 = string_duplicate(ruta);
	string_trim(&ruta_aux2);

	if (strcasecmp(ruta_aux2, "/") == 0 || string_is_empty(ruta_aux2))
		return 0;

	char** ruta_aux = string_split(ruta_aux2, "/");
	int contador = 0;

	while (ruta_aux[contador] != NULL)
		contador++;

	free(ruta_aux2);

	return contador;
}

int es_subdirectorio_by_ruta(char* ruta, char* destino){
	t_directorio_self* directorio_origen = get_directorio_by_ruta(ruta);
	t_directorio_self* directorio_destino;

	int cantidad_niveles_ruta = get_cantidad_niveles(ruta);
	int cantidad_niveles_destino = get_cantidad_niveles(destino);
	if (cantidad_niveles_destino > 0) {
		set_directorio(destino, false);
		directorio_destino = get_directorio_by_ruta(destino);
	}

	return es_subdirectorio_by_directorio(directorio_origen, directorio_destino);
}

int es_subdirectorio_by_directorio(t_directorio_self* ruta, t_directorio_self* destino){
	if(ruta == NULL || destino == NULL)
		return 0;

	if(strcasecmp(ruta->id, destino->id) == 0)
		return 1;

	if(strcasecmp(ruta->padre, "0") == 0 && strcasecmp(destino->padre, "0") == 0)
		return 0;

	if(strcasecmp(destino->padre, "0") == 0)
		return 0;

	if(ruta->nivel == destino->nivel)
		return 0;

	if(ruta->nivel > destino->nivel)
		return 0;

	t_directorio_self* directorio = get_directorio_by_id(destino->padre);

	while(strcasecmp(directorio->padre, "0") != 0){
		if(strcasecmp(ruta->id, directorio->padre) == 0)
			return 1;
		directorio = get_directorio_by_id(directorio->padre);
	}

	return 0;
}

void mover_directorio(char* ruta, char* destino){
	//TODO: Mover archivos contenidos en directorios.
	t_directorio_self* directorio_origen = get_directorio_by_ruta(ruta);
	t_directorio_self* directorio_destino = NULL;
	char* padre_final = strdup("0");
	int cantidad_niveles_ruta = get_cantidad_niveles(ruta);
	int cantidad_niveles_destino = get_cantidad_niveles(destino);
	if (cantidad_niveles_destino > 0) {
		set_directorio(destino, false);
		directorio_destino = get_directorio_by_ruta(destino);
		padre_final = directorio_destino->id;
	}

	if (!es_subdirectorio_by_directorio(directorio_origen, directorio_destino)
			&& strcasecmp(directorio_origen->padre, padre_final) != 0) {
		t_list* directorios_a_modificar = list_create();
		t_list* directorios_actuales = list_create();
		list_add(directorios_actuales, directorio_origen);
		int i, j;

		for (i = list_size(directorios_actuales) - 1; i >= 0; i--) {
			t_directorio_self* directorio_padre = list_get(directorios_actuales,
					i);
			for (j = 0; j < list_size(directorios->directorios); j++) {
				t_directorio_self* directorio_aux = list_get(
						directorios->directorios, j);
				if (strcasecmp(directorio_aux->padre, directorio_padre->id)
						== 0) {
					list_add(directorios_a_modificar, directorio_aux);
					list_add(directorios_actuales, directorio_aux);
				}
			}
			list_remove(directorios_actuales, i);
			i = list_size(directorios_actuales);
		}
		list_destroy(directorios_actuales);

		for (i = 0; i < list_size(directorios_a_modificar); i++) {
			t_directorio_self* directorio_a_eliminar = list_get(
					directorios_a_modificar, i);
			for (j = 0; j < list_size(directorios->directorios); j++) {
				t_directorio_self* directorio_aux = list_get(
						directorios->directorios, j);
				if (strcasecmp(directorio_aux->id, directorio_a_eliminar->id)
						== 0) {
					directorio_aux->nivel = directorio_aux->nivel
							+ (cantidad_niveles_destino - cantidad_niveles_ruta
									+ 1);
					break;
				}
			}
		}

		list_destroy(directorios_a_modificar);
		directorio_origen->nivel = directorio_origen->nivel
				+ (cantidad_niveles_destino - cantidad_niveles_ruta + 1);
		directorio_origen->padre = padre_final;
	}

}

int reconstruir_archivo(t_list* bloques, char * ruta, char* nombre) {
	//TODO: Reconstruir archivo.
	int i;
	char* ruta_aux = string_new();

	string_append(&ruta_aux, ruta);
	string_append(&ruta_aux, "/");
	string_append(&ruta_aux, nombre);

	FILE *f = fopen(ruta_aux, "w");
	if (f == NULL)
	{
	    printf("Error opening file!\n");
	    exit(1);
	}

	for(i = 0; i < list_size(bloques); i++){
		fprintf(f, list_get(bloques, i));
	}

	fclose(f);

	return 1;
}

char* get_data_from_nodo(char* nombre, int * bloque, int campo_temporal) {
	//TODO: Solicitar bloque a nodo.
	return list_get(datos->bloques, campo_temporal);
}

int copiar_archivo_a_local(char* nombre, char* ruta) {
	t_list* bloques_archivo = list_create();
	t_archivo_self * archivo = find_archivo(nombre, archivos);

	int i, j, k;
	int completo = true;

	if (archivo->estado == VALIDO) {
		for (i = 0; i < dictionary_size(archivo->bloques); i++) {
			t_bloque_archivo_self* bloque = dictionary_get(archivo->bloques,
					string_itoa(i));

			int encontrado = false;
			for (j = 0; j < dictionary_size(bloque->copias); j++) {
				t_copia_self* copia = list_get(bloque->copias, j);
				t_nodo_bloque_self* bloque_nodo = copia->nodo_bloque;
				t_nodo_self * nodo = bloque_nodo->nodo;
				if (nodo->estado == VALIDO) {
					list_add(bloques_archivo, get_data_from_nodo(nodo->nombre, bloque_nodo->bloque_nodo, i)); //TEMPORAL: Se va a agregar lo solicitado al nodo.
					encontrado = true;
					break;
				}
			}

			if (!encontrado) {
				completo = false;
				break;
			}
		}
	}

	if (completo) {
		reconstruir_archivo(bloques_archivo, ruta, get_nombre_archivo(archivo->nombre));
		list_destroy(bloques_archivo);
		return 1;
	} else {
		list_destroy(bloques_archivo);
		return 0;
	}
}

void get_MD5(char* nombre) {

}

void mover_bloque(){

}

void borrar_bloque(){

}

void copiar_bloque(){

}