#include <raylib.h>


#include <stdlib.h>         // Required for: malloc(), free()
#include <math.h>           // Required for: sinf()
#include <string.h>         // Required for: memcpy()



#define MAX_MUESTRAS               512
#define MAX_MUESTRAS_POR_FRAME   4096

#define MAX_SENOS 1000

#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include "extras/raygui.h"                 // Required for GUI controls

float seleccionarNota(float perilla5);

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int pantallaAncho = 1200;
    const int pantallaAlto = 600;

    InitWindow(pantallaAncho, pantallaAlto, "Sintetizador de audio");

    InitAudioDevice();              // Initialize audio device

    SetAudioStreamBufferSizeDefault(MAX_MUESTRAS_POR_FRAME);

    // Init raw audio stream (sample rate: 22050, sample size: 16bit-short, channels: 1-mono)
    AudioStream stream = LoadAudioStream(44100, 16, 1);

    // Buffer for the single cycle waveform we are synthesizing
    // short *bugger = (short *)malloc(sizeof(short)*MAX_MUESTRAS);
    short bufferEspecial[MAX_MUESTRAS] = {0};
    // Size in bytes. 2bytes  * 512 = 1024bytes Unsigned long
    short *bufferUnCiclo = (short *)malloc(sizeof(short)*MAX_MUESTRAS);

    // Frame buffer, describing the waveform when repeated over the course of a frame
    short *bufferPorFrame = (short *)malloc(sizeof(short)*MAX_MUESTRAS_POR_FRAME);

    PlayAudioStream(stream);        // Start processing stream buffer (no bufferUnCiclo loaded currently)
  
    // Cycles per second (hz)
    float frecuencia = 0.0f;

    // Previous value, used to test if sine needs to be rewritten, and to smoothly modulate frecuencia
    float viejaFrecuencia = 1.0f;

    // Cursor to read and copy the samples of the sine wave buffer
    int readCursor = 0;

    // Computed size in samples of the sine wave
    int waveLength = 1;

    // Vector2 position = { 0, 0 };

   int amplitud=32000;

    float perilla1= 50.0f;
    float perilla2= 50.0f;
    float perilla3= 50.0f;
    float perilla4= 1.0f;
    float perilla5= 50.0f;
    float perilla6= 0.1f;
    float perilla7= 50.0f;
    float perilla8= 50.0f;


    // Funcion original de seno
    bool originalFunction = true;
    
    // Funcion triangular
    bool triangularFunction = false;
    
    // Funcion cuadrática
    bool cuadraticFunction = false;
    
    // Funcion sierra
    bool sawFunction = false;


    
    SetTargetFPS(30);               // Set our game to run at 30 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        SetMasterVolume(perilla6);
        
        frecuencia = seleccionarNota(perilla5);
        

         

        // Rewrite the sine wave.
        // Compute two cycles to allow the buffer padding, simplifying any modulation, resampling, etc.
        if (frecuencia != viejaFrecuencia)
        {
            // Compute wavelength. Limit size in both directions.

            printf("wavelength = %i\n",waveLength);

            int oldWavelength = waveLength;
            waveLength = (int)(22050/frecuencia);
            if (waveLength > MAX_MUESTRAS/2) waveLength = MAX_MUESTRAS/2;
            if (waveLength < 1) waveLength = 1;
            
            // Write functino to buffer.
            for (int i = 0; i < waveLength*2; i++)
            {

                if(originalFunction){
                    
                    bufferUnCiclo[i] = (sinf(((2*PI*(float)i/waveLength)))*32000);

                }else if (sawFunction) {

                    
                    
                    //For 1 j<N j=j+2
                    for(int j =1; j<MAX_MUESTRAS;j=j+2)
                    {
                        bufferEspecial[i]+= sinf(2*PI*(float)j*i/waveLength)*amplitud/j;
                        bufferEspecial[i]-=sinf(2*PI*(float)(j+1)*i/waveLength)*(amplitud/(j+1));
                    }
                    
                    bufferUnCiclo[i] =  bufferEspecial[i];


                    

                }else if (triangularFunction) {

                    for(int j =1; j<MAX_MUESTRAS;j=j+4)
                    {
                        bufferEspecial[i]+= sinf(2*PI*(float)j*i/waveLength)*(-amplitud/j);     //1
                        bufferEspecial[i]+=sinf(2*PI*(float)(j+2)*i/waveLength)*(amplitud/(j+2));  //3

                    }
                    bufferUnCiclo[i] =  bufferEspecial[i];
                
                }else if (cuadraticFunction) {

                    for(int j =1; j<MAX_MUESTRAS;j=j+4)
                    {
                        bufferEspecial[i]+=  sinf(2*PI*(float)j*i/waveLength)*(amplitud/j);     //1
                        bufferEspecial[i]+=    sinf(2*PI*(float)(j+2)*i/waveLength)*(amplitud/(j+2));  //3
                        // bufferUnCiclo[i]+= perilla4*  sinf(2*PI*(float)j*i/waveLength)*(amplitud/j);     //1
                        // bufferUnCiclo[i]+=  perilla4*  sinf(2*PI*(float)(j+2)*i/waveLength)*(amplitud/(j+2));  //3

                    }

                    bufferUnCiclo[i] =  bufferEspecial[i];
                }
                

                for (int i=0; i < MAX_MUESTRAS; i++)
                {
                        bufferEspecial[i] = 0;
                }
                
                

            }

            // Scale read cursor's position to minimize transition artifacts
            readCursor = (int)(readCursor * ((float)waveLength / (float)oldWavelength));
            
            viejaFrecuencia = frecuencia;
        }

        // Refill audio stream if required
        if (IsAudioStreamProcessed(stream))
        {
            // Synthesize a buffer that is exactly the requested size
            int writeCursor = 0;

            while (writeCursor < MAX_MUESTRAS_POR_FRAME)
            {
                // Start by trying to write the whole chunk at once
                int writeLength = MAX_MUESTRAS_POR_FRAME-writeCursor;

                // Limit to the maximum readable size
                int readLength = waveLength-readCursor;

                if (writeLength > readLength) writeLength = readLength;

                // Write the slice
                memcpy(bufferPorFrame + writeCursor, bufferUnCiclo + readCursor, writeLength*sizeof(short));

                // Update cursors and loop audio
                readCursor = (readCursor + writeLength) % waveLength;

                writeCursor += writeLength;
            }

            // Copy finished frame to audio stream
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

            perilla1 = GuiSliderBar((Rectangle){ 600, 60, 120, 20 }, "1", NULL, perilla1, 0, 100);
            perilla2 = GuiSliderBar((Rectangle){ 600, 100, 120, 20 }, "2", NULL, perilla2, 0, 100);
            perilla3 = GuiSliderBar((Rectangle){ 600, 140, 120, 20 }, "3", NULL, perilla3, 0, 100);
            perilla4 = GuiSliderBar((Rectangle){ 600, 180, 120, 20 }, "4", NULL, perilla4, -1.0f, 1.0f);

            
            perilla5 = GuiSliderBar((Rectangle){ 20, 380, 120, 20 }, "", NULL, perilla5, 1, 5);
            DrawText(TextFormat("Escala actual: %i",(int)perilla5), 20, 340, 20, BLACK);

            perilla6 = GuiSliderBar((Rectangle){ 400, 10, 120, 20 }, "Vol.", NULL, perilla6, 0, 0.25);
            // perilla7 = GuiSliderBar((Rectangle){ 600, 300, 120, 20 }, "7", NULL, perilla7, -1.0, 1.00);
            // perilla8 = GuiSliderBar((Rectangle){ 600, 340, 120, 20 }, "8", NULL, perilla8, -1.0, 1.00);


            // innerRadius = GuiSliderBar((Rectangle){ 600, 140, 120, 20 }, "InnerRadius", NULL, innerRadius, 0, 100);
            // outerRadius = GuiSliderBar((Rectangle){ 600, 170, 120, 20 }, "OuterRadius", NULL, outerRadius, 0, 200);

            originalFunction = GuiCheckBox((Rectangle){ 200, 380, 20, 20 }, "Sin Original", originalFunction);
        
            triangularFunction = GuiCheckBox((Rectangle){ 400, 380, 20, 20 }, "Triangular", triangularFunction);
        
            cuadraticFunction = GuiCheckBox((Rectangle){ 500, 380, 20, 20 }, "Cuadrática", cuadraticFunction);
        
            sawFunction = GuiCheckBox((Rectangle){ 300, 380, 20, 20 }, "Sierra", sawFunction);
            //------------------------------------------------------------------------------

            for (int i = 0; i < pantallaAncho; i++)
            {
                // position.x = (float)i;
                
                DrawPixel(i, 150 + 100*bufferUnCiclo[i*MAX_MUESTRAS/pantallaAncho]/(float)32000, GREEN);

                DrawPixel(i, 300 + 50*bufferPorFrame[i*MAX_MUESTRAS_POR_FRAME/pantallaAncho]/(float)32000, RED);
                
            }
            

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    free(bufferUnCiclo);                 // Unload sine wave bufferUnCiclo
    free(bufferPorFrame);             // Unload write buffer

    UnloadAudioStream(stream);   // Close raw audio stream and delete buffers from RAM
    CloseAudioDevice();         // Close audio device (music streaming is automatically stopped)

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

float seleccionarNota(float perilla5)
{   
            float frequency = 0;
            if (IsKeyDown(KEY_Z)) {
                frequency = 261.6f;

            }
            // frequency para tecla DO #
            if (IsKeyDown(KEY_S)) {
                frequency = 277.2f;
            }
            //frequency para tecla RE
            if (IsKeyDown(KEY_X)) {
                frequency = 293.6f;
            }
            //frequency para tecla RE#
            if (IsKeyDown(KEY_D))  {
                frequency = 311.1f;
            }
            //frequency para tecla MI
            if (IsKeyDown(KEY_C)) {
                frequency = 329.6f;
            }
            //frequency para tecla FA
            if (IsKeyDown(KEY_V)) {
                frequency = 349.2f;
            }
            //frequency para tecla FA#
            if (IsKeyDown(KEY_G)) {
                frequency = 370.0f;
            }
            //frequency para tecla SOL
            if (IsKeyDown(KEY_B)) {
                frequency = 392.0f;
            }
            //frequency para tecla SOL#
            if (IsKeyDown(KEY_H)) {
                frequency = 415.3f;
            }
            //frequency para tecla LA
            if (IsKeyDown(KEY_N)) {
                frequency = 440.0f;
            }
            //frequency para tecla LA#
            if (IsKeyDown(KEY_J)) {
                frequency = 466.2f;
            }
            //frequency para tecla SI
            if (IsKeyDown(KEY_M)) {
                frequency = 493.2f;
            }

            return frequency/perilla5;
}
