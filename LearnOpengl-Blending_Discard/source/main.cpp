#include <3ds.h>
#include <citro3d.h>
#include <tex3ds.h>
#include <camera.h>
#include <iostream>
#include <vector>
#include "vshader_shbin.h"
#include "grass_t3x.h"
#include "metal_t3x.h"
#include "marble_t3x.h"
using namespace std;

#define CLEAR_COLOR 0x68B0D8FF

#define DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))


typedef struct { float position[3]; float texcoord[2]; } vertex;

static  float cubeVertices[] = {
        // positions          // texture Coords
		//back
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,       
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

        //front
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		
		//left
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

static float planeVertices[] = {
        // positions          // texture Coords 
         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,

         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        
         5.0f, -0.5f, -5.0f,  2.0f, 2.0f,
         -5.0f, -0.5f, -5.0f,  0.0f, 2.0f
    };
    
static float transparentVertices[] = {
        // positions         // texture Coords 
        0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.0f, -0.5f,  0.0f,  0.0f,  0.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  0.0f,
        

        0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
        1.0f, -0.5f,  0.0f,  1.0f,  0.0f,
        1.0f,  0.5f,  0.0f,  1.0f,  1.0f
        
    };




#define vertex_count ((sizeof(cubeVertices)+sizeof(planeVertices)+sizeof(transparentVertices))/sizeof(cubeVertices[0]))

Camera camera(FVec3_New(0.0f, 0.0f, 3.0f));
float lastX = 400.0 / 2.0;
float lastY = 240.0 / 2.0;
bool firstMouse = true;
static DVLB_s* vshader_dvlb;
static shaderProgram_s program;
static C3D_Mtx projection;
static C3D_Mtx modelView;
static circlePosition pos;
static void* cubevbo;
static void* floorvbo;
static void* transparentvbo;

static C3D_Tex cubeTexture;
static C3D_Tex floorTexture;
static C3D_Tex transparentTexture;

static C3D_BufInfo cubebufInfo;
static C3D_BufInfo floorbufInfo;
static C3D_BufInfo transparentbufInfo;

static C3D_AttrInfo* attrInfo;

int uLoc_projection;
int uLoc_modelView;

TickCounter frametime;
float deltaTime; //Frame time in seconds


static C3D_FVec vegetation[] =
    {
        FVec3_New(-1.5f, 0.0f, -0.48f),
        FVec3_New( 1.5f, 0.0f, 0.51f),
        FVec3_New( 0.0f, 0.0f, 0.7f),
        FVec3_New(-0.3f, 0.0f, -2.3f),
        FVec3_New(0.5f, 0.0f, -0.6f)
    };

void svcSleep(int seconds){

	s64 nanoseconds = seconds * 1000000000;

	svcSleepThread(nanoseconds);
}

void sceneRender(){

	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);

	C3D_Mtx view = camera.GetViewMatrix();

	C3D_SetBufInfo(&cubebufInfo);
	C3D_TexBind(0, &cubeTexture);
	Mtx_Identity(&modelView);
	Mtx_Translate(&modelView, -1.0f, 0.0f, -1.0f, true);
	Mtx_Multiply(&modelView, &modelView, &view);
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView);

	//cout << "vert num: " << sizeof(cubeVertices)/sizeof(cubeVertices[0]) << endl;
	//draw one cube
	C3D_DrawArrays(GPU_TRIANGLES, 0, 36);

	Mtx_Identity(&modelView);
	Mtx_Translate(&modelView, 2.0f, 0.0f, 0.0f, true);
	Mtx_Multiply(&modelView, &modelView, &view);
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView);
	//draw one cube
	C3D_DrawArrays(GPU_TRIANGLES, 0, 36);


	C3D_SetBufInfo(&floorbufInfo);
	C3D_TexBind(0, &floorTexture);
	Mtx_Identity(&modelView);
	Mtx_Multiply(&modelView, &modelView, &view);
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView);

	//draw one floor
	C3D_DrawArrays(GPU_TRIANGLES, 0, 6);


	C3D_SetBufInfo(&transparentbufInfo);
	C3D_TexBind(0, &transparentTexture);
	for(int i=0; i < sizeof(vegetation)/sizeof(vegetation[0]);i++){


		Mtx_Identity(&modelView);
		Mtx_Translate(&modelView, vegetation[i].x, vegetation[i].y, vegetation[i].z, true);
		Mtx_Multiply(&modelView, &modelView, &view);
		C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView);
		//draw one grass
		C3D_DrawArrays(GPU_TRIANGLES, 0, 6);

	}




}

void sceneExit()
{
	// Free textures
	C3D_TexDelete(&cubeTexture);
	C3D_TexDelete(&floorTexture);
	C3D_TexDelete(&transparentTexture);

	// Free VBOs
	linearFree(cubevbo);
	linearFree(floorvbo);
	linearFree(transparentvbo);

	// Free the shader program
	shaderProgramFree(&program);
	DVLB_Free(vshader_dvlb);
}

void processInput(u32* kDown, u32* kHeld)
{


    if (*kHeld & KEY_DUP)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (*kHeld & KEY_DDOWN)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (*kHeld & KEY_DLEFT)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (*kHeld & KEY_DRIGHT)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    hidCircleRead(&pos);

    if (firstMouse)
    {
        lastX = pos.dx;
        lastY = pos.dy;
        firstMouse = false;
    }

    if(*kHeld & KEY_CPAD_RIGHT || *kHeld & KEY_CPAD_LEFT || *kHeld & KEY_CPAD_UP || *kHeld & KEY_CPAD_DOWN){
    //cout << "pos.dx: " << pos.dx << "pos.dy: " << pos.dy << endl;
    float xoffset = pos.dx;
    float yoffset = pos.dy; // reversed since y-coordinates go from bottom to top

    lastX = pos.dx;
    lastY = pos.dy;

    camera.ProcessMouseMovement(xoffset, yoffset);

	}
}


int main(int argc, char* argv[])
{
	gfxInitDefault();
	Result rs = romfsInit();
	consoleInit(GFX_BOTTOM, NULL);

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

	cout << "enable debug" << endl;
	svcSleep(5);

	// STEP 1: Set rendertargets
	C3D_RenderTarget* target = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetSetOutput(target, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

	//STEP 2: Set a shader program and bind it
	vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
	shaderProgramInit(&program);
	shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
	C3D_BindProgram(&program);

	//STEP 3: Get location of uniforms in the shader

	uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
	uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");

	Mtx_PerspTilt(&projection, C3D_AngleFromDegrees(camera.Zoom), C3D_AspectRatioTop, 0.1f, 100.0f, false);
	Mtx_Identity(&modelView);

	attrInfo = C3D_GetAttrInfo();
	AttrInfo_Init(attrInfo);
	AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
	AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord

	cubevbo = linearAlloc(sizeof(cubeVertices));
	memcpy(cubevbo, cubeVertices, sizeof(cubeVertices));

	floorvbo = linearAlloc(sizeof(planeVertices));
	memcpy(floorvbo, planeVertices, sizeof(planeVertices));

	transparentvbo = linearAlloc(sizeof(transparentVertices));
	memcpy(transparentvbo, transparentVertices, sizeof(transparentVertices));

	// Configure buffers. Each object (VBO) has a different bufInfo.
	//cubebufInfo = C3D_GetBufInfo();
	BufInfo_Init(&cubebufInfo);
	BufInfo_Add(&cubebufInfo, cubevbo, sizeof(float)*5, 2, 0x10);

	BufInfo_Init(&floorbufInfo);
	BufInfo_Add(&floorbufInfo, floorvbo, sizeof(float)*5, 2, 0x10);
	
	BufInfo_Init(&transparentbufInfo);
	BufInfo_Add(&transparentbufInfo, transparentvbo, sizeof(float)*5, 2, 0x10);

	
	Tex3DS_Texture cubet3x = Tex3DS_TextureImport(marble_t3x, marble_t3x_size, &cubeTexture, nullptr, false);
	Tex3DS_Texture floort3x = Tex3DS_TextureImport(metal_t3x, metal_t3x_size, &floorTexture, nullptr, false);
	Tex3DS_Texture transparentt3x = Tex3DS_TextureImport(grass_t3x, grass_t3x_size, &transparentTexture, nullptr, false);

	C3D_TexSetFilter(&cubeTexture, GPU_LINEAR, GPU_LINEAR);
	C3D_TexSetWrap(&cubeTexture, GPU_REPEAT, GPU_REPEAT);
	C3D_TexSetFilterMipmap(&cubeTexture, GPU_LINEAR);
	//C3D_TexBind(0, &marble_tex);

	C3D_TexSetFilter(&floorTexture, GPU_LINEAR, GPU_LINEAR);
	C3D_TexSetWrap(&floorTexture, GPU_REPEAT, GPU_REPEAT);

	C3D_TexSetFilter(&transparentTexture, GPU_LINEAR, GPU_LINEAR);
	C3D_TexSetWrap(&transparentTexture, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);

	cout << "Tex cube size: " << cubeTexture.size << endl;
	cout << "Tex floor size: " << floorTexture.size << endl;
	cout << "Tex transparent size: " << transparentTexture.size << endl;

	
	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
	C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0);
	C3D_TexEnvOpRgb(env, GPU_TEVOP_RGB_SRC_COLOR);
	C3D_TexEnvOpAlpha(env, GPU_TEVOP_A_SRC_ALPHA);
	C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);

	//Configure alpha test to discard transparent fragments.
	C3D_AlphaTest(true, GPU_GREATER, 0);

	

	

		while(aptMainLoop()){

			deltaTime = osTickCounterRead(&frametime) / 1000.0;

			osTickCounterStart(&frametime); 


			//Input
			hidScanInput();
			
			u32 kDown = hidKeysDown();
			u32 kHeld = hidKeysHeld();

				if (kDown & KEY_START)
					break;


			processInput(&kDown,&kHeld);

			std::cout << "\rDrawTime: " << C3D_GetDrawingTime() << std::flush;
			//std::cout << "\rFrameTime: " << deltaTime << std::flush;
			//cout << "DrawTime" << endl;

			// Render
			C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
			C3D_RenderTargetClear(target, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
			C3D_FrameDrawOn(target);
			sceneRender();
			C3D_FrameEnd(0);
			/*cout << "GPU Finished frame" << endl;

			}else cout << "False: Waiting for GPU to Finish" << endl;*/
			osTickCounterUpdate(&frametime); 


			
		}


	Tex3DS_TextureFree(cubet3x);
	Tex3DS_TextureFree(floort3x);
	Tex3DS_TextureFree(transparentt3x);
	
	sceneExit();



	// Main loop
	/*while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		// Your code goes here
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
	}*/

	C3D_Fini();
	gfxExit();
	return 0;
}



/*void inputLoop(){ // funcion para bloquear input

		int c = 0;

		std::cout << "Press A to continue" << 
		while(c == 0)
		{	
			hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();

			//std::cout << "Press A to continue." << std::endl;
			
			
			if(kDown & KEY_A){

				break;
			}

		} 

}
*/