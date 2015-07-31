#include "ABM.h"

void iniciar() {
	m_archivos_id = malloc(sizeof(sem_t));
	sem_init(m_archivos_id, 0, 1);
	m_control_directorios = malloc(sizeof(sem_t));
	sem_init(m_control_directorios, 0, 1);
	m_directorios_id = malloc(sizeof(sem_t));
	sem_init(m_directorios_id, 0, 1);
	m_nodos_id = malloc(sizeof(sem_t));
	sem_init(m_nodos_id, 0, 1);

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
	directorios->contador = mongo_get_long("id", "contadores", "contador_directorios", "global");
	directorios->contador_directorios = mongo_get_integer("id", "control", "control_directorios", "global");

	archivos = list_create();
	nodos = list_create();
	bloques_nodos_archivos = dictionary_create();
	bloques_nodo_disponible = dictionary_create();

	contador_archivos = mongo_get_long("id", "contadores", "contador_archivos", "global");
	contador_nodo = mongo_get_long("id", "contadores", "contador_nodos", "global");

	cargar_datos_prexistentes();
}

void bloquear_contador(sem_t* m_lectura){
	txt_write_in_stdout("BLOQUEA\n");
	sem_wait(m_lectura);
	txt_write_in_stdout("PASO BLOQUEO\n");
}

void limpiar_contador(sem_t* semaphore){
	semaphore->__align = 0;
}

void reiniciar_contador(sem_t* semaphore, int valor){
	semaphore->__align = valor - 1;
	sem_post(semaphore);
}

void desbloquear_contador(sem_t* m_lectura){
	txt_write_in_stdout("DESBLOQUEA\n");
	sem_post(m_lectura);
	txt_write_in_stdout("PASO DESBLOQUEA\n");
}

void agregar_semaforo(char* id, t_control_self* semaforo) {
	t_semaforo_self * lectura;
	lectura = malloc(sizeof(t_semaforo_self));
	lectura->contador = 0;
	lectura->semaforo = malloc(sizeof(sem_t));
	sem_init(lectura->semaforo, 0, 1);

	t_semaforo_self * escritura;
	escritura = malloc(sizeof(t_semaforo_self));
	escritura->contador = 0;
	escritura->semaforo = malloc(sizeof(sem_t));
	sem_init(escritura->semaforo, 0, 1);

	t_semaforo_self * intermedio;
	intermedio = malloc(sizeof(t_semaforo_self));
	intermedio->contador = 0;
	intermedio->semaforo = malloc(sizeof(sem_t));
	sem_init(intermedio->semaforo, 0, 1);

	dictionary_put(semaforo->lectura, id, lectura);
	dictionary_put(semaforo->escritura, id, escritura);
	dictionary_put(semaforo->intermedio, id, intermedio);
}

void desbloquear(char* id, int modo_escritura, t_control_self* semaforo, int modo_bloqueo_total) {
	t_semaforo_self * intermedio = dictionary_get(semaforo->intermedio, id);
	sem_wait(intermedio->semaforo);

	t_semaforo_self * escritura = dictionary_get(semaforo->escritura, id);
	if (modo_bloqueo_total == 0) {
		if (modo_escritura) {
			t_semaforo_self * lectura = dictionary_get(semaforo->lectura, id);
			sem_post(escritura->semaforo);
			int i;
			for (i = 0; i <= lectura->contador; i++)
				sem_post(lectura->semaforo);
			sem_post(intermedio->semaforo);
		} else {
			escritura->contador--;
			if (escritura->contador == 0)
				sem_post(escritura->semaforo);
			sem_post(intermedio->semaforo);
		}
	} else {
		t_semaforo_self * lectura = dictionary_get(semaforo->lectura, id);
		int i, value;
		for (i = 0; i <= lectura->contador; i++)
			sem_post(lectura->semaforo);
		sem_getvalue(escritura, &value);

		if (value < 1)
			sem_post(escritura->semaforo);
	}
}

void bloquear_escritura(char* id, t_semaforo_self * intermedio, t_control_self * semaforo) {
	txt_write_in_stdout("PUEDE ESCRIBIR");
	txt_write_in_stdout("\n");
	t_semaforo_self * lectura = dictionary_get(semaforo->lectura, id);
	sem_wait(lectura->semaforo);
	sem_post(intermedio->semaforo);
}

void bloquear_lectura(char* id, t_semaforo_self * intermedio, t_control_self * semaforo) {
	txt_write_in_stdout("PUEDE LEER");
	txt_write_in_stdout("\n");
	t_semaforo_self * escritura = dictionary_get(semaforo->escritura, id);
	sem_trywait(escritura->semaforo);
	escritura->contador++;
	sem_post(intermedio->semaforo);
}

void bloquear(char* id, int modo_escritura, t_control_self * semaforo, int modo_bloqueo_total) {
	int value;
	t_semaforo_self * intermedio = dictionary_get(semaforo->intermedio, id);
	sem_wait(intermedio->semaforo);

	if (modo_bloqueo_total == 0) {
		if (modo_escritura) {
			txt_write_in_stdout("ESCRITURA");
			txt_write_in_stdout("\n");
			t_semaforo_self * escritura = dictionary_get(semaforo->escritura, id);
			sem_getvalue(escritura->semaforo, &value);

			if (value > 0) {
				sem_wait(escritura->semaforo);
				bloquear_escritura(id, intermedio, semaforo);
			} else {
				txt_write_in_stdout("NO PUEDE ESCRIBIR");
				txt_write_in_stdout("\n");
				sem_post(intermedio->semaforo);
				txt_write_in_stdout("ESPERA ESCRITURA");
				txt_write_in_stdout("\n");
				sem_wait(escritura->semaforo);
				txt_write_in_stdout("RETOMA ESCRITURA");
				txt_write_in_stdout("\n");
				bloquear_escritura(id, intermedio, semaforo);
			}
		} else {
			txt_write_in_stdout("LECTURA");
			txt_write_in_stdout("\n");
			t_semaforo_self * lectura = dictionary_get(semaforo->lectura, id);
			sem_getvalue(lectura->semaforo, &value);

			if (value > 0) {
				bloquear_lectura(id, intermedio, semaforo);
			} else {
				lectura->contador++;
				txt_write_in_stdout("NO PUEDE LEER");
				txt_write_in_stdout("\n");
				sem_post(intermedio->semaforo);
				txt_write_in_stdout("ESPERA LECTURA");
				txt_write_in_stdout("\n");
				sem_wait(lectura->semaforo);
				txt_write_in_stdout("RETOMA LECTURA");
				txt_write_in_stdout("\n");
				bloquear_lectura(id, intermedio, semaforo);
			}
		}
	} else {
		int value_lectura, value_escritura;
		t_semaforo_self * lectura = dictionary_get(semaforo->lectura, id);
		sem_getvalue(lectura->semaforo, &value_lectura);
		t_semaforo_self * escritura = dictionary_get(semaforo->escritura, id);
		sem_getvalue(lectura->semaforo, &value_escritura);

		if (value_lectura && value_escritura) {
			sem_wait(lectura->semaforo);
			sem_wait(escritura->semaforo);
			sem_post(intermedio->semaforo);
		} else if (value_lectura == 0 && value_escritura == 0) {
			sem_post(intermedio->semaforo);
			sem_wait(lectura->semaforo);
			sem_wait(escritura->semaforo);
		} else {
			if (value_lectura) {
				sem_wait(lectura->semaforo);
				sem_post(intermedio->semaforo);
				sem_wait(escritura->semaforo);
			} else {
				sem_wait(escritura->semaforo);
				sem_post(intermedio->semaforo);
				sem_wait(lectura->semaforo);
			}
		}
	}
}

void desbloquear_nodo(char* nodo_id, int modo_escritura, int modo_bloqueo_total) {
	desbloquear(nodo_id, modo_escritura, semaforos_nodo, modo_bloqueo_total);
}

void bloquear_nodo(char* nodo_id, int modo_escritura, int modo_bloqueo_total) {
	bloquear(nodo_id, modo_escritura, semaforos_nodo, modo_bloqueo_total);
}

void agregar_semaforo_nodo(char* nodo_id) {
	agregar_semaforo(nodo_id, semaforos_nodo);
}

void desbloquear_bloque(char* nodo_id, int bloque_id, int modo_escritura, int modo_bloqueo_total) {
	char * clave = string_new();
	string_append(&clave, nodo_id);
	string_append(&clave, "-");
	string_append(&clave, string_itoa(bloque_id));

	desbloquear(clave, modo_escritura, semaforos_bloque, modo_bloqueo_total);
}

void bloquear_bloque(char* nodo_id, int bloque_id, int modo_escritura, int modo_bloqueo_total) {
	char * clave = string_new();
	string_append(&clave, nodo_id);
	string_append(&clave, "-");
	string_append(&clave, string_itoa(bloque_id));

	bloquear(clave, modo_escritura, semaforos_bloque, modo_bloqueo_total);
}

void agregar_semaforo_bloque(char* nodo_id, int bloque_id) {
	char * clave = string_new();
	string_append(&clave, nodo_id);
	string_append(&clave, "-");
	string_append(&clave, string_itoa(bloque_id));

	agregar_semaforo(clave, semaforos_bloque);
}

void desbloquear_archivo(char* archivo_id, int modo_escritura, int modo_bloqueo_total) {
	desbloquear(archivo_id, modo_escritura, semaforos_archivo, modo_bloqueo_total);
}

void bloquear_archivo(char* archivo_id, int modo_escritura, int modo_bloqueo_total) {
	bloquear(archivo_id, modo_escritura, semaforos_archivo, modo_bloqueo_total);
}

void agregar_semaforo_archivo(char* archivo_id) {
	agregar_semaforo(archivo_id, semaforos_archivo);
}

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

t_list * find_archivo_by_directorio_id(char* directorio_id) {
	t_list * lista;
	lista = list_create();

	int i;
	for (i = 0; i < list_size(archivos); i += 1) {
		t_archivo_self * archivo = list_get(archivos, i);
		if (strcasecmp(archivo->ruta_id, directorio_id) == 0) {
			list_add(lista, archivo);
		}
	}
	return lista;
}

char* generate_unique_id_to_nodo() {
	char* resultado;

	bloquear_contador(m_nodos_id);
	contador_nodo++;
	resultado = string_itoa(contador_nodo);
	mongo_update_long("id", "contadores", "contador_nodos", contador_nodo, "global");
	desbloquear_contador(m_nodos_id);

	return resultado;
}

char* generate_unique_id_to_directorio() {
	char* resultado;

	bloquear_contador(m_directorios_id);
	directorios->contador++;
	resultado = string_itoa(directorios->contador);
	mongo_update_long("id", "contadores", "contador_directorios", directorios->contador, "global");
	desbloquear_contador(m_directorios_id);

	return resultado;
}

char* generate_unique_id_to_archivo() {
	char* resultado;

	bloquear_contador(m_directorios_id);
	contador_archivos++;
	resultado = string_itoa(contador_archivos);
	mongo_update_long("id", "contadores", "contador_archivos", contador_archivos, "global");
	desbloquear_contador(m_directorios_id);

	return resultado;
}

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

int no_tiene_copia(t_archivo_self* archivo, int numero_bloque, t_nodo_self* nodo) {
	int i;
	t_bloque_archivo_self * bloque = dictionary_get(archivo->bloques, string_itoa(numero_bloque));

	if (bloque != NULL) {
		for (i = 0; i < list_size(bloque->copias); i++) {
			t_copia_self * copia = list_get(bloque->copias, i);
			if (strcasecmp(copia->nodo_bloque->nodo->nodo_id, nodo->nodo_id) == 0)
				return 0;
		}
	} else
		return 1;

	return 1;
}

t_nodo_self* find_nodo_disponible(int numero_bloque, t_archivo_self* archivo) {
	int espacio_disponible = 0;
	int i;
	t_nodo_self* selecto = NULL;
	for (i = 0; i < list_size(nodos); i++) {
		t_nodo_self * nodo = list_get(nodos, i);

		if (nodo->bloques_disponibles > espacio_disponible && nodo->estado == 1 && no_tiene_copia(archivo, numero_bloque, nodo)) {
			espacio_disponible = nodo->bloques_disponibles;
			selecto = nodo;
		}
	}

	return selecto;
}

int get_bloque_disponible(t_nodo_self* nodo) {
	t_list* lista = dictionary_get(bloques_nodo_disponible, nodo->nodo_id);

	if (list_size(lista)) {
		int disponible = list_get(lista, 0);
		list_remove(lista, 0);
		return disponible;
	}

	return -1;
}

t_nodo_self* find_nodo(char* nombre) {
	int i;
	for (i = 0; i < list_size(nodos); i += 1) {
		t_nodo_self * nodo = list_get(nodos, i);
		if (strcasecmp(nodo->nombre, nombre) == 0)
			return nodo;
	}
	return NULL;
}

t_nodo_self* find_nodo_by_id(char* nodo_id) {
	int i;
	for (i = 0; i < list_size(nodos); i += 1) {
		t_nodo_self * nodo = list_get(nodos, i);
		if (strcasecmp(nodo->nodo_id, nodo_id) == 0)
			return nodo;
	}
	return NULL;
}

void add_nodo_to_nodos(t_nodo_self* nodo, int existente) {
	list_add(nodos, nodo);
	if(!existente)
		iniciar_disponibles(nodo);
	t_list* bloque_archivo_control = list_create();
	dictionary_put(bloques_nodos_archivos, nodo->nodo_id, bloque_archivo_control);
}

void iniciar_disponibles(t_nodo_self* nodo) {
	int i;
	t_list * lista = list_create();
	for (i = 1; i <= CANTIDAD_BLOQUES_NODO_DEFAULT; i++) {
		int* numero;
		numero = malloc(sizeof(int));
		numero = (int) i;
		list_add(lista, numero);

		bson_t *doc;
		doc = bson_new();
		BSON_APPEND_UTF8(doc, "nodo_id", nodo->nodo_id);
		BSON_APPEND_INT32(doc, "numero", numero);
		mongo_insert_bson(doc, "bloques_nodo_disponible");
	}

	dictionary_put(bloques_nodo_disponible, nodo->nodo_id, lista);
}

t_archivo_self* archivo_no_disponible(t_archivo_self* archivo) {
	return archivo->estado == INVALIDO;
}

void chequear_alta_archivo(t_archivo_self* archivo, int numero_bloque) {
	if (archivo->estado == INVALIDO) {
		t_bloque_archivo_self* bloque = dictionary_get(archivo->bloques, string_itoa(numero_bloque));

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

			if (archivo->bloques_invalidos == 0) {
				archivo->estado = VALIDO;
			}
		}
	}
}

void reactivar_nodo(t_nodo_self* nodo) {
	if (nodo != NULL) {
		int i;
		nodo->estado = 1;
		t_list* archivos_nodo = dictionary_get(bloques_nodos_archivos, nodo->nodo_id);

		for (i = 0; i < list_size(archivos_nodo); i++) {
			t_bloque_archivo_control_self* bloque_archivo_control = list_get(archivos_nodo, i);
			t_archivo_self * archivo = find_archivo_by_id(bloque_archivo_control->archivo_id, archivos);
			chequear_alta_archivo(archivo, bloque_archivo_control->archivo_bloque);
		}
	}
}

t_nodo* crear_nodo(char* nombre, int estado, char* nodo_id, int bloques_disponibles){
	t_nodo_self* nodo;
	nodo = malloc(sizeof(t_nodo_self));
	nodo->estado = 1;
	nodo->nombre = string_duplicate(nombre);
	nodo->nodo_id = string_duplicate(nodo_id);
	nodo->bloques_disponibles = bloques_disponibles;
	nodo->bloques = dictionary_create();

	return nodo;
}

void alta_nodo(char* nombre) {
	//TODO: iniciar conexión con nodo.
	t_nodo_self* nodoAux = find_nodo(nombre);
	if (nodoAux == NULL) {
		t_nodo_self* nodo = crear_nodo(nombre, 1, generate_unique_id_to_nodo(), CANTIDAD_BLOQUES_NODO_DEFAULT);

		bson_t *doc;
		doc = bson_new();
		BSON_APPEND_UTF8(doc, "nombre", nodo->nombre);
		BSON_APPEND_INT32(doc, "estado", nodo->estado);
		BSON_APPEND_UTF8(doc, "nodo_id", nodo->nodo_id);
		BSON_APPEND_INT32(doc, "bloques_disponibles", nodo->bloques_disponibles);
		mongo_insert_bson(doc, "nodos");

		add_nodo_to_nodos(nodo, 0);
		agregar_semaforo_nodo(nodo->nodo_id);

		txt_write_in_stdout("Se agrego el nodo ");
		txt_write_in_stdout(nombre);
		txt_write_in_stdout(".\n");
	} else {
		if (nodoAux->estado == INVALIDO) {
			reactivar_nodo(nodoAux);
			desbloquear_nodo(nodoAux->nodo_id, 0, 1);
			txt_write_in_stdout("Se reactivo el nodo ");
			txt_write_in_stdout(nombre);
			txt_write_in_stdout(".\n");
		} else {
			txt_write_in_stdout("El nodo ");
			txt_write_in_stdout(nombre);
			txt_write_in_stdout(" ya se encontraba activo.\n");
		}
	}
}

void chequear_baja_archivo(t_archivo_self* archivo, int archivo_bloque) {
	t_bloque_archivo_self* bloque = dictionary_get(archivo->bloques, string_itoa(archivo_bloque));

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

		bloque->estado = desactivados != list_size(copias) && list_size(copias) != 0;

		if (bloque->estado == INVALIDO) {
			archivo->bloques_invalidos++;
			mongo_update_integer("id", archivo->id, "bloques_invalidos", archivo->bloques_invalidos, "archivos");
			archivo->estado = INVALIDO;
			mongo_update_integer("id", archivo->id, "estado", 0, "archivos");
		}
	}
}

void remove_nodo(char* nombre){

}

void baja_nodo(char* nombre) {
//TODO: detener conexión con nodo.
	t_nodo_self* nodo = find_nodo(nombre);
	if (nodo != NULL) {
		if (nodo->estado != INVALIDO) {
			int i;
			nodo->estado = 0;
			t_list* archivos_nodo = dictionary_get(bloques_nodos_archivos, nodo->nodo_id);

			bloquear_nodo(nodo->nodo_id, 0, 1);
			if (archivos_nodo != NULL) {
				for (i = 0; i < list_size(archivos_nodo); i++) {
					t_bloque_archivo_control_self* bloque_archivo_control = list_get(archivos_nodo, i);
					t_archivo_self * archivo = find_archivo_by_id(bloque_archivo_control->archivo_id, archivos);
					chequear_baja_archivo(archivo, bloque_archivo_control->archivo_bloque);
				}
			}
			desbloquear_nodo(nodo->nodo_id, 0, 1);

			txt_write_in_stdout("El nodo ");
			txt_write_in_stdout(nombre);
			txt_write_in_stdout(" se ha dado de baja.\n");
		} else {
			txt_write_in_stdout("El nodo ");
			txt_write_in_stdout(nombre);
			txt_write_in_stdout(" ya se encontraba inactivo.\n");
		}
	} else {
		txt_write_in_stdout("No existe el nodo indicado.\n");
	}
}

t_directorio_self* crear_directory(char* nombre, int padre, char* nivel, char* ruta, char* directorio_id) {
	t_directorio_self* directorio;
	directorio = malloc(sizeof(t_directorio_self));
	directorio->nombre = string_duplicate(nombre);
	directorio->padre = string_duplicate(padre);
	directorio->nivel = nivel;
	directorio->ruta = string_duplicate(ruta);
	directorio->id = string_duplicate(directorio_id);

	return directorio;
}

char* create_directorio(char* nombre, int nivel, char* padre, char* ruta) {
	t_directorio_self* directorio = crear_directory(nombre, padre, nivel, ruta, generate_unique_id_to_directorio());
	list_add(directorios->directorios, directorio);

	bson_t *doc;
	doc = bson_new();
	BSON_APPEND_UTF8(doc, "nombre", directorio->nombre);
	BSON_APPEND_UTF8(doc, "padre", directorio->padre);
	BSON_APPEND_INT32(doc, "nivel", directorio->nivel);
	BSON_APPEND_UTF8(doc, "ruta", directorio->ruta);
	BSON_APPEND_UTF8(doc, "id", directorio->id);
	mongo_insert_bson(doc, "directorios");

	bloquear_contador(m_control_directorios);
	directorios->contador_directorios++;
	mongo_update_long("id", "control", "control_directorios", directorios->contador_directorios, "global");
	desbloquear_contador(m_control_directorios);

	return directorio->id;
}

char * get_directorio_numero(char* nombre, int nivel, char* padre) {
	int i;
	char* res = "-1";
	for (i = 0; i < list_size(directorios->directorios); i++) {
		t_directorio_self* directorio = list_get(directorios->directorios, i);

		if (directorio->nivel == nivel) {
			if (strcasecmp(directorio->padre, padre) == 0) {
				if (strcasecmp(directorio->nombre, nombre) == 0)
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
	int total_directorios = 0;

	while (directorios_aux[i] != NULL) {
		total_directorios++;
		i++;
	}

	i = 0;
	while (1) {
		if (directorios_aux[i + sum] != NULL) {
			string_append(&ruta_aux, "/");
			string_append(&ruta_aux, directorios_aux[i]);
			if (encontrado) {
				char* padre_aux = get_directorio_numero(directorios_aux[i], i, padre);
				if (!(strcasecmp(padre_aux, "-1") == 0)) {
					padre = padre_aux;
					total_directorios--;
				} else {
					encontrado = 0;
					bloquear_contador(m_directorios_id);

					if (TAMANIO_MAXIMO_DIRECTORIOS >= directorios->contador_directorios + total_directorios) {
						desbloquear_contador(m_directorios_id);
						padre = create_directorio(directorios_aux[i], i, padre, ruta_aux);
					} else {
						desbloquear_contador(m_directorios_id);
						txt_write_in_stdout("No hay directorios disponibles.\n");
						break;
					}
				}
			} else {
				if (i > 0 || TAMANIO_MAXIMO_DIRECTORIOS >= directorios->contador_directorios + total_directorios)
					padre = create_directorio(directorios_aux[i], i, padre, ruta_aux);
				else {
					txt_write_in_stdout("No hay directorios disponibles.\n");
					break;
				}
			}
			i++;
		} else {
			break;
		}
	}

	return padre;
}

t_archivo_self* crear_archivo(char* nombre, int estado, char* archivo_id, int tamanio, int bloques_invalidos){
	t_archivo_self* archivo;
	archivo = malloc(sizeof(t_archivo_self));
	archivo->nombre = string_duplicate(nombre);
	archivo->estado = estado;
	archivo->bloques = dictionary_create();
	archivo->tamanio = tamanio;
	archivo->bloques_invalidos = bloques_invalidos;
	archivo->id = string_duplicate(archivo_id);
	archivo->ruta_id = set_directorio(nombre, true);

	return archivo;
}

t_archivo_self* create_archivo(long tamanio, char* nombre) {
	t_archivo_self* archivo = crear_archivo(nombre, 0, generate_unique_id_to_archivo(), tamanio, 0);

	bson_t *doc;
	doc = bson_new();
	BSON_APPEND_UTF8(doc, "nombre", archivo->nombre);
	BSON_APPEND_INT32(doc, "estado", archivo->estado);
	BSON_APPEND_INT64(doc, "tamanio", archivo->tamanio);
	BSON_APPEND_INT32(doc, "bloques_invalidos", archivo->bloques_invalidos);
	BSON_APPEND_UTF8(doc, "id", archivo->id);
	BSON_APPEND_UTF8(doc, "ruta_id", archivo->ruta_id);
	mongo_insert_bson(doc, "archivos");

	return archivo;
}

t_bloque_archivo_self* create_bloque(int numero, int estado) {
	t_bloque_archivo_self* bloque;
	bloque = malloc(sizeof(t_bloque_archivo_self));
	bloque->copias = list_create();
	bloque->estado = estado;
	bloque->numero = numero;

	return bloque;
}

t_copia_self* create_copia(int bloque, t_nodo_self* nodo, char* archivo_id, int nodo_bloque_destino, int nodo_ocupado, int numero) {
	t_copia_self* copia;
	copia = malloc(sizeof(t_copia_self));
	copia->archivo_bloque = bloque;
	copia->archivo_id = string_duplicate(archivo_id);
	copia->numero = numero;
	asignar_bloque_nodo_a_copia(copia, nodo, nodo_bloque_destino, nodo_ocupado);

// Agrega el nodo_bloque usado a la lista de control.
	t_bloque_archivo_control_self* control;
	control = malloc(sizeof(t_bloque_archivo_control_self));
	control->archivo_bloque = bloque;
	control->archivo_id = string_duplicate(archivo_id);
	t_list* lista = dictionary_get(bloques_nodos_archivos, nodo->nodo_id);
	list_add(lista, control);

	return copia;
}

void asignar_bloque_nodo_a_copia(t_copia_self* copia, t_nodo_self* nodo, int nodo_bloque_destino, int nodo_ocupado) {
	t_nodo_bloque_self *nodo_bloque;
	nodo_bloque = malloc(sizeof(t_nodo_bloque_self));
	nodo_bloque->nodo = nodo;
	if (nodo_bloque_destino == NULL)
		nodo_bloque->bloque_nodo = get_bloque_disponible(nodo);
	else {
		nodo_bloque->bloque_nodo = nodo_bloque_destino;

		if (nodo_ocupado) {
			t_list* lista = ((t_list*) dictionary_get(bloques_nodo_disponible, nodo->nodo_id));
			int i;
			for (i = 0; i < lista->elements_count; i++) {
				int disponible = list_get(lista, i);
				if (disponible == nodo_bloque_destino) {
					list_remove(lista, i);
					break;
				}
			}
		}
	}

	bson_t *doc;
	doc = bson_new();
	BSON_APPEND_UTF8(doc, "nodo_id", nodo->nodo_id);
	BSON_APPEND_INT32(doc, "nodo_bloque", nodo_bloque->bloque_nodo);
	BSON_APPEND_UTF8(doc, "archivo_id", copia->archivo_id);
	BSON_APPEND_INT32(doc, "numero_copia", copia->numero);
	BSON_APPEND_INT32(doc, "archivo_bloque", copia->archivo_bloque);
	mongo_insert_bson(doc, "bloques_nodo_copia");

	dictionary_put(nodo->bloques, string_itoa(nodo_bloque->bloque_nodo), copia);
	copia->nodo_bloque = nodo_bloque;
	if (!nodo_ocupado) {
		nodo->bloques_disponibles--;
		mongo_update_integer("nodo_id", nodo->nodo_id, "bloques_disponibles", nodo->bloques_disponibles, "nodos");
		mongo_delete_2("nodo_id", nodo->nodo_id, "numero", nodo_bloque->bloque_nodo, "bloques_nodo_disponible");
	}
}

void asignar_copia_a_bloque_archivo(t_bloque_archivo_self* bloque, t_copia_self* copia) {
	list_add(bloque->copias, copia);
}

void asignar_bloque_archivo_a_archivo(t_archivo_self* archivo, t_bloque_archivo_self* bloque) {
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
	int max = TAMANIO_BLOQUE_NODO;
	int maxRead = max - sizeof(char);

	file = fopen(nombre, "rb");
	if (file == NULL) {
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

void borrado_fisico_nodo(char* nodo) {
//TODO: Solicitud de borrado de informacion de nodo.
}

void formatear_mdfs() {
	int i, j;

	for (i = 0; i < archivos->elements_count; i++) {
		t_archivo_self* archivo = list_get(archivos, i);
		t_dictionary* bloques = archivo->bloques;
		for (j = 0; j < archivo->cantidad_bloques; j++) {
			t_bloque_archivo_self* bloque = dictionary_get(bloques, string_itoa(j));
			if(bloque != NULL)
				list_clean(bloque->copias);
		}
		dictionary_clean(bloques);
	}
	list_clean(archivos);

	mongo_delete("", "", "bloques_nodo_disponible");

	for (i = 0; i < nodos->elements_count; i++) {
		t_nodo_self* nodo = list_get(nodos, i);
		borrado_fisico_nodo(nodo->nodo_id);
		t_list * bna = dictionary_get(bloques_nodos_archivos, nodo->nodo_id);
		t_list * bnd = dictionary_get(bloques_nodo_disponible, nodo->nodo_id);
		nodo->bloques_disponibles = CANTIDAD_BLOQUES_NODO_DEFAULT;
		mongo_update_integer("nodo_id", nodo->nodo_id, "bloques_disponibles", CANTIDAD_BLOQUES_NODO_DEFAULT, "nodos");

		list_clean(bna);
		list_clean(bnd);
		dictionary_clean(nodo->bloques);
		dictionary_remove(bloques_nodo_disponible, nodo->nodo_id);

		iniciar_disponibles(nodo);
	}

	mongo_delete("", "", "archivos");
	mongo_delete("", "", "bloques_nodo_copia");

	list_clean(directorios->directorios);
	mongo_delete("", "", "directorios");
}

void mover_archivo(char* nombre, char* destino) {
	t_archivo_self* archivo = find_archivo(nombre, archivos);
	archivo->ruta_id = set_directorio(destino, true);
	mongo_update_string("id", archivo->id, "ruta_id", archivo->ruta_id, "archivos");
}

void renombrar_archivo(char * ruta, char* nombre) {
	t_archivo_self* archivo = find_archivo(ruta, archivos);
	int i = 0;
	char** ruta_aux = string_split(archivo->nombre, "/");
	char* destino = "/";

	while (ruta_aux[i + 1] != NULL) {
		string_append(&destino, ruta_aux[i]);
		string_append(&destino, "/");
		i++;
	}

	string_append(&destino, nombre);
	archivo->nombre = set_directorio(destino, true);
	mongo_update_string("id", archivo->id, "nombre", archivo->nombre, "archivos");
}

void eliminar_archivo(char * ruta) {
	int indice, i, j, k;
	t_archivo_self * archivo = NULL;

	for (indice = 0; indice < list_size(archivos); indice += 1) {
		t_archivo_self * archivo_aux = list_get(archivos, indice);
		if (strcasecmp(archivo_aux->nombre, ruta) == 0) {
			archivo = archivo_aux;
			break;
		}
	}

	if (archivo != NULL) {
		for (i = archivo->cantidad_bloques - 1; i >= 0; i--) {
			t_bloque_archivo_self* bloque = dictionary_get(archivo->bloques, string_itoa(i));
			for (j = 0; j < bloque->copias->elements_count	; j++) {
				t_copia_self* copia = list_get(bloque->copias, j);
				t_nodo_bloque_self* bloque_nodo = copia->nodo_bloque;
				t_nodo_self * nodo = bloque_nodo->nodo;
				//TODO: Si el nodo no esta disponible, no se puede borrar los datos.
				nodo->bloques_disponibles++;
				mongo_update_integer("nodo_id", nodo->nodo_id, "bloques_disponibles", nodo->bloques_disponibles, "nodos");

				dictionary_put(bloques_nodo_disponible, nodo->nodo_id, bloque_nodo->bloque_nodo);
				bson_t *doc;
				doc = bson_new();
				BSON_APPEND_UTF8(doc, "nodo_id", nodo->nodo_id);
				BSON_APPEND_INT32(doc, "numero", bloque_nodo->bloque_nodo);
				mongo_insert_bson(doc, "bloques_nodo_disponible");

				t_list* archivos_nodo = dictionary_get(bloques_nodos_archivos, nodo->nodo_id);

				for (k = list_size(archivos_nodo) - 1; k >= 0; k--) {
					t_bloque_archivo_control_self* bloque_archivo_control = list_get(archivos_nodo, k);
					if (strcasecmp(bloque_archivo_control->archivo_id, archivo->id) == 0) {
						list_remove(archivos_nodo, k);
						break;
					}
				}
				mongo_delete("archivo_id", archivo->id, "bloques_nodo_copia");
			}
			list_destroy(bloque->copias);
		}

		list_remove(archivos, indice);
		dictionary_destroy(archivo->bloques);
		mongo_delete("archivo_id", archivo->id, "bloques_nodo_copia");
		mongo_delete("id", archivo->id, "archivos");
		free(archivo);
	}
}

void crear_directorio(char* ruta, char* nombre) {
	char * destino = string_new();
	if (ruta != NULL)
		string_append(&destino, ruta);

	string_append(&destino, "/");
	string_append(&destino, nombre);
	set_directorio(destino, false);
}

t_directorio_self* get_directorio_by_id(char* archivo_id) {
	int i;
	for (i = 0; i < list_size(directorios->directorios); i++) {
		t_directorio_self* directorio_aux = list_get(directorios->directorios, i);
		if (strcasecmp(archivo_id, directorio_aux->id) == 0)
			return directorio_aux;
	}

	return NULL;
}

char * get_ruta_directorio(t_directorio_self* directorio) {
	char* padre = directorio->padre;
	char* ultimo = strdup("0");
	char* ruta = string_new();
	string_append(&ruta, "/");
	string_append(&ruta, directorio->nombre);

	while (strcasecmp(padre, ultimo) != 0) {
		t_directorio_self* directorio_aux = get_directorio_by_id(padre);
		char* ruta_aux = strdup("/");
		string_append(&ruta_aux, directorio_aux->nombre);
		string_append(&ruta_aux, ruta);
		ruta = string_duplicate(ruta_aux);
		padre = string_duplicate(directorio_aux->padre);
	}

	return ruta;
}

t_directorio_self* get_directorio_by_ruta(char* ruta) {
	char** ruta_aux = string_split(ruta, "/");
	int i = 0;
	int j;
	char * padre = string_new();
	string_append(&padre, "0");
	t_directorio_self * resultado;
	resultado = malloc(sizeof(t_directorio_self));

	while (ruta_aux[i] != NULL) {
		resultado = NULL;
		for (j = 0; j < list_size(directorios->directorios); j++) {
			t_directorio_self * directorio = list_get(directorios->directorios, j);
			if (strcasecmp(directorio->nombre, ruta_aux[i]) == 0) {
				if (directorio->nivel == i) {
					if (strcasecmp(directorio->padre, padre) == 0) {
						padre = directorio->id;
						resultado = list_get(directorios->directorios, j);
						break;
					}
				}
			}
		}
		i++;
	}

	return resultado;
}

void renombrar_directorio(char* ruta, char* nombre) {
	char* ruta_aux = string_new();

	if (strncmp(ruta, "/", 1) == 0) {

	} else {
		string_append(&ruta_aux, "/");
	}
	string_append(&ruta_aux, ruta);

	t_directorio_self* directorio = get_directorio_by_ruta(ruta_aux);
	directorio->nombre = nombre;
	mongo_update_string("id", directorio->id, "nombre", directorio->nombre, "directorios");

	char** directorios_aux = string_split(ruta_aux, "/");
	int i = 0;
	char * ruta_final = string_new();

	while (i < directorio->nivel) {
		string_append(&ruta_final, "/");
		string_append(&ruta_final, directorios_aux[i]);
		i++;
	}

	string_append(&ruta_final, "/");
	string_append(&ruta_final, nombre);
	directorio->ruta = ruta_final;
	mongo_update_string("id", directorio->id, "ruta", directorio->ruta, "directorios");

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

int get_espacio_disponible() {
	int i;
	int total = 0;
	for (i = 0; i < nodos->elements_count; i++) {
		t_nodo_self* nodo = list_get(nodos, i);
		if (nodo->estado == VALIDO)
			total += nodo->bloques_disponibles;
	}

	return total;
}

int hay_espacio_disponible(int cantidad_bloques) {
	int i;
	int restantes = 0;
	int cantidad_necesaria = restantes = cantidad_bloques * 3;
	t_dictionary * nodos_aux = dictionary_create();

	for (i = 0; i < nodos->elements_count; i++) {
		t_nodo_self* nodo = list_get(nodos, i);
		if (nodo->estado == VALIDO) {

			if (nodo->bloques_disponibles > cantidad_bloques)
				restantes -= cantidad_bloques;
			else
				restantes -= nodo->bloques_disponibles;

			if (restantes <= 0)
				return true;
		}
	}

	if (restantes <= 0)
		return true;
	else
		return false;
}

void copiar_archivo_a_mdfs(char* ruta, char* destino_aux) {
//	t_archivo_datos_self* archivo_datos = dividir_archivo(ruta);

	FILE *file;
	long filelen;
	int i;
	int max = TAMANIO_BLOQUE_NODO;
	int maxRead = max - sizeof(char);

	file = fopen(ruta, "rb");

	if (file != NULL) {
		datos = malloc(sizeof(datos));
		datos->bloques = list_create();

		fseek(file, 0, SEEK_END);
		filelen = ftell(file);
		rewind(file);

		int cantidad_bloques = filelen / maxRead;
		int resto = filelen % maxRead;
		cantidad_bloques = cantidad_bloques + (resto != 0 ? 1 : 0);

		int espacio_disponible = hay_espacio_disponible(cantidad_bloques);

		if (espacio_disponible) {
			char * nombre_archivo = get_nombre_archivo(ruta);
			char * destino = string_duplicate(destino_aux);
			if (!string_ends_with(destino, "/"))
				string_append(&destino, "/");
			string_append(&destino, nombre_archivo);
			t_archivo_self * existe_archivo = find_archivo(destino, archivos);

			if (existe_archivo == NULL) {
				t_archivo_self* archivo = create_archivo(filelen, destino);
				archivo->cantidad_bloques = cantidad_bloques;
				mongo_update_integer("id", archivo->id, "cantidad_bloques", cantidad_bloques, "archivos");

				int i;
				for (i = 0; i < cantidad_bloques; i++) {
					t_bloque_archivo_self* bloque = create_bloque(i, 1);
					asignar_bloque_archivo_a_archivo(archivo, bloque);

					char* segmento;
					segmento = (char *) malloc((max) * sizeof(char));
					if (i - 1 < cantidad_bloques || resto == 0) {
						fread(*&segmento, maxRead, 1, file);
						segmento = strcat(segmento, "\0");
					} else {
						fread(*&segmento, max, 1, file);

						for (i = 0; i < resto; i++) {
							segmento = strcat(segmento, "\0");
						}
					}

					list_add(datos->bloques, segmento);
					//TODO: Enviar datos a nodos.
					t_copia_self* copia1 = create_copia(i, find_nodo_disponible(i, archivo), archivo->id, NULL,
					false, 1);
					asignar_copia_a_bloque_archivo(bloque, copia1);

					t_copia_self* copia2 = create_copia(i, find_nodo_disponible(i, archivo), archivo->id, NULL,
					false, 2);
					asignar_copia_a_bloque_archivo(bloque, copia2);

					t_copia_self* copia3 = create_copia(i, find_nodo_disponible(i, archivo), archivo->id, NULL,
					false, 3);
					asignar_copia_a_bloque_archivo(bloque, copia3);
				}
				archivo->estado = VALIDO;
				mongo_update_integer("id", archivo->id, "estado", 1, "archivos");

				add_archivo_to_archivos(archivo);
			} else {
				txt_write_in_stdout("Ya existe un archivo con el mismo nombre en la ruta indicada.\n");
			}
		} else {
			txt_write_in_stdout("No hay espacio disponible en el filesystem.\n");
		}
		fclose(file);
	} else {
		txt_write_in_stdout("No se encontro el archivo indicado.\n");
	}
}

void listar_nodos() {
	int i;

	for (i = 0; i < nodos->elements_count; i++) {
		t_nodo_self * nodo = list_get(nodos, i);
		txt_write_in_stdout("Nodo: ");
		txt_write_in_stdout(nodo->nombre);
		txt_write_in_stdout(" - Id: ");
		txt_write_in_stdout(nodo->nodo_id);
		txt_write_in_stdout(" - Bloques disponibles: ");
		txt_write_in_stdout(string_itoa(nodo->bloques_disponibles));
		txt_write_in_stdout(" - Estado: ");
		txt_write_in_stdout(string_itoa(nodo->estado));
		txt_write_in_stdout(".\n");
	}
}

void listar_bloques_archivo(char* nombre) {
	int j, k;

	t_archivo_self* archivo = find_archivo(nombre, archivos);

	if (archivo != NULL) {
		txt_write_in_stdout(get_nombre_archivo(archivo->nombre));
		txt_write_in_stdout("\n");
		txt_write_in_stdout("Cantidad de bloques: ");
		txt_write_in_stdout(string_itoa(archivo->cantidad_bloques));
		txt_write_in_stdout("\n");

		for (j = 0; j < archivo->cantidad_bloques; j++) {
			t_bloque_archivo_self * bloque = dictionary_get(archivo->bloques, string_itoa(j));
			if (bloque != NULL) {
				txt_write_in_stdout("	Bloque: ");
				txt_write_in_stdout(string_itoa(bloque->numero));
				txt_write_in_stdout(" - Estado: ");
				txt_write_in_stdout(string_itoa(bloque->estado));
				txt_write_in_stdout(" - Cantidad de copias: ");
				txt_write_in_stdout(string_itoa(bloque->copias->elements_count));
				txt_write_in_stdout("\n");

				for (k = 0; k < bloque->copias->elements_count; k++) {
					t_copia_self * copia = list_get(bloque->copias, k);
					txt_write_in_stdout("		Copia: ");
					txt_write_in_stdout(string_itoa(copia->numero));
					txt_write_in_stdout(" - Nodo: ");
					txt_write_in_stdout(copia->nodo_bloque->nodo->nombre);
					txt_write_in_stdout(" - Bloque: ");
					txt_write_in_stdout(string_itoa(copia->nodo_bloque->bloque_nodo));
					txt_write_in_stdout(".\n");
				}
			}
		}
	} else {
		txt_write_in_stdout("Archivo no encontrado.\n");
	}
}

void listar_todo() {
	int i, j, k;
	int cola = list_size(directorios->directorios);
	int encontrado = false;

	for (j = 0; j < cola; j++) {
		for (i = 0; i < list_size(directorios->directorios); i++) {
			t_directorio_self * directorio = list_get(directorios->directorios, i);
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

				for (k = 0; k < archivos->elements_count; k++) {
					t_archivo_self * archivo = list_get(archivos, k);
					if (strcasecmp(archivo->ruta_id, directorio->id) == 0) {
						txt_write_in_stdout(get_nombre_archivo(archivo->nombre));
						txt_write_in_stdout(": ");
						txt_write_in_stdout(archivo->nombre);
						txt_write_in_stdout(" ");
						txt_write_in_stdout(string_itoa(archivo->tamanio));
						txt_write_in_stdout(" ");
						txt_write_in_stdout(string_itoa(archivo->estado));
						txt_write_in_stdout(".\n");
					}
				}
			}
		}
		if (encontrado)
			encontrado = false;
		else
			break;
	}
}

void listar_archivos() {
	int i;
	for (i = 0; i < archivos->elements_count; i++) {
		t_archivo_self * archivo = list_get(archivos, i);
		txt_write_in_stdout(get_nombre_archivo(archivo->nombre));
		txt_write_in_stdout(": ");
		txt_write_in_stdout(archivo->nombre);
		txt_write_in_stdout(" ");
		txt_write_in_stdout(string_itoa(archivo->tamanio));
		txt_write_in_stdout(" ");
		txt_write_in_stdout(string_itoa(archivo->estado));
		txt_write_in_stdout(".\n");
	}
}

void listar_directorios() {
	int i;
	int j;
	int cola = list_size(directorios->directorios);
	int encontrado = false;

	for (j = 0; j < cola; j++) {
		for (i = 0; i < list_size(directorios->directorios); i++) {
			t_directorio_self * directorio = list_get(directorios->directorios, i);
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

void eliminar_directorio(char* ruta) {
//TODO: Eliminar archivos contenidos en los directorios.
	t_directorio_self* directorio = get_directorio_by_ruta(ruta);
	t_list* directorios_a_eliminar = list_create();
	t_list* directorios_actuales = list_create();
	list_add(directorios_actuales, directorio);
	int i, j, k;

	for (i = list_size(directorios_actuales) - 1; i >= 0; i--) {
		t_directorio_self* directorio_padre = list_get(directorios_actuales, i);
		for (j = 0; j < list_size(directorios->directorios); j++) {
			t_directorio_self* directorio_aux = list_get(directorios->directorios, j);
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
			t_directorio_self* directorio_aux = list_get(directorios->directorios, j);
			if (strcasecmp(directorio_aux->id, directorio_a_eliminar->id) == 0) {
				list_remove(directorios->directorios, j);
				mongo_delete("id", directorio_aux->id, "directorios");

				t_list * lista_archivos = find_archivo_by_directorio_id(directorio_aux->id);
				for (k = 0; k < lista_archivos->elements_count; k++) {
					t_archivo_self* archivo = list_get(lista_archivos, k);
					eliminar_archivo(archivo->nombre);
				}

				bloquear_contador(m_directorios_id);
				directorios->contador_directorios--;
				mongo_update_long("id", "contadores", "contador_directorios", directorios->contador_directorios, "global");
				desbloquear_contador(m_directorios_id);
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

int es_subdirectorio_by_ruta(char* ruta, char* destino) {
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

int es_subdirectorio_by_directorio(t_directorio_self* ruta, t_directorio_self* destino) {
	if (ruta == NULL || destino == NULL)
		return 0;

	if (strcasecmp(ruta->id, destino->id) == 0)
		return 1;

	if (strcasecmp(ruta->padre, "0") == 0 && strcasecmp(destino->padre, "0") == 0)
		return 0;

	if (strcasecmp(destino->padre, "0") == 0)
		return 0;

	if (ruta->nivel == destino->nivel)
		return 0;

	if (ruta->nivel > destino->nivel)
		return 0;

	t_directorio_self* directorio = get_directorio_by_id(destino->padre);

	while (strcasecmp(directorio->padre, "0") != 0) {
		if (strcasecmp(ruta->id, directorio->padre) == 0)
			return 1;
		directorio = get_directorio_by_id(directorio->padre);
	}

	return 0;
}

void mover_directorio(char* ruta, char* destino) {
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
			t_directorio_self* directorio_padre = list_get(directorios_actuales, i);
			for (j = 0; j < list_size(directorios->directorios); j++) {
				t_directorio_self* directorio_aux = list_get(directorios->directorios, j);
				if (strcasecmp(directorio_aux->padre, directorio_padre->id) == 0) {
					list_add(directorios_a_modificar, directorio_aux);
					list_add(directorios_actuales, directorio_aux);
				}
			}
			list_remove(directorios_actuales, i);
			i = list_size(directorios_actuales);
		}
		list_destroy(directorios_actuales);

		for (i = 0; i < list_size(directorios_a_modificar); i++) {
			t_directorio_self* directorio_a_eliminar = list_get(directorios_a_modificar, i);
			for (j = 0; j < list_size(directorios->directorios); j++) {
				t_directorio_self* directorio_aux = list_get(directorios->directorios, j);
				if (strcasecmp(directorio_aux->id, directorio_a_eliminar->id) == 0) {
					directorio_aux->nivel = directorio_aux->nivel + (cantidad_niveles_destino - cantidad_niveles_ruta + 1);
					mongo_update_integer("id", directorio_aux->id, "nivel", directorio_aux->nivel, "directorios");
					break;
				}
			}
		}

		list_destroy(directorios_a_modificar);
		directorio_origen->nivel = directorio_origen->nivel + (cantidad_niveles_destino - cantidad_niveles_ruta + 1);
		mongo_update_integer("id", directorio_origen->id, "nivel", directorio_origen->nivel, "directorios");
		directorio_origen->padre = padre_final;
		mongo_update_string("id", directorio_origen->id, "padre", directorio_origen->padre, "directorios");
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
	if (f == NULL) {
		printf("Error opening file!\n");
		exit(1);
	}

	for (i = 0; i < list_size(bloques); i++) {
		fprintf(f, list_get(bloques, i));
	}

	fclose(f);

	return 1;
}

char* get_data_from_nodo(char* nombre, t_copia_self * bloque, int campo_temporal) {
//TODO: Solicitar bloque a nodo.
//TEMPORAL: Se va a agregar lo solicitado al nodo.
	return list_get(datos->bloques, campo_temporal);
}

int copiar_archivo_a_local(char* nombre, char* ruta) {
	t_list* bloques_archivo = list_create();
	t_archivo_self * archivo = find_archivo(nombre, archivos);

	int i, j, k;
	int completo = true;

	if (archivo->estado == VALIDO) {
		for (i = 0; i < dictionary_size(archivo->bloques); i++) {
			t_bloque_archivo_self* bloque = dictionary_get(archivo->bloques, string_itoa(i));

			int encontrado = false;
			for (j = 0; j < dictionary_size(bloque->copias); j++) {
				t_copia_self* copia = list_get(bloque->copias, j);
				t_nodo_bloque_self* bloque_nodo = copia->nodo_bloque;
				t_nodo_self * nodo = bloque_nodo->nodo;
				if (nodo->estado == VALIDO) {
					list_add(bloques_archivo, get_data_from_nodo(nodo->nodo_id, copia, i));
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

t_bloque_archivo_self * get_bloque(char* nombre, int bloque_id) {
	t_archivo_self * archivo = find_archivo(nombre, archivos);

	if (archivo != NULL) {
		return dictionary_get(archivo->bloques, string_itoa(bloque_id));
	} else {
		return NULL;
	}
}

t_copia_self * get_copia_bloque_valida(t_bloque_archivo_self * bloque) {
	int i;
	for (i = bloque->copias->elements_count - 1; i >= 0; i--) {
		t_copia_self * copia = list_get(bloque->copias, i);

		if (copia->nodo_bloque->nodo->estado == VALIDO)
			return copia;
	}

	return NULL;
}

void ver_bloque(char* nombre, int bloque_id) {
	t_nodo_self * nodo = find_nodo(nombre);

	if (nodo != NULL) {
		t_copia_self * copia = dictionary_get(nodo->bloques, string_itoa(bloque_id));

		if (copia != NULL) {
			char * data = string_new();
			txt_write_in_stdout("CONTENIDO:\n");
			txt_write_in_stdout(get_data_from_nodo(nombre, copia, bloque_id));
			txt_write_in_stdout("\n");
		} else {
			txt_write_in_stdout("No hay datos dentro de ese nodo.\n");
		}
	} else {
		txt_write_in_stdout("No se encontro el nodo solicitado.\n");
	}
}

//void ver_bloque(char* nombre, int bloque_id) {
//	t_bloque_archivo_self * bloque = get_bloque(nombre, bloque_id);
//
//	if (bloque != NULL) {
//		t_copia_self * copia = get_copia_bloque_valida(bloque);
//
//		if (copia != NULL) {
//			char * data = string_new();
//			txt_write_in_stdout("CONTENIDO:\n");
//			txt_write_in_stdout(get_data_from_nodo(nombre, copia, bloque_id));
//			txt_write_in_stdout("\n");
//		} else {
//			txt_write_in_stdout("No se encontro una copia disponible.\n");
//		}
//	} else {
//		txt_write_in_stdout("No se encontro el bloque solicitado.\n");
//	}
//}

void borrado_fisico_bloque() {

}

//void borrar_archivo(char* nombre) {
//	int i, j;
//	t_archivo_self * archivo = find_archivo(nombre, archivos);
//	t_dictionary* bloques = archivo->bloques;
//
//	for (i = archivo->cantidad_bloques; i >= 0; i--) {
//		t_list* copias = list_get(bloques, i);
//
//		for (j = archivo->cantidad_bloques; j >= 0; j--) {
//			t_copia_self* copia = list_get(copias, j);
//			copia->nodo_bloque->nodo->bloques_disponibles++;
//			mongo_update_integer("nodo_id", copia->nodo_bloque->nodo->nodo_id, "bloques_disponibles",
//					copia->nodo_bloque->nodo->bloques_disponibles, "nodos");
//		}
//		list_destroy(copias);
//	}
//	list_destroy(bloques);
//}

void borrar_bloque(char* nombre, int bloque_nodo_id) {

	if (bloque_nodo_id <= CANTIDAD_BLOQUES_NODO_DEFAULT) {
		int i;
		t_nodo_self * nodo = find_nodo(nombre);

		if (nodo != NULL) {
			if (nodo->estado == VALIDO) {
				t_copia_self * copia = dictionary_get(nodo->bloques, string_itoa(bloque_nodo_id));

				if (copia != NULL) {
//					bloquear_bloque(nodo->nodo_id, bloque_nodo_id, true, false);
					borrado_fisico_bloque(); //TODO: borrado fisico de bloque del nodo.
					t_archivo_self* archivo = find_archivo_by_id(copia->archivo_id, archivos);
					t_bloque_archivo_self * bloque = dictionary_get(archivo->bloques, string_itoa(copia->archivo_bloque));
					int archivo_bloque = copia->archivo_bloque;
					int numero_copia = copia->numero;

					dictionary_put(bloques_nodo_disponible, nodo->nodo_id, copia->nodo_bloque->bloque_nodo);
					bson_t *doc;
					doc = bson_new();
					BSON_APPEND_UTF8(doc, "nodo_id", nodo->nodo_id);
					BSON_APPEND_INT32(doc, "numero", copia->nodo_bloque->bloque_nodo);
					mongo_insert_bson(doc, "bloques_nodo_disponible");

					dictionary_remove(nodo->bloques, string_itoa(bloque_nodo_id));
					mongo_delete_2("nodo_id", nodo->nodo_id, "nodo_bloque", copia->nodo_bloque->bloque_nodo, "bloques_nodo_copia");
					nodo->bloques_disponibles++;
					mongo_update_integer("nodo_id", nodo->nodo_id, "bloques_disponibles", nodo->bloques_disponibles, "nodos");

					for (i = 0; i < bloque->copias->elements_count; i++) {
						t_copia_self * copia_aux = list_get(bloque->copias, i);
						if (copia_aux->numero == numero_copia) {
							list_remove(bloque->copias, i);
							if (numero_copia <= bloque->copias->elements_count) {
								t_copia_self * copia_aux2 = list_get(bloque->copias, bloque->copias->elements_count - 1);
								copia_aux2->numero = numero_copia;
								mongo_update_2("nodo_id", copia_aux2->nodo_bloque->nodo->nodo_id, "nodo_bloque",
										copia_aux2->nodo_bloque->bloque_nodo, "numero_copia", string_itoa(numero_copia), "bloques_nodo_copia", 1);
							}
							break;
						}
					}

					t_list* archivos_nodo = dictionary_get(bloques_nodos_archivos, nodo->nodo_id);

					for (i = 0; i < archivos_nodo->elements_count; i++) {
						t_bloque_archivo_control_self* bloque_archivo_control = list_get(archivos_nodo, i);
						if (strcasecmp(bloque_archivo_control->archivo_id, archivo->id) == 0
								&& bloque_archivo_control->archivo_bloque == archivo_bloque) {
							list_remove(archivos_nodo, i);
							break;
						}
					}
					chequear_baja_archivo(archivo, archivo_bloque);
//					desbloquear_bloque(nodo->nodo_id, bloque_nodo_id, true, false);

					txt_write_in_stdout("El bloque fue borrado exitosamente.\n");
				} else {
					txt_write_in_stdout("No hay informacion en el bloque indicado.\n");
				}
			} else {
				txt_write_in_stdout("El nodo se encuentra inactivo.\n");
			}
		} else {
			txt_write_in_stdout("No existe el nodo indicado.\n");
		}
	} else {
		txt_write_in_stdout("No existe el bloque indicado.\n");
	}
}

// Esto borraba un bloque de un archivo en general, lo que se hizo fue que borre un archivo de nodo.
//void borrar_bloque(char* nombre, int bloque_id) {
//	int i;
//	borrado_fisico_bloque();//TODO: borrado fisico de bloque del nodo.
//	t_archivo_self * archivo = find_archivo(nombre, archivos);
//	t_bloque_archivo_self * bloque = dictionary_get(archivo->bloques,string_itoa(bloque_id));
//
//	for (i = bloque->copias->elements_count - 1; i >= 0; i--) {
//		t_copia_self * copia = list_get(bloque->copias, i);
//		t_nodo_bloque_self * nodo_bloque = copia->nodo_bloque;
//		nodo_bloque->nodo->bloques_disponibles++;
//		free(nodo_bloque);
//	}
//	archivo->estado = INVALIDO;
//
//	list_destroy(bloque->copias);
//	dictionary_remove(archivo->bloques, string_itoa(bloque_id));
//}

//void copiar_bloque(char* nombre, int bloque_id, char* destino) {
//	//TODO: Copiar fisicamente al nodo indicado.
//	t_archivo_self * archivo = find_archivo(nombre, archivos);
//
//	if (archivo != NULL) {
//		t_bloque_archivo_self * bloque = get_bloque(nombre, bloque_id);
//
//		if (bloque != NULL) {
//			t_copia_self * copia = get_copia_bloque_valida(bloque);
//
//			if (copia != NULL) {
//				t_nodo_self* nodo_destino;
//				if (destino == NULL) {
//					nodo_destino = find_nodo_disponible(bloque_id, archivo);
//				} else {
//					nodo_destino = find_nodo(destino);
//				}
//
//				if (nodo_destino != NULL) {
//					if (no_tiene_copia(archivo, bloque_id, nodo_destino)) {
//						t_copia_self * copia = create_copia(bloque_id,
//								nodo_destino, archivo->id);
//						if (nodo_destino->bloques_disponibles > 0) {
//							asignar_copia_a_bloque_archivo(bloque, copia);
//							if (bloque->estado == INVALIDO) {
//								chequear_alta_archivo(archivo, bloque_id);
//
//								if (archivo->estado == VALIDO)
//									txt_write_in_stdout(
//											"Se activo un archivo inactivo.\n");
//							}
//						} else {
//							txt_write_in_stdout(
//									"El nodo indicado no tiene espacio disponible.\n");
//						}
//					} else {
//						txt_write_in_stdout(
//								"El nodo indicado ya tiene una copia del bloque.\n");
//					}
//				} else {
//					txt_write_in_stdout("No se encontro el nodo indicado.\n");
//				}
//			} else {
//				txt_write_in_stdout("No se encontro una copia disponible.\n");
//			}
//		} else {
//			txt_write_in_stdout("No se encontro el bloque solicitado.\n");
//		}
//	} else {
//		txt_write_in_stdout("No se encontro el archivo solicitado.\n");
//	}
//}

int existe_copia_en_nodo(t_bloque_archivo_self* bloque, t_nodo_self* nodo) {
	int i;
	for (i = 0; i < bloque->copias->elements_count; i++) {
		t_copia_self* copia = list_get(bloque->copias, i);
		if (strcasecmp(copia->nodo_bloque->nodo->nodo_id, nodo->nodo_id) == 0)
			return true;
	}

	return false;
}

void copiar_bloque(char* nombre_nodo_origen, int bloque_origen, char* nombre_nodo_destino, int bloque_destino) {
//TODO: Copiar fisicamente al nodo indicado.

	if (bloque_origen <= TAMANIO_BLOQUE_NODO && bloque_destino <= TAMANIO_BLOQUE_NODO) {
		t_nodo_self * nodo_origen = find_nodo(nombre_nodo_origen);

		if (nodo_origen != NULL) {

			if (nodo_origen->estado == VALIDO) {
				t_copia_self * copia_origen = dictionary_get(nodo_origen->bloques, string_itoa(bloque_origen));

				if (copia_origen != NULL) {
					t_nodo_self* nodo_destino = find_nodo(nombre_nodo_destino);

					if (nodo_destino != NULL) {

						if (nodo_destino->estado == VALIDO) {
							int ocupado = false;
							t_archivo_self* archivo_origen = find_archivo_by_id(copia_origen->archivo_id, archivos);
							t_bloque_archivo_self* bloque_archivo = dictionary_get(archivo_origen->bloques,
									string_itoa(copia_origen->archivo_bloque));
							if (!existe_copia_en_nodo(bloque_archivo, nodo_destino)) {

								t_copia_self * copia_destino = dictionary_get(nodo_destino->bloques, string_itoa(bloque_destino));
//								bloquear_bloque(nodo_destino->nodo_id, bloque_destino, 1, 0);

								if (copia_destino != NULL) {
									int i;
									ocupado = true;
									t_archivo_self* archivo_destino = find_archivo_by_id(copia_destino->archivo_id, archivos);
									t_bloque_archivo_self * bloque_archivo_destino = dictionary_get(archivo_destino->bloques,
											string_itoa(copia_destino->archivo_bloque));
									int archivo_bloque_destino = copia_destino->archivo_bloque;
									int numero_copia_destino = copia_destino->numero;

									dictionary_put(bloques_nodo_disponible, nodo_destino->nodo_id,
											copia_destino->nodo_bloque->bloque_nodo);
									dictionary_remove(nodo_destino->bloques, string_itoa(bloque_destino));

									for (i = 0; i < bloque_archivo_destino->copias->elements_count; i++) {
										t_copia_self * copia_aux = list_get(bloque_archivo_destino->copias, i);
										if (copia_aux->numero == numero_copia_destino) {
											list_remove(bloque_archivo_destino->copias, i);
											break;
										}
									}

									t_list* archivos_nodo = dictionary_get(bloques_nodos_archivos, nodo_destino->nodo_id);

									int numero_copia = copia_destino->numero;

									for (i = 0; i < archivos_nodo->elements_count; i++) {
										t_bloque_archivo_control_self* bloque_archivo_control = list_get(archivos_nodo, i);
										if (strcasecmp(bloque_archivo_control->archivo_id, archivo_destino->id) == 0
												&& bloque_archivo_control->archivo_bloque == archivo_bloque_destino) {
											list_remove(archivos_nodo, i);
											break;
										}
									}

									if (numero_copia <= bloque_archivo_destino->copias->elements_count && bloque_archivo_destino->copias->elements_count > 0) {
										t_copia_self * copia_aux2 = list_get(bloque_archivo_destino->copias, bloque_archivo_destino->copias->elements_count - 1);
										copia_aux2->numero = numero_copia;
									}

									mongo_delete_2("nodo_id", nodo_destino->nodo_id, "nodo_bloque", bloque_destino, "bloques_nodo_copia");
									chequear_baja_archivo(archivo_destino, archivo_bloque_destino);
								}

								t_copia_self * duplicado = create_copia(copia_origen->archivo_bloque, nodo_destino,
										copia_origen->archivo_id, bloque_destino, ocupado,
										bloque_archivo->copias->elements_count + 1);
								asignar_copia_a_bloque_archivo(bloque_archivo, duplicado);

								txt_write_in_stdout("El bloque fue copiado exitosamente.\n");
							} else {
								txt_write_in_stdout("Ya existe una copia en nodo destino.\n");
							}
						} else {
							txt_write_in_stdout("El nodo destino esta inactivo.\n");
						}
					} else {
						txt_write_in_stdout("No se encontro el nodo destino indicado.\n");
					}
				} else {
					txt_write_in_stdout("No se encontro informacion en el bloque indicado.\n");
				}
			} else {
				txt_write_in_stdout("El nodo origen esta inactivo.\n");
			}
		} else {
			txt_write_in_stdout("No se encontro el nodo origen indicado.\n");
		}
	} else {
		txt_write_in_stdout("El bloque indicado no existe.\n");
	}
}

void mongo_insert_bson(bson_t *doc, char* collection_name) {
	mongoc_collection_t *collection;
	bson_oid_t oid;
	bson_error_t error;

	mongoc_client_t* mongo_client;
	mongoc_init();

	mongo_client = mongoc_client_new(MONGO_SERVER);
	collection = mongoc_client_get_collection(mongo_client, MONGO_DB, collection_name);

	bson_oid_init(&oid, NULL);
	BSON_APPEND_OID(doc, "_id", &oid);

	if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
		printf("%s\n", error.message);
	}

	bson_destroy(doc);
	mongoc_collection_destroy(collection);
	mongoc_client_destroy(mongo_client);
}

void mongo_insert(char* key, char* value, char* collection_name) {
	mongoc_collection_t *collection;
	bson_error_t error;
	bson_oid_t oid;
	bson_t *doc;

	mongoc_client_t* mongo_client;
	mongoc_init();

	mongo_client = mongoc_client_new(MONGO_SERVER);
	collection = mongoc_client_get_collection(mongo_client, MONGO_DB, collection_name);

	doc = bson_new();
	bson_oid_init(&oid, NULL);
	BSON_APPEND_OID(doc, "_id", &oid);
	BSON_APPEND_UTF8(doc, key, value);

	if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
		printf("%s\n", error.message);
	}

	bson_destroy(doc);
	mongoc_collection_destroy(collection);
	mongoc_client_destroy(mongo_client);
}

void mongo_update(char* key_to_search, char* value_to_search, char* key_to_update, char* value_to_update, char* collection_name, int type) {
	mongoc_collection_t *collection;
	bson_error_t error;
	bson_t *doc = NULL;
	bson_t *update = NULL;
	bson_t *query = NULL;

	mongoc_client_t* mongo_client;
	mongoc_init();

	mongo_client = mongoc_client_new(MONGO_SERVER);
	collection = mongoc_client_get_collection(mongo_client, MONGO_DB, collection_name);

	query = BCON_NEW(key_to_search, BCON_UTF8 (value_to_search));

	if (type == 0) {
		update = BCON_NEW("$set", "{", key_to_update, BCON_UTF8 (value_to_update), "}");
	} else if (type == 1) {
		update = BCON_NEW("$set", "{", key_to_update, BCON_INT32((int) strtol(value_to_update, (char **)NULL, 10)), "}");
	} else if (type == 2) {
		update = BCON_NEW("$set", "{", key_to_update, BCON_INT64((long) strtol(value_to_update, (char **)NULL, 10)), "}");
	}

	if (!mongoc_collection_update(collection, MONGOC_UPDATE_NONE, query, update, NULL, &error)) {
		printf("%s\n", error.message);
		goto fail;
	}

	fail: if (doc)
		bson_destroy(doc);
	if (query)
		bson_destroy(query);
	if (update)
		bson_destroy(update);

	mongoc_collection_destroy(collection);
	mongoc_client_destroy(mongo_client);
}

void mongo_update_2(char* key_to_search, char* value_to_search, char* key_to_search2, int value_to_search2,
		char* key_to_update, char* value_to_update, char* collection_name, int type) {
	mongoc_collection_t *collection;
	bson_error_t error;
	bson_t *doc = NULL;
	bson_t *update = NULL;
	bson_t *query = NULL;

	mongoc_client_t* mongo_client;
	mongoc_init();

	mongo_client = mongoc_client_new(MONGO_SERVER);
	collection = mongoc_client_get_collection(mongo_client, MONGO_DB, collection_name);

	query = BCON_NEW(key_to_search, BCON_UTF8 (value_to_search));
	BCON_APPEND(query, key_to_search2, BCON_INT32 (value_to_search2));

	if (type == 0) {
		update = BCON_NEW("$set", "{", key_to_update, BCON_UTF8 (value_to_update), "}");
	} else if (type == 1) {
		update = BCON_NEW("$set", "{", key_to_update, BCON_INT32((int) strtol(value_to_update, (char **)NULL, 10)), "}");
	} else if (type == 2) {
		update = BCON_NEW("$set", "{", key_to_update, BCON_INT64((long) strtol(value_to_update, (char **)NULL, 10)), "}");
	}

	if (!mongoc_collection_update(collection, MONGOC_UPDATE_NONE, query, update, NULL, &error)) {
		printf("%s\n", error.message);
		goto fail;
	}

	fail: if (doc)
		bson_destroy(doc);
	if (query)
		bson_destroy(query);
	if (update)
		bson_destroy(update);

	mongoc_collection_destroy(collection);
	mongoc_client_destroy(mongo_client);
}

void mongo_update_string(char* key_to_search, char* value_to_search, char* key_to_update, char* value_to_update,
		char* collection_name) {
	mongo_update(key_to_search, value_to_search, key_to_update, value_to_update, collection_name, 0);
}

void mongo_update_integer(char* key_to_search, char* value_to_search, char* key_to_update, int value_to_update,
		char* collection_name) {
	mongo_update(key_to_search, value_to_search, key_to_update, string_itoa(value_to_update), collection_name, 1);
}

void mongo_update_long(char* key_to_search, char* value_to_search, char* key_to_update, long value_to_update,
		char* collection_name) {
	mongo_update(key_to_search, value_to_search, key_to_update, string_itoa(value_to_update), collection_name, 2);
}

void mongo_delete(char* key, char* value, char* collection_name) {
	mongoc_collection_t *collection;
	bson_error_t error;
	bson_t *doc;
	mongoc_delete_flags_t  tipo_delete;

	mongoc_client_t* mongo_client;
	mongoc_init();

	mongo_client = mongoc_client_new(MONGO_SERVER);
	collection = mongoc_client_get_collection(mongo_client, MONGO_DB, collection_name);

	doc = bson_new();
	if (strcasecmp(key, "") != 0) {
		BSON_APPEND_UTF8(doc, key, value);
		tipo_delete = MONGOC_DELETE_SINGLE_REMOVE;
	} else
		tipo_delete = MONGOC_DELETE_NONE;



	if (!mongoc_collection_delete(collection, tipo_delete, doc, NULL, &error)) {
		printf("Delete failed: %s\n", error.message);
	}

	bson_destroy(doc);
	mongoc_collection_destroy(collection);
	mongoc_client_destroy(mongo_client);
}

void mongo_delete_2(char* key, char* value, char* key2, int value2, char* collection_name) {
	mongoc_collection_t *collection;
	bson_error_t error;
	bson_t *doc;

	mongoc_client_t* mongo_client;
	mongoc_init();

	mongo_client = mongoc_client_new(MONGO_SERVER);
	collection = mongoc_client_get_collection(mongo_client, MONGO_DB, collection_name);

	doc = bson_new();
	BSON_APPEND_UTF8(doc, key, value);
	BSON_APPEND_INT32(doc, key2, value2);

	if (!mongoc_collection_delete(collection, MONGOC_DELETE_SINGLE_REMOVE, doc, NULL, &error)) {
		printf("Delete failed: %s\n", error.message);
	}

	bson_destroy(doc);
	mongoc_collection_destroy(collection);
	mongoc_client_destroy(mongo_client);
}

char * mongo_get(char* key, char* compare_value, char* key_result, char* collection_name, int type) {
	mongoc_client_t *mongo_client;
	mongoc_collection_t *collection;
	mongoc_cursor_t *cursor;
	const bson_t *doc;
	bson_t *query;
	const bson_value_t *valor;
	bson_iter_t iter;
	const char * clave;
	char* resultado;

	mongoc_init();

	mongo_client = mongoc_client_new(MONGO_SERVER);
	collection = mongoc_client_get_collection(mongo_client, MONGO_DB, collection_name);
	query = bson_new();
	BSON_APPEND_UTF8(query, key, compare_value);

	cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
	while (mongoc_cursor_next(cursor, &doc)) {
		if (bson_iter_init(&iter, doc)) {
			while (bson_iter_next(&iter)) {
				clave = bson_iter_key(&iter);
				valor = bson_iter_value(&iter);

				if (strcasecmp(clave, key_result) == 0) {
					if (type == 0) {
						resultado = malloc(sizeof(char) * string_length(valor->value.v_utf8.str));
						resultado = valor->value.v_utf8.str;
					} else if (type == 1) {
						resultado = malloc(sizeof(char) * string_length(string_itoa(valor->value.v_int32)));
						resultado = string_itoa(valor->value.v_int32);
					} else if (type == 2) {
						resultado = malloc(sizeof(char) * string_length(string_itoa(valor->value.v_int64)));
						resultado = string_itoa(valor->value.v_int64);
					}
				}
			}
		}
	}
	bson_destroy(query);
	mongoc_cursor_destroy(cursor);
	mongoc_collection_destroy(collection);
	mongoc_client_destroy(mongo_client);

	return resultado;
}

t_list * mongo_get_list(char* key, char* compare_value, char* collection_name) {
	mongoc_client_t *mongo_client;
	mongoc_collection_t *collection;
	mongoc_cursor_t *cursor;
	const bson_t *doc;
	t_dictionary * lista;
	bson_t *query;
	const bson_value_t *valor;
	bson_iter_t iter;
	const char * clave;
	lista = list_create();

	mongoc_init();

	mongo_client = mongoc_client_new(MONGO_SERVER);
	collection = mongoc_client_get_collection(mongo_client, MONGO_DB, collection_name);
	query = bson_new();
	if (strcasecmp(key, "") != 0)
		BSON_APPEND_UTF8(query, key, compare_value);

	cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
	int i = 0;
	while (mongoc_cursor_next(cursor, &doc)) {
		if (bson_iter_init(&iter, doc)) {
			t_dictionary * resultado;
			resultado = dictionary_create();

			while (bson_iter_next(&iter)) {
				clave = bson_iter_key(&iter);
				valor = bson_iter_value(&iter);
				dictionary_put(resultado, clave, valor->value.v_utf8.str);
			}
			list_add(lista, resultado);
		}
		i++;
	}
	bson_destroy(query);
	mongoc_cursor_destroy(cursor);
	mongoc_collection_destroy(collection);
	mongoc_client_destroy(mongo_client);

	return lista;
}

int mongo_get_integer(char* key, char* compare_value, char* key_result, char* collection_name) {
	char * resultado;
	resultado = string_duplicate(mongo_get(key, compare_value, key_result, collection_name, 1));

	if (strcasecmp(resultado, "") != 0)
		return (int) strtol(resultado, (char **)NULL, 10);
	else
		return NULL;
}

long mongo_get_long(char* key, char* compare_value, char* key_result, char* collection_name) {
	char * resultado;
	resultado = string_duplicate(mongo_get(key, compare_value, key_result, collection_name, 2));

	if (strcasecmp(resultado, "") != 0)
		return (long) strtol(resultado, (char **)NULL, 10);
	else
		return NULL;
}

char* mongo_get_string(char* key, char* compare_value, char* key_result, char* collection_name) {
	char * resultado;
	resultado = string_duplicate(mongo_get(key, compare_value, key_result, collection_name, 2));

	if (strcasecmp(resultado, "") != 0)
		return resultado;
	else
		return NULL;
}

void cargar_nodos_prexistentes() {
	int i;
	t_list* nodos_aux = mongo_get_list("", "", "nodos");

	for (i = 0; i < nodos_aux->elements_count; i++) {
		t_dictionary* nodo_aux = list_get(nodos_aux, i);
		//Estado = 0 porque supuestamente no conecto todavia.
		t_nodo_self* nodo = crear_nodo(dictionary_get(nodo_aux, "nombre"), 0, dictionary_get(nodo_aux, "nodo_id"),
				dictionary_get(nodo_aux, "bloques_disponibles"));
		nodo->estado = 0;

		dictionary_destroy(nodo_aux);

		agregar_semaforo_nodo(nodo->nodo_id);

		add_nodo_to_nodos(nodo, 1);
	}
	list_destroy(nodos_aux);
}

void cargar_bloques_nodo_disponible_prexistentes() {
	int i, j;
	for (i = 0; i < nodos->elements_count; i++) {
		t_nodo_self * nodo = list_get(nodos, i);
		t_list* bloques_disponibles_aux = mongo_get_list("nodo_id", nodo->nodo_id, "bloques_nodo_disponible");
		t_list* lista_bloques_disponibles = list_create();
		for (j = 0; j < bloques_disponibles_aux->elements_count; j++) {
			t_dictionary* bloque_disponible_aux = list_get(bloques_disponibles_aux, j);
			int* numero;
			numero = malloc(sizeof(int));
			numero = (int) dictionary_get(bloque_disponible_aux, "numero");
			list_add(lista_bloques_disponibles, numero);
			dictionary_destroy(bloque_disponible_aux);
		}
		dictionary_put(bloques_nodo_disponible, nodo->nodo_id, lista_bloques_disponibles);

		list_destroy(bloques_disponibles_aux);
	}
}

void cargar_archivos_prexistentes() {
	int i, j;
	t_list* archivos_aux = mongo_get_list("", "", "archivos");

	for (i = 0; i < archivos_aux->elements_count; i++) {
		t_dictionary* archivo_aux = list_get(archivos_aux, i);
		t_archivo_self* archivo = crear_archivo(dictionary_get(archivo_aux, "nombre"), 0,
				dictionary_get(archivo_aux, "id"), dictionary_get(archivo_aux, "tamanio"),
				dictionary_get(archivo_aux, "bloques_invalidos"));
		archivo->cantidad_bloques = dictionary_get(archivo_aux, "cantidad_bloques");
		archivo->bloques_invalidos = archivo->cantidad_bloques;

		for (j = 0; j < archivo->cantidad_bloques; j++) {
			t_bloque_archivo_self* bloque = create_bloque(j, 0);
			asignar_bloque_archivo_a_archivo(archivo, bloque);
		}

		dictionary_destroy(archivo_aux);
		list_add(archivos, archivo);
	}

	list_destroy(archivos_aux);
}

void cargar_bloques_nodo_copia() {
	int i;
	t_list* bloques_nodo_copia_aux = mongo_get_list("", "", "bloques_nodo_copia");

	for (i = 0; i < bloques_nodo_copia_aux->elements_count; i++) {
		t_dictionary* bloque_nodo_copia_aux = list_get(bloques_nodo_copia_aux, i);

		t_nodo_bloque_self *nodo_bloque;
		nodo_bloque = malloc(sizeof(t_nodo_bloque_self));
		//TODO: En el elemento83 SF.
		char* nodo_id = string_duplicate(dictionary_get(bloque_nodo_copia_aux, "nodo_id"));
		nodo_bloque->nodo = find_nodo_by_id(nodo_id);
		nodo_bloque->bloque_nodo = dictionary_get(bloque_nodo_copia_aux, "nodo_bloque");

		t_copia_self* copia;
		copia = malloc(sizeof(t_copia_self));
		copia->archivo_bloque = dictionary_get(bloque_nodo_copia_aux, "archivo_bloque");
		copia->archivo_id = string_duplicate(dictionary_get(bloque_nodo_copia_aux, "archivo_id"));
		copia->nodo_bloque = nodo_bloque;
		copia->numero = dictionary_get(bloque_nodo_copia_aux, "numero_copia");

		dictionary_put(nodo_bloque->nodo->bloques, string_itoa(nodo_bloque->bloque_nodo), copia);

		t_bloque_archivo_control_self* control;
		control = malloc(sizeof(t_bloque_archivo_control_self));
		control->archivo_bloque = copia->archivo_bloque;
		control->archivo_id = string_duplicate(copia->archivo_id);
		t_list* lista = dictionary_get(bloques_nodos_archivos, nodo_bloque->nodo->nodo_id);
		list_add(lista, control);

		t_archivo_self* archivo = find_archivo_by_id(copia->archivo_id, archivos);
		asignar_copia_a_bloque_archivo(dictionary_get(archivo->bloques, string_itoa(copia->archivo_bloque)), copia);

		dictionary_destroy(bloque_nodo_copia_aux);
	}

	list_destroy(bloques_nodo_copia_aux);
}

void cargar_bloques_nodos_archivos_prexistentes() {

}

void cargar_directorios_prexistentes() {
	int i;
	t_list* directorios_aux = mongo_get_list("", "", "directorios");

	for (i = 0; i < directorios_aux->elements_count; i++) {
		t_dictionary* directorio_aux = list_get(directorios_aux, i);
		t_directorio_self* directorio = crear_directory(dictionary_get(directorio_aux, "nombre"), dictionary_get(directorio_aux, "padre"),
				dictionary_get(directorio_aux, "nivel"), dictionary_get(directorio_aux, "ruta"),
				dictionary_get(directorio_aux, "id"));

		dictionary_destroy(directorio_aux);

		list_add(directorios->directorios, directorio);
	}

	list_destroy(directorios_aux);
}

void cargar_datos_prexistentes() {
	cargar_directorios_prexistentes();
	cargar_nodos_prexistentes();
	cargar_bloques_nodo_disponible_prexistentes();
	cargar_archivos_prexistentes();
	cargar_bloques_nodo_copia();
	cargar_bloques_nodos_archivos_prexistentes();
}
