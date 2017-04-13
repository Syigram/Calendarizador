# Calendarizador
Proyecto I - Calendarizador de Procesos


Descripción de contenidos de la carpeta PintOS/

Devices: Contiene el código fuente para la interfaz de comunicación con los dispositivos de entrada y salida, tales como VGA, telcado, disco, etc.

Filesys: Código fuente para el sistema de archivos básico del sistema.

lib: Contiene algunos códigos fuente de la biblioteca C. También cuenta con las carpetas kernel y user, en donde se realizaran las compilaciones para programas de usuario y de kernel. También se incluyen definiciones de tipos de datos utilizables en el código de kernel.

misc: Contiene parches y archivos para ayudar a debuggear PintOS.

threads: Código fuente base del kernel. Contiene la definición de hilos, alocaciones de memoria, entrada y salida, y llamadas de inicio del sistema.

userprog: Código fuente para los programas de usuario, además contiene algunas declaraciones como syscall.

utils: Archivos para facilitar la instalación de PintOS en un computador propio.

vm: archivos para memoria virtual.


Archivos dentro de /threads


Makefile: Una copia de "pintos/src/Makefile.build". Describe cómo compilar el kernel.

kernel.o: Object file para todo el kernel. Es el resultado de unir los object files compilados a partir de cada archivo fuente del kernel, en un único object file. Contiene información para debug.

kernel.bin: Imagen de memoria del kernel, esto significa, los bytes exactos que son cargados a memoria para ejecutar el Pintos kernel. Esto es como el "kernel.o" sin la información de debug, lo cual ahorra bastante espacio.

loader.bin: Imagen de memoria para el kernel loader, una pequeña porción de código es escrita en lenguaje ensamblador que lee el kernel del disco a memoria y lo inicializa. Mide exactamente 512 bytes de largo, un tamaño fijado por el PC BIOS.
