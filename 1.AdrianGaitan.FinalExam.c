/*
- Fecha: 26 de noviembre 2023
- Hora: 2:30 pm
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

union DoubleToBytes {
    double d;
    char bytes[8];
};

union IntToBytes {
    int i;
    char bytes[4];
};

int binarySearch(FILE *indexFile, FILE *countRegisters, char *cedula) {//Each index record have this structure: cedula: int (4 bytes) { startByte: int (4 bytes) { size: int (4 bytes) } -> 15 bytes
    //If the file is empty, the register doesnt exist
    if (fgetc(indexFile) == EOF) {
        return 0;
    }

    //Read the count of registers
    fseek(countRegisters, 0L, SEEK_SET);
    union IntToBytes numberOfRegisters;
    for (int i = 0; i < 4; i++) {
        numberOfRegisters.bytes[i] = fgetc(countRegisters);
    }

    int left = 0;
    int right = numberOfRegisters.i - 1;
    int middle = 0;
    int found = 0;
    int cedulaInt = atoi(cedula);
    
    while (left <= right) {
        middle = (left + right) / 2;
        //Read the cedula of the middle register
        fseek(indexFile, middle * 15, SEEK_SET);
        union IntToBytes cedulaMiddle;
        for (int i = 0; i < 4; i++) {
            cedulaMiddle.bytes[i] = fgetc(indexFile);
        }
        //Compare the cedula of the middle register with the cedula of the register to search
        if (cedulaInt == cedulaMiddle.i) {
            found = 1;
            break;
        } else if (cedulaInt < cedulaMiddle.i) {
            right = middle - 1;
        } else {
            left = middle + 1;
        }
    }
    if (found == 1) {
        return 1;
    } else {
        return 0;
    }
}

int addIndexFile(FILE *indexFile, FILE *countRegisters, char *cedula, int startByte, int size) {
    //If the file is empty, the register is added at the beginning of the file
    if (fgetc(indexFile) == EOF) {
        //Write the cedula
        union IntToBytes cedulaInt;
        cedulaInt.i = atoi(cedula);
        for (int i = 0; i < 4; i++) {
            fputc(cedulaInt.bytes[i], indexFile);
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
    for (int i = 0; i < numberOfRegisters.i; i++) {
        // Read the cedula of the current register
        fseek(indexFile, i * 15, SEEK_SET);
        union IntToBytes cedulaCurrent;
        for (int j = 0; j < 4; j++) {
            cedulaCurrent.bytes[j] = fgetc(indexFile);
        }

        // Compare the cedula of the current register with the cedula of the new record
        if (atoi(cedula) < cedulaCurrent.i) {
            position = i;
            found = 1;
            break;
        }
    }

    // Write the new record in the temporary file
    fseek(indexFile, 0L, SEEK_SET);
    for (int i = 0; i < position * 15; i++) {
        fputc(fgetc(indexFile), tempFile);
    }

    // Write the new record
    union IntToBytes cedulaInt;
    cedulaInt.i = atoi(cedula);
    for (int i = 0; i < 4; i++) {
        fputc(cedulaInt.bytes[i], tempFile);
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
    fseek(indexFile, position * 15, SEEK_SET);
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

    // // Ensure the last character is '}'
    // fseek(indexFile, -1, SEEK_END);
    // if (fgetc(indexFile) != '}') {
    //     fputc('}', indexFile);
    // }

    // Close and remove the temporary file
    fclose(tempFile);
    remove("tempIndexFile.tmp");


    return 0;
}
    
    


    /*
    The structure of each record is:
    Cedula: char caracteres alfanuméricos, tamaño variable
    {
    Nombre: char caracteres alfanuméricos, tamaño variable
    {
    Apellidos: char caracteres alfanuméricos, tamaño variable
    {
    Direccion: char de caracteres alfanuméricos, tamaño variable
    {
    Sueldo: double (8 bytes)
    }

    The structure of the index is:
    cedula: int (4 bytes) this is the cedula, but converts into integer from the string}
    {
    startByte: int (4 bytes)
    {
    size: int (4 bytes)
    }

    The structure of the count register is:
    count: int (4 bytes) (the number of registers in the main file)

    The program must be able to:
    Add a register
    Delete a register
    Modify a register
    Search a register by id
    */
int addRegister() {
    //Binary search in index file to know if the register exists
    // if it exists, the register cant be added and the function ends
    // if it doesnt exist, the register can be added
    //The index is sorted by id of the register and the id is unique

    //The index must be sort in every register added, so the index must be readed and sorted every time a register is going to be added

    //The program must ask for the data of the register
    //Every time question if the user wants to add another register


    int adding = 1;
    do {
        FILE *file, *indexFile, *countRegisters;

        //Open the files
        file = fopen("registros.dat", "r+b");
        indexFile = fopen("registrosIndex.dat", "r+b");
        countRegisters = fopen("countRegisters.dat", "r+b");

        //If the files dont exist, create them
        if (file == NULL) {
            file = fopen("registros.dat", "w+b");
            if (file == NULL) {
                printf("No se pudo crear el archivo. registros.dat\n");
                return 1;
            }
        }
        if (indexFile == NULL) {
            indexFile = fopen("registrosIndex.dat", "w+b");
            if (indexFile == NULL) {
                printf("No se pudo crear el archivo. registrosIndex.dat\n");
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
        scanf("%s", nombre);
        printf("Apellidos: ");
        scanf("%s", apellidos);
        printf("Direccion: ");
        scanf("%s", direccion);
        printf("Sueldo: ");
        scanf("%lf", &sueldo);

        //Binary search in index file to know if the register exists
        //Start byte of the register in registros.dat
        int startByte = 0;
        //Size of the register
        int size = 0;
        //For know if the register exists
        int exists = binarySearch(indexFile, countRegisters, cedula);
        if (exists == 1) {
            printf("The register exists\n");
            return 1;
        }
        if (exists == 2) {
            printf("An error\n");
            return 1;
        }

        //Write the register in registros.dat appending at the end of the file
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
    file = fopen("registros.dat", "r+b");
    if (file == NULL) {
        printf("No se pudo abrir el archivo. registros.dat\n");
        return 1;
    }

    char cedula[100], nombre[100], apellidos[100], direccion[100];
    double sueldo;
    int c, i;

    printf("Cedula\tNombre\tApellidos\tDireccion\tSueldo\n");

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

        printf("%s\t%s\t%s\t%s\t%.2lf\n", cedula, nombre, apellidos, direccion, sueldo);

        // Skip '}' and the next '{'
        fgetc(file);
    }

    fclose(file);

    return 0;
}

 // Create this funtion to print the registers sorted by id like a table using the index file to print it in order

int printfSortedRegisters( ) {

    FILE *file, *indexFile, *countRegisters;

    //Open the files
    file = fopen("registros.dat", "r+b");
    indexFile = fopen("registrosIndex.dat", "r+b");
    countRegisters = fopen("countRegisters.dat", "r+b");

    char cedula[100], nombre[100], apellidos[100], direccion[100];
    double sueldo;

    int c, i;

    printf("Cedula\tNombre\tApellidos\tDireccion\tSueldo\n");

    //Read the count of registers
    fseek(countRegisters, 0L, SEEK_SET);
    union IntToBytes numberOfRegisters;
    for (int i = 0; i < 4; i++) {
        numberOfRegisters.bytes[i] = fgetc(countRegisters);
    }

    //Read the index file
    fseek(indexFile, 0L, SEEK_SET);
    //Get the start byte and the size of each register
    int startByte = 0;
    int size = 0;

    printf("Counter: %d\n", numberOfRegisters.i);

    for (int i = 0; i < numberOfRegisters.i; i++) {
        //Read the cedula of the current register
        fseek(indexFile, i * 15, SEEK_SET);
        union IntToBytes cedulaCurrent;
        for (int j = 0; j < 4; j++) {
            cedulaCurrent.bytes[j] = fgetc(indexFile);
        }
        //Read the start byte of the current register
        fseek(indexFile, i * 15 + 5, SEEK_SET);
        union IntToBytes startByteCurrent;
        for (int j = 0; j < 4; j++) {
            startByteCurrent.bytes[j] = fgetc(indexFile);
        }
        //Read the size of the current register
        fseek(indexFile, i * 15 + 10, SEEK_SET);
        union IntToBytes sizeCurrent;
        for (int j = 0; j < 4; j++) {
            sizeCurrent.bytes[j] = fgetc(indexFile);
        }
        //Read the register
        fseek(file, startByteCurrent.i, SEEK_SET);
        //Read the cedula
        i = 0;
        while ((c = fgetc(file)) != '{' && c != EOF) {
            cedula[i++] = c;
        }
        cedula[i] = '\0';

        //Read the nombre
        i = 0;
        while ((c = fgetc(file)) != '{' && c != EOF) {
            nombre[i++] = c;
        }
        nombre[i] = '\0';

        //Read the apellidos
        i = 0;
        while ((c = fgetc(file)) != '{' && c != EOF) {
            apellidos[i++] = c;
        }
        apellidos[i] = '\0';

        //Read the direccion
        i = 0;
        while ((c = fgetc(file)) != '{' && c != EOF) {
            direccion[i++] = c;
        }
        direccion[i] = '\0';

        //Read the sueldo
        fread(&sueldo, sizeof(double), 1, file);

        printf("%s\t%s\t%s\t%s\t%.2lf\n", cedula, nombre, apellidos, direccion, sueldo);

        // Skip '}' and the next '{'
        fgetc(file);

        
    }

    fclose(file);
    fclose(indexFile);
    fclose(countRegisters);

    return 0;
}



int main() {
    //Add register appending at the end of the file
    
    //First, a binary search in index file to know if the register exists
    //All the files are a variable number of bytes, the registers are separated by Ç, and the end of each register is a ƒ symbol, just the count register has a fixed size of 4 bytes
    //The index contains the start byte of the register and the size of the register and the id of the register, to know if the register exists
    //If it exists, the register cant be added
    //If it doesnt exist, the register can be added
    //The index is sorted by id of the register and the id is unique
    //The index is a binary file
    //The index must be sort in every register added, so the index must be readed and sorted every time a register is going to be added
    /*
    The structure of each record is:
    Cedula: char caracteres alfanuméricos, tamaño variable
    Nombre: char caracteres alfanuméricos, tamaño variable
    Apellidos: char caracteres alfanuméricos, tamaño variable
    Direccion: char de caracteres alfanuméricos, tamaño variable
    Sueldo: double (8 bytes)

    The structure of the index is:
    cedula: variable number of bytes (the size of the id is the size of the cedula)
    startByte: int (4 bytes)
    size: int (4 bytes)

    The structure of the count register is:
    count: int (4 bytes) (the number of registers in the main file)

    The program must be able to:
    Add a register
    Delete a register
    Modify a register
    Search a register by id

    The program must have a menu with the options:
    0. Exit
    1. Add a register (the program must ask for the data of the register) and every time question if the user wants to add another register
    2. Print registros.dat file like a table
    3. Print the registers sorted by id like a table using the index file to print it in order
    */

    int count = 0;

    //Menu options
    int option = 0;
    printf("0. Exit\n");
    printf("1. Add a register\n");
    printf("2. Print registros.dat file like a table\n");
    printf("3. Print the registers sorted by id like a table using the index file to print it in order\n");

    scanf("%d", &option);

    do {
        switch (option) {
            case 0:
                printf("Exiting...\n");
                break;
            case 1:
                printf("Add a register function\n");

                addRegister();
                break;
            case 2:
                printf("Print registros.dat file like a table\n");
                //Print registros.dat file like a table

                printEmpleadosDat();
                break;
            case 3:
                printf("Print the registers sorted by id like a table using the index file to print it in order\n");
                //Print the registers sorted by id like a table using the index file to print it in order
                printfSortedRegisters();
                break;
            default:
                printf("Invalid option\n");
                break;
        }
        printf("0. Exit\n");
        printf("1. Add a register\n");
        printf("2. Delete a register\n");
        printf("3. Modify a register\n");
        scanf("%d", &option);
    } while (option != 0);






    // fclose(archivo);
    // fclose(archivoIndex);
    return 0;
}
