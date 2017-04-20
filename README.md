# Calendarizador
## Proyecto I - Calendarizador de Procesos


### Descripción de contenidos de la carpeta PintOS

**Devices:** Contiene el código fuente para la interfaz de comunicación con los dispositivos de entrada y salida, tales como VGA, teclado, disco, etc. También se incluye la definición del timer, el cual controla los ciclos de reloj del sistema operativo mediante ticks, con una frecuencia establecida.

**Filesys:** Código fuente para el sistema de archivos básico del sistema.

**lib:** Contiene algunos códigos fuente de la biblioteca C. También cuenta con las carpetas kernel y user, en donde se realizaran las compilaciones para programas de usuario y de kernel. Además, se incluyen definiciones de tipos de datos utilizables en el código de kernel.

**misc:** Contiene parches y archivos para ayudar a debuggear PintOS.

**threads:** Código fuente base del kernel. Contiene la definición de hilos, funciones de petición de memoria, entrada y salida, y llamadas de inicio del sistema.

**userprog:** Código fuente para los programas de usuario, además contiene algunas declaraciones como syscall.

**utils:** Archivos para la instalación y ejecución de PintOS en el computador.

**vm:** archivos para memoria virtual.



### Archivos dentro de /threads

**Fixedpoint.h:** Biblioteca y definición de tipos para las operaciones de punto flotante utilizadas para cálculos de variables de algoritmos Shortest-Job-First y Colas Multinivel.

**init.c:** Es el archivo principal de PintOS para el arranque del sistema, se encarga de parsear las opciones que se introducen al momento de la ejecución y se encarga de llamar a los archivos correspondientes.

**interrupt.c:** Definición de los PICS programables, y las variables necesarias para generar una interrupción al sistema.

**intr-stubs.S:** Punto principal de entrada a las interrupciones, funciona como interfaz de interrupciones externas e internas con el manejador de interrupciones del sistema operativo.

**io.h:** Funciones de entrada y salida de tipos de datos en diferentes direcciones de memoria.

**kernel.lds.S:** Se encarga del manejo de kernel y su ubicación en la memoria.

**loader.S:** Carga el kernel del sistema operativo.

**Makefile:** Una copia de "pintos/src/Makefile.build". Describe cómo compilar el kernel y permite utilizar el comando make para realizar dicha compilación.

**malloc.c:** Una implementación de la función malloc. Se encarga de buscar memoria para un programa que lo solicite.

**palloc.c:** Implementación del palloc. Se encarga de buscar memoria en tamaños de páginas definidos.

**pte.h:** Funciones y macros para trabajar con tablas de páginas de hardware de arquitectura x86.

**start.S:** Se llama cuando se ha cargado el kernel. Se encarga de llamar la función main, iniciando el sistema.

**switch.S:** Se encarga de cambiar el thread que está ejecutándose por el siguiente a ejecutar. También se encarga de manejar todo el cambio de contexto implicado en el cambio de los threads.

**synch.c:** Contiene la definición e implementación de funciones de sincronización y semáforos usados en el SO.

**thread.c:** Contiene la definición de un thread, y todas las funciones del manejo de los threads. Además, se encarga de manejar las interrupciones relacionadas con los threads del sistema. También cuenta con la implementación de los algoritmos calendarizadores del sistema: First-Come-First-Serve, Shortest-Job-First, Round Robin, y Colas Multinivel.

**vaddr.h:** Funciones y macros para trabajar con direcciones de memoria virtuales.

**/build/kernel.o:** Object file para todo el kernel. Es el resultado de unir los object files compilados a partir de cada archivo fuente del kernel, en un único object file. Contiene información para debug.

**/build/kernel.bin:** Imagen de memoria del kernel, esto significa, los bytes exactos que son cargados a memoria para ejecutar el Pintos kernel. Esto es como el "kernel.o" sin la información de debug, lo cual ahorra bastante espacio.

**/build/loader.bin:** Imagen de memoria para el kernel loader, una pequeña porción de código es escrita en lenguaje ensamblador que lee el kernel del disco a memoria y lo inicializa. Mide exactamente 512 bytes de largo, un tamaño fijado por el PC BIOS.

