/*

En tu código, no veo ninguna constante específica que se deba representar con un enum. Los enum son útiles cuando deseas asignar nombres significativos a valores enteros constantes. Por lo general, se utilizan cuando quieres proporcionar una semántica más clara para valores que pueden tener varios significados.

En tu código, estás manejando errores, y estás utilizando mensajes personalizados y valores constantes como 1 para indicar errores. Si deseas hacer que los valores de error sean más legibles y significativos, podrías definir un enum para representar los códigos de error, por ejemplo:

*/


enum ErrorCode {
    ERROR_NONE = 0,      // Sin error
    ERROR_FORK = 1,      // Error al crear un proceso hijo
    ERROR_CHDIR = 2,     // Error al cambiar el directorio
    ERROR_UNKNOWN_PATH = 3,  // Ruta desconocida
    // Agregar otros códigos de error según sea necesario
};

void fork_failed() {
    perror("An error has occurred with the fork");
    exit(ERROR_FORK);
}

void unknown_path(int i) {
    printf("The path %d doesn't exist \n", i);
    exit(ERROR_UNKNOWN_PATH);
}
