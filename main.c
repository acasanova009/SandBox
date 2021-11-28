#include <raylib.h>


#include <stdlib.h>         // Required for: malloc(), free()
#include <math.h>           // Required for: sinf()
#include <string.h>         // Required for: memcpy()

#include <time.h>

#define MAX_MUESTRAS               512
#define MAX_MUESTRAS_POR_FRAME   4096

#define MAX_SENOS 1000

#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include "extras/raygui.h"                 // Required for GUI controls

float seleccionarNota(float perilla5);
float valorRandomAEscala(float perilla5);

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int pantallaAncho = 1200;
    const int pantallaAlto = 600;
    //Se iniia la patnnala
    InitWindow(pantallaAncho, pantallaAlto, "Sintetizador de audio");
    //Se conecta con el disposiso ade audio
    InitAudioDevice();              // Initialize audio device

    SetAudioStreamBufferSizeDefault(MAX_MUESTRAS_POR_FRAME);

    // Init raw audio torrenteDeAudio (sample rate: 22050, sample size: 16bit-short, channels: 1-mono)
    AudioStream torrenteDeAudio = LoadAudioStream(44100, 16, 1);

    // Buffer for the single cycle waveform we are synthesizing
    // short *bugger = (short *)malloc(sizeof(short)*MAX_MUESTRAS);
    short bufferSumaDeSignals[MAX_MUESTRAS] = {0};

    for (int i=0; i < MAX_MUESTRAS; i++)
        bufferSumaDeSignals[i] = 0;
    
    // Size in bytes. 2bytes  * 512 = 1024bytes Unsigned long
    short *bufferUnCiclo = (short *)malloc(sizeof(short)*MAX_MUESTRAS);

    // Frame buffer, describing the waveform when repeated over the course of a frame
    short *bufferPorFrame = (short *)malloc(sizeof(short)*MAX_MUESTRAS_POR_FRAME);

    PlayAudioStream(torrenteDeAudio);        // Start processing torrenteDeAudio buffer (no bufferUnCiclo loaded currently)
  
    // Cycles per second (hz)
    float frecuencia = 0.0f;

    // Previous value, used to test if sine needs to be rewritten, and to smoothly modulate frecuencia
    float viejaFrecuencia = 1.0f;

    // Cursor to read and copy the samples of the sine wave buffer
    int cursorDeLectura = 0;

    // Computed size in samples of the sine wave
    int longitudDeOnda = 1;

    

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
    bool funcionSenoActivada = true;
    
    // Funcion triangular
    bool funcionTrinagularActivada = false;
    
    // Funcion cuadrática
    bool funcionCuadraticaActivada = false;
    
    // Funcion sierra
    bool fundionSierraActivada = false;


    time_t t;
    srand((unsigned) time(&t));


    
    SetTargetFPS(30);               // Set our game to run at 30 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        SetMasterVolume(perilla6);
        frecuencia = seleccionarNota(perilla5);

        if(escalasAleatorias){
            frecuencia = seleccionarNota(valorRandomAEscala(perilla5));
        }

        
        

         

        // Rewrite the sine wave.
        // Compute two cycles to allow the buffer padding, simplifying any modulation, resampling, etc.
        // if (frecuencia != viejaFrecuencia)
        // {
            // Compute wavelength. Limit size in both directions.

            

            int viejaLongitudDeOnda = longitudDeOnda;
            longitudDeOnda = (int)(22050/frecuencia);
            if (longitudDeOnda > MAX_MUESTRAS/2) longitudDeOnda = MAX_MUESTRAS/2;
            if (longitudDeOnda < 1) longitudDeOnda = 1;
            
            // Write functino to buffer.
            for (int i = 0; i < longitudDeOnda*2; i++)
            {

                if(funcionSenoActivada){
                    
                    bufferSumaDeSignals[i] += perilla1* (sinf(((2*PI*(float)i/longitudDeOnda)))*32000);

                }
                
                
                if (funcionTrinagularActivada) {

                    for(int j =1; j<MAX_MUESTRAS;j=j+4)
                    {
                        bufferSumaDeSignals[i]+=perilla2* sinf(2*PI*(float)j*i/longitudDeOnda)*(-amplitud/j);     //1
                        bufferSumaDeSignals[i]+=perilla2* sinf(2*PI*(float)(j+2)*i/longitudDeOnda)*(amplitud/(j+2));  //3

                    }
                    
                
                }
                
                if (funcionCuadraticaActivada) {

                    for(int j =1; j<MAX_MUESTRAS;j=j+4)
                    {
                        bufferSumaDeSignals[i]+= perilla3* sinf(2*PI*(float)j*i/longitudDeOnda)*(amplitud/j);     //1
                        bufferSumaDeSignals[i]+=  perilla3*  sinf(2*PI*(float)(j+2)*i/longitudDeOnda)*(amplitud/(j+2));  //3

                    }

                }
                //Aqui se desarolla la funcion sierra
                if (fundionSierraActivada) {

                    for(int j =1; j<MAX_MUESTRAS;j=j+2)
                    {
                        bufferSumaDeSignals[i]+=perilla4*  sinf(2*PI*(float)j*i/longitudDeOnda)*amplitud/j;
                        bufferSumaDeSignals[i]-= perilla4* sinf(2*PI*(float)(j+1)*i/longitudDeOnda)*(amplitud/(j+1));
                    }
                }
                bufferUnCiclo[i] =  bufferSumaDeSignals[i];

                for (int i=0; i < MAX_MUESTRAS; i++)
                {
                        bufferSumaDeSignals[i] = 0;
                }
                
                

            }

            // Scale read cursor's position to minimize transition artifacts
            cursorDeLectura = (int)(cursorDeLectura * ((float)longitudDeOnda / (float)viejaLongitudDeOnda));
            
            viejaFrecuencia = frecuencia;
        // }

        //Se rellena el strem de adudio desde el buffer
        if (IsAudioStreamProcessed(torrenteDeAudio))
        {
            // Synthesize a buffer that is exactly the requested size
            int cursorDeEscritura = 0;

            while (cursorDeEscritura < MAX_MUESTRAS_POR_FRAME)
            {
                // Start by trying to write the whole chunk at once
                int tamanoEscritura = MAX_MUESTRAS_POR_FRAME-cursorDeEscritura;

                // Limit to the maximum readable size
                int tamanodeLectura = longitudDeOnda-cursorDeLectura;

                if (tamanoEscritura > tamanodeLectura) tamanoEscritura = tamanodeLectura;

                // Write the slice
                memcpy(bufferPorFrame + cursorDeEscritura, bufferUnCiclo + cursorDeLectura, tamanoEscritura*sizeof(short));

                // Update cursors and loop audio
                cursorDeLectura = (cursorDeLectura + tamanoEscritura) % longitudDeOnda;

                cursorDeEscritura += tamanoEscritura;
            }

            // Copy finished frame to audio torrenteDeAudio
            UpdateAudioStream(torrenteDeAudio, bufferPorFrame, MAX_MUESTRAS_POR_FRAME);
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

        if(funcionSenoActivada)
            perilla1 = GuiSliderBar((Rectangle){ 600, 380, 120, 20 }, "1", NULL, perilla1, -1.0f, 1.0f);
        if(funcionTrinagularActivada)
            perilla2 = GuiSliderBar((Rectangle){ 600, 420, 120, 20 }, "2", NULL, perilla2, -1.0f, 1.0f);
        if(funcionCuadraticaActivada)
            perilla3 = GuiSliderBar((Rectangle){ 600, 460, 120, 20 }, "3", NULL, perilla3, -1.0f, 1.0f);
        if(fundionSierraActivada)
            perilla4 = GuiSliderBar((Rectangle){ 600, 500, 120, 20 }, "4", NULL, perilla4, -1.0f, 1.0f);

            if(!escalasAleatorias)
            {
                perilla5 = GuiSliderBar((Rectangle){ 20, 380, 120, 20 }, "", NULL, perilla5, 1, 3);
                DrawText(TextFormat("Escala manual: %i",(int)perilla5), 20, 340, 20, BLACK);

            }

            perilla6 = GuiSliderBar((Rectangle){ 400, 10, 120, 20 }, "Vol.", NULL, perilla6, 0, 0.25);
            // perilla7 = GuiSliderBar((Rectangle){ 600, 300, 120, 20 }, "7", NULL, perilla7, -1.0, 1.00);
            // perilla8 = GuiSliderBar((Rectangle){ 600, 340, 120, 20 }, "8", NULL, perilla8, -1.0, 1.00);


            // innerRadius = GuiSliderBar((Rectangle){ 600, 140, 120, 20 }, "InnerRadius", NULL, innerRadius, 0, 100);
            // outerRadius = GuiSliderBar((Rectangle){ 600, 170, 120, 20 }, "OuterRadius", NULL, outerRadius, 0, 200);

            escalasAleatorias = GuiCheckBox((Rectangle){ 150, 380, 20, 20 }, "Escalas aleatorias", escalasAleatorias);


            funcionSenoActivada = GuiCheckBox((Rectangle){ 750, 380, 20, 20 }, "Sin Original", funcionSenoActivada);
        
            funcionTrinagularActivada = GuiCheckBox((Rectangle){ 750, 420, 20, 20 }, "Triangular", funcionTrinagularActivada);
        
            funcionCuadraticaActivada = GuiCheckBox((Rectangle){ 750, 460, 20, 20 }, "Cuadrada", funcionCuadraticaActivada);
        
            fundionSierraActivada = GuiCheckBox((Rectangle){ 750, 500, 20, 20 }, "Sierra", fundionSierraActivada);
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

    // De-Initialization
    //--------------------------------------------------------------------------------------
    free(bufferUnCiclo);                 // Unload sine wave bufferUnCiclo
    free(bufferPorFrame);             // Unload write buffer

    UnloadAudioStream(torrenteDeAudio);   // Close raw audio torrenteDeAudio and delete buffers from RAM
    CloseAudioDevice();         // Close audio device (music streaming is automatically stopped)

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

float seleccionarNota(float perilla5)
{   
            float freq = 0;
            if (IsKeyDown(KEY_Z)) {
                freq = 261.6f;

            }
            // freq para tecla DO #
            if (IsKeyDown(KEY_S)) {
                freq = 277.2f;
            }
            //freq para tecla RE
            if (IsKeyDown(KEY_X)) {
                freq = 293.6f;
            }
            //freq para tecla RE#
            if (IsKeyDown(KEY_D))  {
                freq = 311.1f;
            }
            //freq para tecla MI
            if (IsKeyDown(KEY_C)) {
                freq = 329.6f;
            }
            //freq para tecla FA
            if (IsKeyDown(KEY_V)) {
                freq = 349.2f;
            }
            //freq para tecla FA#
            if (IsKeyDown(KEY_G)) {
                freq = 370.0f;
            }
            //freq para tecla SOL
            if (IsKeyDown(KEY_B)) {
                freq = 392.0f;
            }
            //freq para tecla SOL#
            if (IsKeyDown(KEY_H)) {
                freq = 415.3f;
            }
            //freq para tecla LA
            if (IsKeyDown(KEY_N)) {
                freq = 440.0f;
            }
            //freq para tecla LA#
            if (IsKeyDown(KEY_J)) {
                freq = 466.2f;
            }
            //freq para tecla SI
            if (IsKeyDown(KEY_M)) {
                freq = 493.2f;
            }

            return freq/perilla5;
}
float valorRandomAEscala(float perilla5){

     //random in range from 5-10;

    static int bajo = 1, alto = 5;

    float i = (rand()%(alto - bajo + 1)) + bajo;
    

    return i;
}