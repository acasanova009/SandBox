#include <raylib.h>


#include <stdlib.h>         // Requerido para: malloc(), free()
#include <math.h>           // Requerido para: sinf()
#include <string.h>         // Requerido para: memcpy()

#include <time.h>

#define MAX_MUESTRAS               512
#define MAX_MUESTRAS_POR_FRAME   4096

#define MAX_SENOS 1000

#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include "extras/raygui.h"                 // Requerido para controles GUI 

float seleccionarNota(float perilla5);
float valorRandomAEscala(float perilla5);

int main(void)
{
    // Inicializacion 
    //--------------------------------------------------------------------------------------
    const int pantallaAncho = 1200;
    const int pantallaAlto = 600;

    InitWindow(pantallaAncho, pantallaAlto, "Sintetizador de audio");

    InitAudioDevice();              // Inicializacion de dispositivo de audio

    SetAudioStreamBufferSizeDefault(MAX_MUESTRAS_POR_FRAME);

    // Flujo de audio crudo (frecuencia de muestra: 22050, tamaño de muestra: 16bit-short, channels: 1-mono)
    AudioStream stream = LoadAudioStream(44100, 16, 1);

    // Buffer para la onda de ciclo unico que estamos utilizando
    // short *bugger = (short *)malloc(sizeof(short)*MAX_MUESTRAS);
    short bufferEspecial[MAX_MUESTRAS] = {0};

    for (int i=0; i < MAX_MUESTRAS; i++)
        bufferEspecial[i] = 0;
    
    // Tamaño en bytes. 2bytes  * 512 = 1024bytes Unsigned long
    short *bufferUnCiclo = (short *)malloc(sizeof(short)*MAX_MUESTRAS);

    // Frame buffer, describe la forma de onda cuando se repite en el transcurso de un fotograma.
    short *bufferPorFrame = (short *)malloc(sizeof(short)*MAX_MUESTRAS_POR_FRAME);

    PlayAudioStream(stream);        // Comienzo de proceso el buffer de flujo (no bufferUnCiclo cargado actualmente)
  
    // Ciclos por segundo (hz)
    float frecuencia = 0.0f;

    // Valor previo, usado pra comprobar si el seno necesita ser rescrito, y para modular la frecuencia suavemente
    float viejaFrecuencia = 1.0f;

    // Cursor para leer y copiar las muestras del buffer de onda sinusoidal
    int readCursor = 0;

    // Tamaño calculado en muestras de la onda sinusoidal
    int waveLength = 1;

    // Vector2 position = { 0, 0 };

   int amplitud=32000;

    float perilla1= 1.0f;
    float perilla2= 1.0f;
    float perilla3= 1.0f;
    float perilla4= 1.0f;
    float perilla5= 50.0f;
    float perilla6= 0.1f;
    float perilla7= 50.0f;
    float perilla8= 50.0f;

    bool escalasAleatorias = false;
    // Funcion original de seno
    bool senoFunction = true;
    
    // Funcion triangular
    bool triangularFunction = false;
    
    // Funcion cuadrática
    bool cuadradaFunction = false;
    
    // Funcion sierra
    bool sawFunction = false;


    time_t t;
    srand((unsigned) time(&t));


    
    SetTargetFPS(30);               // Configurar para que se ejecute a 30 cuadros por segundo
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detectar cierre de ventana o tecla ESC
    {
        // Actualizacion
        //----------------------------------------------------------------------------------
        SetMasterVolume(perilla6);
        frecuencia = seleccionarNota(perilla5);

        if(escalasAleatorias){
            frecuencia = seleccionarNota(valorRandomAEscala(perilla5));
        }

        
        

         

        // Rescribir la onda sinusoidal.
        // Computar dos ciclos para permitir el relleno del búfer, simplificando cualquier modulacion, remuestrendo, etc.
        // If (frecuencia != viejaFrecuencia)
        // {
            // Calcular la longitud de onda. Limitar su tamaño en ambas direcciones.

            

            int oldWavelength = waveLength;
            waveLength = (int)(22050/frecuencia);
            if (waveLength > MAX_MUESTRAS/2) waveLength = MAX_MUESTRAS/2;
            if (waveLength < 1) waveLength = 1;
            
            // Escribir funcion al buffer.
            for (int i = 0; i < waveLength*2; i++)
            {

                if(senoFunction){
                    
                    bufferEspecial[i] += perilla1* (sinf(((2*PI*(float)i/waveLength)))*32000);

                }
                
                
                if (triangularFunction) {

                    for(int j =1; j<MAX_MUESTRAS;j=j+4)
                    {
                        bufferEspecial[i]+=perilla2* sinf(2*PI*(float)j*i/waveLength)*(-amplitud/j);     //1
                        bufferEspecial[i]+=perilla2* sinf(2*PI*(float)(j+2)*i/waveLength)*(amplitud/(j+2));  //3

                    }
                    
                
                }
                
                if (cuadradaFunction) {

                    for(int j =1; j<MAX_MUESTRAS;j=j+4)
                    {
                        bufferEspecial[i]+= perilla3* sinf(2*PI*(float)j*i/waveLength)*(amplitud/j);     //1
                        bufferEspecial[i]+=  perilla3*  sinf(2*PI*(float)(j+2)*i/waveLength)*(amplitud/(j+2));  //3

                    }

                }
                
                if (sawFunction) {

                    for(int j =1; j<MAX_MUESTRAS;j=j+2)
                    {
                        bufferEspecial[i]+=perilla4*  sinf(2*PI*(float)j*i/waveLength)*amplitud/j;
                        bufferEspecial[i]-= perilla4* sinf(2*PI*(float)(j+1)*i/waveLength)*(amplitud/(j+1));
                    }
                }
                bufferUnCiclo[i] =  bufferEspecial[i];

                for (int i=0; i < MAX_MUESTRAS; i++)
                {
                        bufferEspecial[i] = 0;
                }
                
                

            }

            // Escalar la posicion del cursor para minimizar los artefactos de transicion 
            readCursor = (int)(readCursor * ((float)waveLength / (float)oldWavelength));
            
            viejaFrecuencia = frecuencia;
        // }

        // Rellenar la onda de audio si es necesario 
        if (IsAudioStreamProcessed(stream))
        {
            // Sintetizar al buffer con el tamaño exacto 
            int writeCursor = 0;

            while (writeCursor < MAX_MUESTRAS_POR_FRAME)
            {
                // Empezar por tratar de escribir todo el pedazo 
                int writeLength = MAX_MUESTRAS_POR_FRAME-writeCursor;

                // Limite para el tamaño leible maximo 
                int readLength = waveLength-readCursor;

                if (writeLength > readLength) writeLength = readLength;

                // Escribe la rebanada
                memcpy(bufferPorFrame + writeCursor, bufferUnCiclo + readCursor, writeLength*sizeof(short));

                // Actualizar el cursor y el loop de audio 
                readCursor = (readCursor + writeLength) % waveLength;

                writeCursor += writeLength;
            }

            // Copiar el frame terminado al flujo de audio 
            UpdateAudioStream(stream, bufferPorFrame, MAX_MUESTRAS_POR_FRAME);
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText(TextFormat("frecuencia del seno: %i",(int)frecuencia), GetScreenWidth() - 300, 10, 20, DARKGRAY);
            DrawText("Presionar de z a m escala normal.", 10, 10, 20, DARKGRAY);
            // DrawText("Presionar q para cambiar de funcion.", 10, 30, 20, DARKGRAY);
            // DrawText("Presionar p para pausa.", 10, 50, 20, DARKGRAY);
            // DrawText("Presionar l para bucle de sonido.", 10, 70, 20, DARKGRAY);

        if(senoFunction)
            perilla1 = GuiSliderBar((Rectangle){ 600, 380, 120, 20 }, "1", NULL, perilla1, -1.0f, 1.0f);
        if(triangularFunction)
            perilla2 = GuiSliderBar((Rectangle){ 600, 420, 120, 20 }, "2", NULL, perilla2, -1.0f, 1.0f);
        if(cuadradaFunction)
            perilla3 = GuiSliderBar((Rectangle){ 600, 460, 120, 20 }, "3", NULL, perilla3, -1.0f, 1.0f);
        if(sawFunction)
            perilla4 = GuiSliderBar((Rectangle){ 600, 500, 120, 20 }, "4", NULL, perilla4, -1.0f, 1.0f);

            if(!escalasAleatorias)
            {
                perilla5 = GuiSliderBar((Rectangle){ 20, 380, 120, 20 }, "", NULL, perilla5, 1, 3);
                DrawText(TextFormat("Escala manual: %i",(int)perilla5), 20, 340, 20, BLACK);

            }

            perilla6 = GuiSliderBar((Rectangle){ 400, 10, 120, 20 }, "Vol.", NULL, perilla6, 0, 0.25);
            // perilla7 = GuiSliderBar((Rectangle){ 600, 300, 120, 20 }, "7", NULL, perilla7, -1.0, 1.00);
            // perilla8 = GuiSliderBar((Rectangle){ 600, 340, 120, 20 }, "8", NULL, perilla8, -1.0, 1.00);


            // Radiointerno = GuiSliderBar((Rectangle){ 600, 140, 120, 20 }, "RadioInterno", NULL, RadioInterno, 0, 100);
            // Radioexterno = GuiSliderBar((Rectangle){ 600, 170, 120, 20 }, "RadioExterno", NULL, RadioExterno, 0, 200);

            escalasAleatorias = GuiCheckBox((Rectangle){ 150, 380, 20, 20 }, "Escalas aleatorias", escalasAleatorias);


            senoFunction = GuiCheckBox((Rectangle){ 750, 380, 20, 20 }, "Sin Original", senoFunction);
        
            triangularFunction = GuiCheckBox((Rectangle){ 750, 420, 20, 20 }, "Triangular", triangularFunction);
        
            cuadradaFunction = GuiCheckBox((Rectangle){ 750, 460, 20, 20 }, "Cuadrada", cuadradaFunction);
        
            sawFunction = GuiCheckBox((Rectangle){ 750, 500, 20, 20 }, "Sierra", sawFunction);
            //------------------------------------------------------------------------------

            for (int i = 0; i < pantallaAncho; i++)
            {
                // position.x = (float)i;
                
                DrawPixel(i, 150 + 100*bufferUnCiclo[i*MAX_MUESTRAS/pantallaAncho]/(float)32000, GREEN);
                DrawLine(i, 150 + 100*bufferUnCiclo[i*MAX_MUESTRAS/pantallaAncho]/(float)32000, i+1,150 + 100*bufferUnCiclo[(i+1)*MAX_MUESTRAS/pantallaAncho]/(float)32000,BLUE);

                DrawPixel(i, 300 + 50*bufferPorFrame[i*MAX_MUESTRAS_POR_FRAME/pantallaAncho]/(float)32000, RED);

                DrawLine(i, 300 + 50*bufferPorFrame[i*MAX_MUESTRAS_POR_FRAME/pantallaAncho]/(float)32000, i+1,300 + 50*bufferPorFrame[(i+1)*MAX_MUESTRAS_POR_FRAME/pantallaAncho]/(float)32000,RED);
                
            }
            

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Inicializacion
    //--------------------------------------------------------------------------------------
    free(bufferUnCiclo);                 // Descargar onda sinusoidal al bufferUnCiclo
    free(bufferPorFrame);             // Descargar el buffer

    UnloadAudioStream(stream);   // Cerrar el fljo de audio crudo y borrar buffers de la RAM
    CloseAudioDevice();         // Cerrar el dispositivo de audio (La musica para automaticamente)

    CloseWindow();              // Cerrar ventana y OpenGL
    //--------------------------------------------------------------------------------------

    return 0;
}

float seleccionarNota(float perilla5)
{   
            float frequency = 0;
            if (IsKeyDown(KEY_Z)) {
                frequency = 261.6f;

            }
            // frecuencia para tecla DO #
            if (IsKeyDown(KEY_S)) {
                frequency = 277.2f;
            }
            //frecuencia para tecla RE
            if (IsKeyDown(KEY_X)) {
                frequency = 293.6f;
            }
            //frecuencia para tecla RE#
            if (IsKeyDown(KEY_D))  {
                frequency = 311.1f;
            }
            //frecuencia para tecla MI
            if (IsKeyDown(KEY_C)) {
                frequency = 329.6f;
            }
            //frecuencia para tecla FA
            if (IsKeyDown(KEY_V)) {
                frequency = 349.2f;
            }
            //frecuencia para tecla FA#
            if (IsKeyDown(KEY_G)) {
                frequency = 370.0f;
            }
            //frecuencia para tecla SOL
            if (IsKeyDown(KEY_B)) {
                frequency = 392.0f;
            }
            //frecuencia para tecla SOL#
            if (IsKeyDown(KEY_H)) {
                frequency = 415.3f;
            }
            //frecuencia para tecla LA
            if (IsKeyDown(KEY_N)) {
                frequency = 440.0f;
            }
            //frecuencia para tecla LA#
            if (IsKeyDown(KEY_J)) {
                frequency = 466.2f;
            }
            //frecuencia para tecla SI
            if (IsKeyDown(KEY_M)) {
                frequency = 493.2f;
            }

            return frequency/perilla5;
}
float valorRandomAEscala(float perilla5){

     //aleatoria en el rango de 5-10;

    static int lower = 1, upper = 5;

    float i = (rand()%(upper - lower + 1)) + lower;
    

    return i;
}