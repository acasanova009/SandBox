#include <raylib.h>         //Requerido para libreria raylib
#include <stdlib.h>        // Requerido para: malloc(), free()
#include <math.h>            // Requerido para: sinf()
#include <string.h>         // Requerido para: memcpy()
#include <time.h>
#define RAYGUI_IMPLEMENTATION //Se tiene que definir como implementacion de controles GUI.
#include "extras/raygui.h"      //Requerido para  GUI controls

//Nos permite definir el total de muestra basico
#define MAX_MUESTRAS               500
//Nos permite definir tl tamano de podra contener el buffer antes de enviar el audio al audiostream.
#define MAX_MUESTRAS_POR_FRAME   4000
//Se usa para definir la cantidad de iteraciones para sumar senos en armonicos. Max para perilla.
#define MAX_SENOS 100
//Esta funcion basicamente selecciona la frecuencia conforme a la tecla presionada.
float seleccionarNota(float perilla5);

//Nos genera un valor aleatorio para poder cambiar las escalas.
float valorRandomAEscala();

int main(void)
{
    // Inicializacion
    //--------------------------------------------------------------------------------------
    const int pantallaAncho = 1200;
    const int pantallaAlto = 600;
    
    InitWindow(pantallaAncho, pantallaAlto, "Sintetizador de audio");
    // Inicializacion de dispositivo de audio
    InitAudioDevice();              // Inicializacion audio device

    SetAudioStreamBufferSizeDefault(MAX_MUESTRAS_POR_FRAME);

    // Flujo de audio crudo (frecuencia de muestra: 22050, tamaño de muestra: 16bit-short, channels: 1-mono)
    AudioStream torrenteDeAudio = LoadAudioStream(44100, 16, 1);

    

    // Buffer para la onda de suma de sanles, para aromincos
    //Estatico
    short bufferSumaDeSignals[MAX_MUESTRAS] = {0};

    //Nos aseguramos este limpio el arreglo, o de otra manera genera ruidos.
    for (int i=0; i < MAX_MUESTRAS; i++)
        bufferSumaDeSignals[i] = 0;
    
    
    // Tamaño en bytes. 2bytes  * 512 = 1024bytes
    //DINamico
    
    short *bufferUnCiclo = (short *)malloc(sizeof(short)*MAX_MUESTRAS);

    // Frame buffer, describe la forma de onda cuando se repite en el transcurso de un fotograma, mientras se va a copiar directamente al torrente de audio
    short *bufferPorFrame = (short *)malloc(sizeof(short)*MAX_MUESTRAS_POR_FRAME);

     // Comienzo de proceso el buffer de flujo (no bufferUnCiclo cargado actualmente)
    PlayAudioStream(torrenteDeAudio);       
  
    // Ciclos por segundo (hz)
    float frecuencia = 0.0f;


    // Cursor para leer y copiar las muestras del buffer de onda sinusoidal
    int cursorDeLectura = 0;

    // Tamaño calculado en muestras de la onda sinusoidal
    int longitudDeOnda = 1;

    
    //Esta ampltid se define como el total de posibilidades de 2a la 16, y se multiplica es seno -1 a 1 por esta cantidad.
   int bitRateCorrecto=10000;

    //Esta primer perilla va a cambiar la onda sin
    float perilla1= 1.0f;
    //Esta perilla va a cambiar la onda triangular
    float perilla2= 0.2f;
    //Esta perilla va a cambiar la onda cuadrada
    float perilla3= 0.5f;
    //Esta perilla va a cambiar la onda sierra
    float perilla4= 0.5f;

    //Esta perilla modifica la escala de la notas.
    float perilla5= 50.0f;

    //Esta perilla modifica el volumen
    float perilla6= 0.03f;

    //Modificar cantidad de senos.
    int perilla7= 3.0f;
    

    // float perilla7= 50.0f;

    bool escalasAleatorias = false;
    // Funcion original de seno
    bool funcionSenoActivada = true;
    
    // Funcion triangular
    bool funcionTrinagularActivada = false;
    
    // Funcion cuadrática
    bool funcionCuadraticaActivada = false;
    
    // Funcion sierra
    bool fundionSierraActivada = false;

    //Preparamos un aleatorio en cuestion de tiempo para las escalas aleatorias.
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
        //Seleccionamos la frecuencia dependiendo de la tecla presionada y la perilla
        frecuencia = seleccionarNota(perilla5);

        if(escalasAleatorias){
            frecuencia = seleccionarNota(valorRandomAEscala());
        }
 
                    
            //Mantenemos registro de la ultima longitud de onda
            int viejaLongitudDeOnda = longitudDeOnda;

            //Obtenemos la longitud de onda.
            longitudDeOnda = (int)(22050/frecuencia);
            //Revisamos que no salga del espacio predefinido del buffer por el lado derecho.
            if (longitudDeOnda > MAX_MUESTRAS/2) longitudDeOnda = MAX_MUESTRAS/2;
            //Revisamos que no salga del espacio predefinido por el lado izquierdo
            if (longitudDeOnda < 1) longitudDeOnda = 1;
            
            // Escribir funcion al buffer de senales. Pero antes al buffer de suma de senales.
            for (int i = 0; i < longitudDeOnda*1; i++)
            {
                //Funcion matematica para sin
                if(funcionSenoActivada){
                    
                    bufferSumaDeSignals[i] += perilla1* (sinf(((2*PI*(float)i/longitudDeOnda)))*bitRateCorrecto);

                }
                
                //Se desarolla la funcion metamatica para la funcion trinagular.
                if (funcionTrinagularActivada) {

                    for(int j =1; j<perilla7;j=j+4)
                    {
                        bufferSumaDeSignals[i]+=perilla2* sinf(2*PI*(float)j*i/longitudDeOnda)*(-bitRateCorrecto/j);     //1
                        bufferSumaDeSignals[i]+=perilla2* sinf(2*PI*(float)(j+2)*i/longitudDeOnda)*(bitRateCorrecto/(j+2));  //3

                    }

                    // bufferUnCiclo[i] = ((short)((sinf((2*PI*(float)i/waveLength)))(amplitud*perilla1))-(short)((sinf((2*PI(float)2*i/waveLength)))((amplitud/2)*perilla2))+(short)((sinf((2*PI(float)3*i/waveLength)))((amplitud/3)*perilla3))-(short)((sinf((2*PI(float)4*i/waveLength)))((amplitud/4)*perilla4))+(short)((sinf((2*PI(float)5*i/waveLength)))((amplitud/5)*perilla5))-(short)((sinf((2*PI(float)6*i/waveLength)))((amplitud/6)*perilla6))+(short)((sinf((2*PI(float)7*i/waveLength)))((amplitud/7)*perilla7))-(short)((sinf((2*PI(float)8*i/waveLength)))*((amplitud/8)*perilla8)))+(short)((sinf((2*PI(float)5*i/waveLength)))((amplitud/5)*perilla5))-(short)((sinf((2*PI(float)6*i/waveLength)))((amplitud/6)*perilla6))+(short)((sinf((2*PI(float)7*i/waveLength)))((amplitud/7)*perilla7))-(short)((sinf((2*PI(float)8*i/waveLength)))*((amplitud/8)*perilla8)))+(short)((sinf((2*PI(float)7*i/waveLength)))((amplitud/7)*perilla7))-(short)((sinf((2*PI(float)8*i/waveLength)))*((amplitud/8)*perilla8)))+(short)((sinf((2*PI(float)5*i/waveLength)))((amplitud/5)*perilla5))-(short)((sinf((2*PI(float)6*i/waveLength)))((amplitud/6)*perilla6))+(short)((sinf((2*PI(float)7*i/waveLength)))((amplitud/7)*perilla7))-(short)((sinf((2*PI(float)8*i/waveLength)))*((amplitud/8)*perilla8)))
                    
                
                }
                
                //Se desarolla la funcion metamatica para la funcion cuadratica.
                if (funcionCuadraticaActivada) {

                    for(int j =1; j<perilla7;j=j+4)
                    {
                        bufferSumaDeSignals[i]+= perilla3* sinf(2*PI*(float)j*i/longitudDeOnda)*(bitRateCorrecto/j);     //1
                        bufferSumaDeSignals[i]+=  perilla3*  sinf(2*PI*(float)(j+2)*i/longitudDeOnda)*(bitRateCorrecto/(j+2));  //3

                    }

                }
                
                //Se desarolla la funcion metamatica para la funcion sierra.
                if (fundionSierraActivada) {

                    for(int j =1; j<perilla7;j=j+2)
                    {
                        bufferSumaDeSignals[i]+=perilla4*  sinf(2*PI*(float)j*i/longitudDeOnda)*bitRateCorrecto/j;
                        bufferSumaDeSignals[i]-= perilla4* sinf(2*PI*(float)(j+1)*i/longitudDeOnda)*(bitRateCorrecto/(j+1));

                    }
                }

                //Aqui depositamos la suma de senales al bufferCiclo donde copiaremos al buffer final.
                bufferUnCiclo[i] =  bufferSumaDeSignals[i];

                //Nos aseguramos limpiar la suma de senales para evitar acumulacion de sonidos.
                for (int i=0; i < MAX_MUESTRAS; i++)
                {
                        bufferSumaDeSignals[i] = 0;
                }
                
                

            }

            // Basicamente reubicamos el cursor de lectura al aproxiamado de bitRateCorrecto relativo a la ultima onda, pero con cuidado de no empezar desde en inicio de la nueva onda, porque sonaria mal. Tenemos que asegurar que es relativo.
            cursorDeLectura = (int)(cursorDeLectura * ((float)longitudDeOnda / (float)viejaLongitudDeOnda));
            //CUIDADO. Esto tiene que estar perfectamente ligado con el tamano de lectura, de otra manera se desconfigura todo el buffer. Porque se saldria el area de lectura.
          
            
        // }

        // Rellenar la onda de audio si es necesario 
        if (IsAudioStreamProcessed(torrenteDeAudio))
        {
             // Sintetizar al buffer con el tamaño exacto 
            int cursorDeEscritura = 0;

            while (cursorDeEscritura < MAX_MUESTRAS_POR_FRAME)
            {
                 // Empezar por tratar de escribir todo el pedazo 
                int tamanoEscritura = MAX_MUESTRAS_POR_FRAME-cursorDeEscritura;

               // Limite para el tamaño leible maximo 
               //Anteriormente en la actualizacion del cursor de lectura, tener cuidado ya podria desconfigurar el buffer.
                int tamanodeLectura = longitudDeOnda-cursorDeLectura;

                //No importa que a veces sea mayor el tamano de lectrua, ya que solo vamos a escribir el tamano de escritura.
                //Es decir el tercer argumento del siguiente comando.
                if (tamanoEscritura > tamanodeLectura) tamanoEscritura = tamanodeLectura;

                // Escribe la rebanada
                //Se usa meemory allco malloc para poder usar eventaulemnete memcpy.
                memcpy(bufferPorFrame + cursorDeEscritura, bufferUnCiclo + cursorDeLectura, tamanoEscritura*sizeof(short));

                //Reposicionar el cursorDe Escritura conforme a la longitud del tamano de lectura.
                cursorDeEscritura += tamanoEscritura;

                
                // Este cursor de Lectura se usa principalmente cuando se acaba el tamano del buffer grande, y se tiene que reorganizar desde donde empezar a copiar la informacion del buffer pequeno. Porque se habia cortado la ultima onda, y sonaria mal.
                cursorDeLectura = (cursorDeLectura + tamanoEscritura) % longitudDeOnda;
                if (cursorDeLectura != 0)
                {

                    printf("PRIMERO cL: %d   tE %d    lO %d \n ", cursorDeLectura, tamanoEscritura, longitudDeOnda);
                    /* code */
                }


                

                
            }

            // Copiar el frame terminado al flujo de audio 
            UpdateAudioStream(torrenteDeAudio, bufferPorFrame, MAX_MUESTRAS_POR_FRAME);
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
//
            DrawText(TextFormat("frecuencia del seno: %i",(int)frecuencia), GetScreenWidth() - 300, 10, 20, DARKGRAY);
            DrawText("Presionar de z a m escala normal.", 10, 10, 20, DARKGRAY);
            // DrawText("Presionar q para cambiar de funcion.", 10, 30, 20, DARKGRAY);
            // DrawText("Presionar p para pausa.", 10, 50, 20, DARKGRAY);
            // DrawText("Presionar l para bucle de sonido.", 10, 70, 20, DARKGRAY);


        //Estas perillas se activan conforme los booleanos
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


            //Perilla de volumen.
            perilla6 = GuiSliderBar((Rectangle){ 400, 10, 120, 20 }, "Vol.", NULL, perilla6, 0, 0.01);
            //Cantidad de iteraciones para el seno.
            perilla7 = GuiSliderBar((Rectangle){ 600, 10, 120, 20 }, "Iterac.", NULL, perilla7, 0, MAX_SENOS);


            

            //Preparamos las casillas para que esten en la interfaz. y las conectamos con las variables correpondientes.
            escalasAleatorias = GuiCheckBox((Rectangle){ 150, 380, 20, 20 }, "Escalas aleatorias", escalasAleatorias);

            funcionSenoActivada = GuiCheckBox((Rectangle){ 750, 380, 20, 20 }, "Sin Original", funcionSenoActivada);
        
            funcionTrinagularActivada = GuiCheckBox((Rectangle){ 750, 420, 20, 20 }, "Triangular", funcionTrinagularActivada);
        
            funcionCuadraticaActivada = GuiCheckBox((Rectangle){ 750, 460, 20, 20 }, "Cuadrada", funcionCuadraticaActivada);
        
            fundionSierraActivada = GuiCheckBox((Rectangle){ 750, 500, 20, 20 }, "Sierra", fundionSierraActivada);
            //------------------------------------------------------------------------------



            //Para corroborar que en efecto se esta desarollando las funciones se implementaron los metodos de dibujo de pixeles directamente el la pantalla.



            for (int i = 0; i < longitudDeOnda*5; i++)
            {

            }
            
            for (int i = 0; i < pantallaAncho; i++)
            {
                
                // Primero dibujamos la funcion con pixeles
                DrawPixel(i, 150 + 100*bufferUnCiclo[i*MAX_MUESTRAS/pantallaAncho]/(float)bitRateCorrecto, GREEN);
                //Como quedan espacios huecos, los rellenados con linea.
                DrawLine(i, 150 + 100*bufferUnCiclo[i*MAX_MUESTRAS/pantallaAncho]/(float)bitRateCorrecto, i+1,150 + 100*bufferUnCiclo[(i+1)*MAX_MUESTRAS/pantallaAncho]/(float)bitRateCorrecto,BLUE);

                // Primero dibujamos la funcion con pixeles, aplicamos la misma logica pero para el buffer de frames.
                DrawPixel(i, 300 + 50*bufferPorFrame[i*MAX_MUESTRAS_POR_FRAME/pantallaAncho]/(float)bitRateCorrecto, RED);

                //Como quedan espacios huecos, para el buffer de frames.
                DrawLine(i, 300 + 50*bufferPorFrame[i*MAX_MUESTRAS_POR_FRAME/pantallaAncho]/(float)bitRateCorrecto, i+1,300 + 50*bufferPorFrame[(i+1)*MAX_MUESTRAS_POR_FRAME/pantallaAncho]/(float)bitRateCorrecto,RED);
                
            }
            

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Inicializacion
    //--------------------------------------------------------------------------------------
    free(bufferUnCiclo);                // Descargar onda sinusoidal al bufferUnCiclo

    free(bufferPorFrame);             // Descargar el buffer

    UnloadAudioStream(torrenteDeAudio);   // Cerrar el fljo de audio crudo y borrar buffers de la RAM
    CloseAudioDevice();         // Cerrar el dispositivo de audio (La musica para automaticamente)

    CloseWindow();              // Cerrar ventana y OpenGL
    //--------------------------------------------------------------------------------------

    return 0;
}

float seleccionarNota(float perilla5)
{   
            float freq = 0;

            // freq para tecla DO
            
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

            return freq/(int)perilla5;
}
float valorRandomAEscala(){

     //aleatoria en el rango de 5-10;

    static int bajo = 1, alto = 5;

    float i = (rand()%(alto - bajo + 1)) + bajo;
    

    return i;
}

