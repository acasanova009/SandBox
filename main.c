#include <raylib.h>


#include <stdlib.h>         // Required for: malloc(), free()
#include <math.h>           // Required for: sinf()
#include <string.h>         // Required for: memcpy()



#define MAX_MUESTRAS               512
#define MAX_MUESTRAS_POR_FRAME   4096

#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include "extras/raygui.h"                 // Required for GUI controls

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int pantallaAncho = 800;
    const int pantallaAlto = 450;

    InitWindow(pantallaAncho, pantallaAlto, "Sintetizador de audio");

    InitAudioDevice();              // Initialize audio device

    SetAudioStreamBufferSizeDefault(MAX_MUESTRAS_POR_FRAME);

    // Init raw audio stream (sample rate: 22050, sample size: 16bit-short, channels: 1-mono)
    AudioStream stream = LoadAudioStream(44100, 16, 1);

    // Buffer for the single cycle waveform we are synthesizing
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

   

    float perilla1= 50.0f;
    float perilla2= 50.0f;
    float perilla3= 50.0f;
    float perilla4= 50.0f;
    float perilla5= 50.0f;


    // Funcion original de seno
    bool originalFunction = true;
    
    // Funcion triangular
    bool triangularFunction = true;
    
    // Funcion cuadrática
    bool cuadraticFunction = true;
    
    // Funcion sierra
    bool sawFunction = true;


    
    SetTargetFPS(30);               // Set our game to run at 30 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        
        frecuencia = 0;
        

         // frecuencia para tecla DO
            if (IsKeyDown(KEY_Z)) {
               
                frecuencia = 261.6f;

            }
            // frecuencia para tecla DO #
            if (IsKeyDown(KEY_S)) {
                frecuencia = 277.2f;
            }
            //frecuencia para tecla RE
            if (IsKeyDown(KEY_X)) {
                frecuencia = 293.6f;
            }
            //frecuencia para tecla RE#
            if (IsKeyDown(KEY_D))  {
                frecuencia = 311.1f;
            }
            //frecuencia para tecla MI
            if (IsKeyDown(KEY_C)) {
                frecuencia = 329.6f;
            }
            //frecuencia para tecla FA
            if (IsKeyDown(KEY_V)) {
                frecuencia = 349.2f;
            }
            //frecuencia para tecla FA#
            if (IsKeyDown(KEY_G)) {
                frecuencia = 370.0f;
            }
            //frecuencia para tecla SOL
            if (IsKeyDown(KEY_B)) {
                frecuencia = 392.0f;
            }
            //frecuencia para tecla SOL#
            if (IsKeyDown(KEY_H)) {
                frecuencia = 415.3f;
            }
            //frecuencia para tecla LA
            if (IsKeyDown(KEY_N)) {
                frecuencia = 440.0f;
            }
            //frecuencia para tecla LA#
            if (IsKeyDown(KEY_J)) {
                frecuencia = 466.2f;
            }
            //frecuencia para tecla SI
            if (IsKeyDown(KEY_M)) {
                frecuencia = 493.2f;
            }
            

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

                    bufferUnCiclo[i] = (sinf(((2*PI*(float)i/waveLength)))*32000);

                }else if (triangularFunction) {


                    bufferUnCiclo[i] = (sinf(((2*PI*(float)i/waveLength)))*32000);
                
                }else if (cuadraticFunction) {

                    bufferUnCiclo[i] = (sinf(((2*PI*(float)i/waveLength)))*32000);

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
            perilla4 = GuiSliderBar((Rectangle){ 600, 180, 120, 20 }, "4", NULL, perilla4, 0, 100);
            perilla5 = GuiSliderBar((Rectangle){ 600, 220, 120, 20 }, "5", NULL, perilla5, 0, 100);


            // innerRadius = GuiSliderBar((Rectangle){ 600, 140, 120, 20 }, "InnerRadius", NULL, innerRadius, 0, 100);
            // outerRadius = GuiSliderBar((Rectangle){ 600, 170, 120, 20 }, "OuterRadius", NULL, outerRadius, 0, 200);

            originalFunction = GuiCheckBox((Rectangle){ 600, 380, 20, 20 }, "Sin Original", originalFunction);
        
            triangularFunction = GuiCheckBox((Rectangle){ 500, 380, 20, 20 }, "Triangular", triangularFunction);
        
            cuadraticFunction = GuiCheckBox((Rectangle){ 400, 380, 20, 20 }, "Cuadrática", cuadraticFunction);
        
            sawFunction = GuiCheckBox((Rectangle){ 300, 380, 20, 20 }, "Sierra", sawFunction);
            //------------------------------------------------------------------------------

            

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