# SandBox

Para poder correr el codigo compatible con Linux o MacOS buscar instrucciones directamente en raylib wiki.
Para poder compilar el codigo en windows tener mingw32 con threads de wind32 instalado y activado correctamente en las variables del sistema. Igualmente buscar la información directamente en raylib wiki para mayor informacion.

Posteriormente, descargar el zip, o repositorio, y pegar esta linea de comando en cmd.

gcc main.c -o main.exe -O1 -Wall -std=c99 -Wno-missing-braces -I include/ -L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm

o utilizar el .bat para más rapido.
