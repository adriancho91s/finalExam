/*
- Fecha: 27 de noviembre 2023
- Hora: 1:44 am
- Versión del código: 3.8
- Presentado por: Adrián Fernando Gaitán Londoño Ing(c)
- Presentado a: Doctor, Ricardo Moreno Laverde
- Lenguaje: C
- Version del lenguaje: ISO/IEC 9899:2018 || C18
- Compilador: Apple clang version 14.0.0 (clang-1400.0.28.1)
- Descripción del programa: Programa que permite agregar registros a un archivo binario, imprimir el archivo binario como una tabla y ordenar los registros por cedula e imprimirlos como una tabla.
- Universidad Tecnológica de Pereira
- Programa de Ingeniería de Sistemas y Computación
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Union that allows conversion between a double and its byte representation.
 */
union DoubleToBytes {
    double d; /**< To get the value in integer. */
    char bytes[8]; /**< To get each the representation of each byte to write and read in the file. */
};

/**
 * @brief Union that allows conversion between an integer and its corresponding bytes.
 */
union IntToBytes {
    int i; /**< To get the value in integer. */
    char bytes[4]; /**< To get each the representation of each byte to write and read in the file. */
};


/**
 * Performs a binary search on the indexFile to find a specific cedula.
 * 
 * @param indexFile The file containing the index of cedulas.
 * @param countRegisters The file containing the count of registers.
 * @param cedula The cedula to search for.
 * @return 1 if the cedula is found, 0 otherwise.
 */
int binarySearch(FILE *indexFile, FILE *countRegisters, char *cedula) {
    //Read the count of registers
    fseek(countRegisters, 0L, SEEK_SET);
    union IntToBytes numberOfRegisters;
    for (int i = 0; i < 4; i++) {
        numberOfRegisters.bytes[i] = fgetc(countRegisters);
    }
    
    int start = 0;
    int end = numberOfRegisters.i - 1;
    int middle = 0;
    int found = 0;
    int position = 0;

    while (start <= end) {
        middle = (start + end) / 2;
        //Read the cedula of the current register
        fseek(indexFile, 0, SEEK_SET);
        for (int j = 0; j < middle; ) {
            char c = fgetc(indexFile);
            if (c == '}') {
                j++;
            }
        }
        char cedulaCurrent[256];
        int j = 0;
        char c;
        while ((c = fgetc(indexFile)) != '{' && c != EOF) {
            cedulaCurrent[j++] = c;
        }
        cedulaCurrent[j] = '\0';
        // printf("Cedula current: %s\n", cedulaCurrent);

        int cedulaCurrentInt = atoi(cedulaCurrent);
        int cedulaInt = atoi(cedula);
        // printf("Cedula: %s\n", cedula);
        // printf("Compare binary: %d\n", cedulaCurrentInt > cedulaInt);

        if (cedulaCurrentInt > cedulaInt) {
            end = middle - 1;
        } else if (cedulaCurrentInt < cedulaInt) {
            start = middle + 1;
        } else {
            found = 1;
            position = middle;
            break;
        }
    }

    if (found == 1) {
        return 1;
    } else {
        return 0;
    }

}

/**
 * Adds a new index record to the index file.
 * If the file is empty, the record is added at the beginning of the file.
 * Otherwise, the record is inserted in the correct position based on the cedula value.
 *
 * @param indexFile The index file to add the record to.
 * @param countRegisters The file containing the count of registers.
 * @param cedula The cedula value of the new record.
 * @param startByte The start byte value of the new record.
 * @param size The size value of the new record.
 * @return 0 if the record is successfully added, -1 if there is an error creating the temporary file.
 */
int addIndexFile(FILE *indexFile, FILE *countRegisters, char *cedula, int startByte, int size) {
    //If the file is empty, the register is added at the beginning of the file
    fseek(indexFile, 0L, SEEK_SET);
    if (fgetc(indexFile) == EOF) {
        //Write the cedula like a string
        for (int i = 0; i < strlen(cedula); i++) {
            fputc(cedula[i], indexFile);
        }
        fputc('{', indexFile);

        //Write the start byte
        union IntToBytes startByteInt;
        startByteInt.i = startByte;
        for (int i = 0; i < 4; i++) {
            fputc(startByteInt.bytes[i], indexFile);
        }
        fputc('{', indexFile);

        //Write the size
        union IntToBytes sizeInt;
        sizeInt.i = size;
        for (int i = 0; i < 4; i++) {
            fputc(sizeInt.bytes[i], indexFile);
        }
        fputc('}', indexFile);

        return 0;
    }

    //Read the count of registers
    fseek(countRegisters, 0L, SEEK_SET);
    union IntToBytes numberOfRegisters;
    for (int i = 0; i < 4; i++) {
        numberOfRegisters.bytes[i] = fgetc(countRegisters);
    }

    // Create a temporary file
    FILE *tempFile = fopen("tempIndexFile.tmp", "w+");
    if (tempFile == NULL) {
        return -1; // Error creating temporary file
    }

    // Find the correct position to insert the new record
    int position = 0;
    int found = 0;
    fseek(indexFile, 0L, SEEK_SET);
    for (int i = 0; i < numberOfRegisters.i; i++) {
        // Read the cedula of the current register
        for (int j = 0; j < i; ){
            char c = fgetc(indexFile);
            if (c == '}') {
                j++;
            }
        }
        
        //Cedula can be a variable number of bytes
        char cedulaCurrent[256];
        int j = 0;
        char c;
        while ((c = fgetc(indexFile)) != '{') {
            cedulaCurrent[j++] = c;
        }
        cedulaCurrent[j] = '\0';
        printf("Cedula current: %s\n", cedulaCurrent);
        // Compare the cedula of the current register with the cedula of the new register
        int cedulaCurrentInt = atoi(cedulaCurrent);
        int cedulaInt = atoi(cedula);
        
        // If the cedula of the current register is greater than the cedula of the new register, the new register must be inserted before the current register
        if (cedulaCurrentInt > cedulaInt) {
            position = ftell(indexFile) - strlen(cedulaCurrent) - 1;
            found = 1;
            break;
        }

    }
    // Write the new record in the temporary file
    fseek(indexFile, 0L, SEEK_SET);
    for (int i = 0; i < position; i++) {
        // printf("ENtre aquí\n");
        fputc(fgetc(indexFile), tempFile);
    }

    // Write the new record
    for (int i = 0; i < strlen(cedula); i++) {
        fputc(cedula[i], tempFile);
    }
    fputc('{', tempFile);

    union IntToBytes startByteInt;
    startByteInt.i = startByte;
    for (int i = 0; i < 4; i++) {
        fputc(startByteInt.bytes[i], tempFile);
    }
    fputc('{', tempFile);

    union IntToBytes sizeInt;
    sizeInt.i = size;
    for (int i = 0; i < 4; i++) {
        fputc(sizeInt.bytes[i], tempFile);
    }
    fputc('}', tempFile);

    // Write the remaining records from the index file to the temporary file
    fseek(indexFile, position, SEEK_SET);
    while (!feof(indexFile)) {
        fputc(fgetc(indexFile), tempFile);
    }

    // Write each byte of the temporary file back to the index file
    fseek(tempFile, 0L, SEEK_SET);
    fseek(indexFile, 0L, SEEK_SET);
    int c;
    while ((c = fgetc(tempFile)) != EOF) {
        fputc(c, indexFile);
    }

    // Close and remove the temporary file
    fclose(tempFile);
    remove("tempIndexFile.tmp");

    return 0;
}
    
    
int addRegister() {

    int adding = 1;
    do {
        FILE *file, *indexFile, *countRegisters;

        //Open the files
        file = fopen("recordsDb.dat", "r+b");
        indexFile = fopen("recordsIndex.dat", "r+b");
        countRegisters = fopen("countRegisters.dat", "r+b");

        //If the files dont exist, create them
        if (file == NULL) {
            file = fopen("recordsDb.dat", "w+b");
            if (file == NULL) {
                printf("No se pudo crear el archivo. recordsDb.dat\n");
                return 1;
            }
        }
        if (indexFile == NULL) {
            indexFile = fopen("recordsIndex.dat", "w+b");
            if (indexFile == NULL) {
                printf("No se pudo crear el archivo. recordsIndex.dat\n");
                return 1;
            }
        }
        if (countRegisters == NULL) {
            countRegisters = fopen("countRegisters.dat", "w+b");
            if (countRegisters == NULL) {
                printf("No se pudo crear el archivo. countRegisters.dat\n");
                return 1;
            }
        }
        //Ask for the data of the register
        //Includes spaces in the strings
        char cedula[256];
        char nombre[256];
        char apellidos[256];
        char direccion[256];
        double sueldo;

        printf("Cedula: ");
    scanf("%s", cedula);

    printf("Nombre: ");
    scanf(" %[^\n]", nombre);  // Note the space before %[^\n]

    printf("Apellidos: ");
    scanf(" %[^\n]", apellidos);

    printf("Direccion: ");
    scanf(" %[^\n]", direccion);

    printf("Sueldo: ");
    scanf("%lf", &sueldo);

        //Binary search in index file to know if the register exists
        //Start byte of the register in recordsDb.dat
        int startByte = 0;
        //Size of the register
        int size = 0;
        //For know if the register exists
        int exists = binarySearch(indexFile, countRegisters, cedula);

        // int exists = 0;
        if (exists == 1) {
            printf("\nEl registro ya existe en la base de datos\n");
            return 1;
        }
        if (exists == 2) {
            printf("An error\n");
            return 1;
        }

        //Write the register in recordsDb.dat appending at the end of the file
        fseek(file, 0L, SEEK_END);
        startByte = ftell(file);
        //Calculate the size of the register
        printf("Cedula strlen:%lu\n", strlen(cedula));
        printf("Nombre strlen:%lu\n", strlen(nombre));
        printf("Apellidos strlen:%lu\n", strlen(apellidos));
        printf("Direccion strlen:%lu\n", strlen(direccion));
        printf("Sueldo:%lu\n", sizeof(sueldo));
        size = strlen(cedula) + strlen(nombre) + strlen(apellidos) + strlen(direccion) + 8 + 5;
        printf("Start byte: %d\n", startByte);
        printf("Size: %d\n", size);
        //Write the cedula
        for (int i = 0; i < strlen(cedula); i++) {
            fputc(cedula[i], file);
        }
        //Write the nombre
        fputc('{', file);// 'Ç'
        for (int i = 0; i < strlen(nombre); i++) {
            fputc(nombre[i], file);
        }
        //Write the apellidos
        fputc('{', file);
        for (int i = 0; i < strlen(apellidos); i++) {
            fputc(apellidos[i], file);
        }
        //Write the direccion
        fputc('{', file);
        for (int i = 0; i < strlen(direccion); i++) {
            fputc(direccion[i], file);
        }
        //Write the sueldo
        fputc('{', file);
        union DoubleToBytes sueldoBytes;
        sueldoBytes.d = sueldo;
        for (int i = 0; i < 8; i++) {
            fputc(sueldoBytes.bytes[i], file);
        }
        //Write the end of the register
        fputc('}', file);// 'ƒ'

        
        //Add into index file sorting by id
        fseek(indexFile, 0L, SEEK_END);
        fseek(countRegisters, 0L, SEEK_END);
        addIndexFile(indexFile, countRegisters, cedula, startByte, size);

        fseek(countRegisters, 0L, SEEK_SET);
        
        union IntToBytes numberOfRegisters;
        //If the file is empty, the count is 0
        if (fgetc(countRegisters) == EOF) {
            numberOfRegisters.i = 1;
        } else {
            //Read the count of registers
            fseek(countRegisters, 0L, SEEK_SET);
            for (int i = 0; i < 4; i++) {
                numberOfRegisters.bytes[i] = fgetc(countRegisters);
            }
        }
        printf("Counter: %d\n", numberOfRegisters.i);
        numberOfRegisters.i = numberOfRegisters.i + 1;
        //Write the count of registers
        fseek(countRegisters, 0L, SEEK_SET);
        for (int i = 0; i < 4; i++) {
            fputc(numberOfRegisters.bytes[i], countRegisters);
        }
        //Read the index file
        char c;

        //Ask if the user wants to add another register
        printf("Do you want to add another register? 1. Yes 0. No\n");
        scanf("%d", &adding);

        union IntToBytes prueba;
        fseek(countRegisters, 0L, SEEK_SET);
        for(int i = 0; i < 4; i++) {
            prueba.bytes[i] = fgetc(countRegisters);
        }

        printf("Counter: %d\n", prueba.i);
        fclose(file);
        fclose(indexFile);
        fclose(countRegisters);
    } while (adding == 1);

    return 0;

}


int printEmpleadosDat() {
    FILE *file;
    file = fopen("recordsDb.dat", "r+b");
    if (file == NULL) {
        printf("No se pudo abrir el archivo. recordsDb.dat\n");
        return 1;
    }

    char cedula[100], nombre[100], apellidos[100], direccion[100];
    double sueldo;
    int c, i;

    printf("%-20s%-20s%-20s%-30s%-10s\n", "Cedula", "Nombre", "Apellidos", "Direccion", "Sueldo");
    printf("-------------------------------------------------------------------------------------------------------\n");

    while ((c = fgetc(file)) != EOF) {
        i = 0;
        while (c != '{' && c != EOF) {
            cedula[i++] = c;
            c = fgetc(file);
        }
        cedula[i] = '\0';

        i = 0;
        c = fgetc(file);
        while (c != '{' && c != EOF) {
            nombre[i++] = c;
            c = fgetc(file);
        }
        nombre[i] = '\0';

        i = 0;
        c = fgetc(file);
        while (c != '{' && c != EOF) {
            apellidos[i++] = c;
            c = fgetc(file);
        }
        apellidos[i] = '\0';

        i = 0;
        c = fgetc(file);
        while (c != '{' && c != EOF) {
            direccion[i++] = c;
            c = fgetc(file);
        }
        direccion[i] = '\0';

        fread(&sueldo, sizeof(double), 1, file);

        printf("%-20s%-20s%-20s%-30s%.2lf\n", cedula, nombre, apellidos, direccion, sueldo);

        // Skip '}' and the next '{'
        fgetc(file);
    }

    fclose(file);

    return 0;
}

int printfSortedRegisters() {
    FILE *file, *indexFile, *countRegisters;

    // Open the files
    file = fopen("recordsDb.dat", "r+b");
    indexFile = fopen("recordsIndex.dat", "r+b");
    countRegisters = fopen("countRegisters.dat", "r+b");

    if (file == NULL || indexFile == NULL || countRegisters == NULL) {
        printf("Failed to open files.\n");
        return 1;
    }

    char cedula[100], nombre[100], apellidos[100], direccion[100];
    double sueldo;

    int c;

    printf("%-20s %-20s %-23s %-20s %-20s \n", "Cedula", "Nombre", "Apellidos", "Direccion", "Sueldo");
    printf("-------------------------------------------------------------------------------------------\n");

    // Read the count of registers
    fseek(countRegisters, 0L, SEEK_SET);
    union IntToBytes numberOfRegisters;
    for (int i = 0; i < 4; i++) {
        numberOfRegisters.bytes[i] = fgetc(countRegisters);
    }

    // Read the index file
    fseek(indexFile, 0L, SEEK_SET);

    for (int i = 0; i < numberOfRegisters.i - 1; i++) {
        // Read the cedula of the current register
        char cedulaIndex[256];
        int j = 0;
        char c;
        while ((c = fgetc(indexFile)) != '{' && c != EOF) {
            cedulaIndex[j++] = c;
        }
        cedulaIndex[j] = '\0';

        // Read the start byte of the current register
        union IntToBytes startByteCurrent;
        for (int j = 0; j < 4; j++) {
            startByteCurrent.bytes[j] = fgetc(indexFile);
        }

        c = fgetc(indexFile); // Skip '{'

        // Read the size of the current register
        union IntToBytes sizeCurrent;
        for (int j = 0; j < 4; j++) {
            sizeCurrent.bytes[j] = fgetc(indexFile);
        }
        c = fgetc(indexFile); // Skip '}'

        // Read the register
        fseek(file, startByteCurrent.i, SEEK_SET);

        // Read the cedula
        j = 0;
        while ((c = fgetc(file)) != '{' && c != EOF) {
            cedula[j++] = c;
        }
        cedula[j] = '\0';

        // Read the nombre
        j = 0;
        while ((c = fgetc(file)) != '{' && c != EOF) {
            nombre[j++] = c;
        }
        nombre[j] = '\0';

        // Read the apellidos
        j = 0;
        while ((c = fgetc(file)) != '{' && c != EOF) {
            apellidos[j++] = c;
        }
        apellidos[j] = '\0';

        // Read the direccion
        j = 0;
        while ((c = fgetc(file)) != '{' && c != EOF) {
            direccion[j++] = c;
        }
        direccion[j] = '\0';


        // Read the sueldo
        union DoubleToBytes sueldoBytes;
        for (int j = 0; j < 8; j++) {
            sueldoBytes.bytes[j] = fgetc(file);
        }
        sueldo = sueldoBytes.d;

        printf("%-20s %-20s %-23s %-20s %-6.2lf\n", cedula, nombre, apellidos, direccion, sueldo);

        // Skip '}' and the next '{'
        fgetc(file);

        // Move to the end of the current record in the index file
        
    }

    fclose(file);
    fclose(indexFile);
    fclose(countRegisters);

    return 0;
}



int main() {

    int count = 0;

    //Menu options
    int option = 0;
    printf("0. Salir de la aplicación\n");
    printf("1. Entrar registros a “maestro.dat”\n");
    printf("2. Imprimir “maestro.dat” como fue entrado debidamente tabulada la información\n");
    printf("3. Imprimir “maestro.dat” ordenado por cedula de menor a mayor apoyándose en el archivo\nindexado por cedula y debidamente tabulada la información\n");
    printf("\nOption: ");
    scanf("%d", &option);

    do {
        switch (option) {
            case 0:
                printf("Exiting...\n");
                break;
            case 1:
                // printf("Add a register function\n");
                addRegister();
                break;
            case 2:
                // printf("Print recordsDb.dat file like a table\n");
                printEmpleadosDat();
                break;
            case 3:
                // printf("Print the registers sorted by id like a table using the index file to print it in order\n");

                printfSortedRegisters();
                break;
            default:
                printf("Invalid option\n");
                break;
        }
        printf("\n0. Salir de la aplicación\n");
        printf("1. Entrar registros a “maestro.dat”\n");
        printf("2. Imprimir “maestro.dat” como fue entrado debidamente tabulada la información\n");
        printf("3. Imprimir “maestro.dat” ordenado por cedula de menor a mayor apoyándose en el archivo\nindexado por cedula y debidamente tabulada la información\n");
        printf("\nOption: ");
        scanf("%d", &option);
    } while (option != 0);

    return 0;
}
