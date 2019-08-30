#include <3ds.h>
#include <citro3d.h>
#include <tex3ds.h>
#include <camera.h>
#include <iostream>
#include <vector>
#include "vshader_shbin.h"
// libpng -- needs zlib too
#include <png.h>

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

static C3D_Tex* cubeTexture;
static C3D_Tex* floorTexture;
static C3D_Tex* transparentTexture;

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
	C3D_TexBind(0, cubeTexture);
	Mtx_Identity(&modelView);
	Mtx_Translate(&modelView, -1.0f, 0.0f, -1.0f, true);
	Mtx_Multiply(&modelView, &modelView, &view);
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView);

	//draw one cube
	C3D_DrawArrays(GPU_TRIANGLES, 0, 36);

	Mtx_Identity(&modelView);
	Mtx_Translate(&modelView, 2.0f, 0.0f, 0.0f, true);
	Mtx_Multiply(&modelView, &modelView, &view);
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView);
	//draw one cube
	C3D_DrawArrays(GPU_TRIANGLES, 0, 36);


	/*C3D_SetBufInfo(&floorbufInfo);
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

	}*/
	



}

void sceneExit()
{
	// Free texture data
	C3D_TexDelete(cubeTexture);
	//C3D_TexDelete(&floorTexture);
	//C3D_TexDelete(&transparentTexture);

	// Free VBOs
	linearFree(cubevbo);
	//linearFree(floorvbo);
	//linearFree(transparentvbo);

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


C3D_Tex* LoadTextureFromFile(char file[]){


        unsigned char header[8];    // 8 is the maximum size that can be checked

        /* open file and test for it being a png */
        FILE *fp = fopen(file, "rb");
        if (!fp)
                //abort_("[read_png_file] File %s could not be opened for reading", file_name);
        	cout << "error fopen" << endl;
        	
        fread(header, 1, 8, fp);
        if (png_sig_cmp(header, 0, 8))
                //abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);
        	cout << "error sig_cmp" << endl;
      





        /* initialize stuff */
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        if (!png){
        	cout << "error png_create_read_struct" << endl;
        	return nullptr;
        }
        	

        png_infop info = png_create_info_struct(png);
        if (!info){
        		cout << "error png_create_info_struct" << endl;
        		return nullptr;
        }

         if(setjmp(png_jmpbuf(png)))
	    {
	        png_destroy_read_struct(&png, &info, NULL);
	        cout << "error png_jmpbuf" << endl;
	        return nullptr;
	    }


        png_init_io(png, fp);
        png_set_sig_bytes(png, 8);

        png_read_info(png, info);

        int width = png_get_image_width(png, info);
        int height = png_get_image_height(png, info);


        png_byte color_type = png_get_color_type(png, info);
        png_byte bit_depth = png_get_bit_depth(png, info);

        cout << "width is: " << width << endl;
        cout << "height is: " << height << endl;

        cout << "texture bit depth is: " << (int) bit_depth << endl; //bit depth 8

        cout << "texture color type is: " << (int) color_type << endl; // color type 6

        /*
		   Color    Allowed    Interpretation
		   Type    Bit Depths
		   
		   0       1,2,4,8,16  Each pixel is a grayscale sample.
		   
		   2       8,16        Each pixel is an R,G,B triple.
		   
		   3       1,2,4,8     Each pixel is a palette index;
		                       a PLTE chunk must appear.
		   
		   4       8,16        Each pixel is a grayscale sample,
		                       followed by an alpha sample.
		   
		   6       8,16        Each pixel is an R,G,B triple,
		                       followed by an alpha sample.
        */

        if (png_get_valid(png, info, PNG_INFO_tRNS)) 
        	png_set_tRNS_to_alpha(png);


        png_set_bgr(png);
        png_set_swap_alpha(png);
        //number_of_passes = png_set_interlace_handling(png_ptr); //1048576 bytes
        png_read_update_info(png, info);

        //get rows
       	//png_bytep* row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);

       	 // Row size in bytes.
   		int rowbytes = png_get_rowbytes(png, info);

   		//cout << "rowbytes: " << rowbytes << endl;
 
   		//one big block of memory holds all the image data
       	png_byte* image_data = new png_byte[rowbytes * height];

       	//pointers to image_data for reading the png with libpng
    	png_bytep* row_pointers = new png_bytep[height];

    	//set pointers for each row
    	for (int i = 0; i < height; i++){
     		row_pointers[i] = image_data + i * rowbytes;

    	}

        png_read_image(png, row_pointers);

        //fclose(fp);

        //png_destroy_read_struct(&png, &info, NULL);

        C3D_Tex* tex = new C3D_Tex;
        bool alpha = false;
        int bytespp;

        C3D_TexInitParams params;
        memset(&params, 0, sizeof(params));
        params.width    = width;
        params.height   = height;
        params.maxLevel = C3D_TexCalcMaxLevel(width, height);
        params.type = GPU_TEX_2D;
        params.onVram   = false;

        if (color_type == PNG_COLOR_TYPE_RGB ){
        	params.format = GPU_RGB8;
        	bytespp = 3;
        	cout << "doesn't have alpha" << endl;

    	}else if(color_type == PNG_COLOR_TYPE_RGB_ALPHA){
    		params.format = GPU_RGBA8;
    		alpha = true;
    		bytespp = 4;
    		cout << "has alpha" << endl;

    	}else cout << "texture color type not supported" << endl;

    	C3D_TexInitWithParams(tex, nullptr, params);

	  
    	//memset(tex->data, 100, tex->size);

    	/* Textures in the 3DS need to be swizzled and in ABGR format!!!!!!!!!

    	*/

    	
    	//The magic function
	    	for(int j = 0; j < height; j++) {
		        png_bytep row = row_pointers[j];
		        for(int i = 0; i < width; i++) {
		            png_bytep px = &(row[i * bytespp]);
		            u32 dst = ((((j >> 3) * (width >> 3) + (i >> 3)) << 6) + ((i & 1) | ((j & 1) << 1) | ((i & 2) << 1) | ((j & 2) << 2) | ((i & 4) << 2) | ((j & 4) << 3))) * bytespp; 

		            memcpy(tex->data + dst, px, sizeof(u32));
		        }
	    	}
    

    	//C3D_TexLoadImage(tex, row_pointers[0], GPU_TEXFACE_2D, 0);
    	C3D_TexFlush(tex);
    	 

    	//generate mipmaps, doesn't work? 
    	C3D_TexGenerateMipmap(tex, GPU_TEXFACE_2D);

		//clean up memory and close stuff
	   png_destroy_read_struct(&png, &info, nullptr);
	   delete[] image_data;
	   delete[] row_pointers;
	   fclose(fp);

       return tex;

}


int main(int argc, char* argv[])
{
	gfxInitDefault();
	Result rs = romfsInit();
	consoleInit(GFX_BOTTOM, NULL);

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

	//cout << "enable debug" << endl;
	//svcSleep(5);

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

	/*floorvbo = linearAlloc(sizeof(planeVertices));
	memcpy(floorvbo, planeVertices, sizeof(planeVertices));

	transparentvbo = linearAlloc(sizeof(transparentVertices));
	memcpy(transparentvbo, transparentVertices, sizeof(transparentVertices));*/

	// Configure buffers. Each object (VBO) has a different bufInfo.
	//cubebufInfo = C3D_GetBufInfo();
	BufInfo_Init(&cubebufInfo);
	BufInfo_Add(&cubebufInfo, cubevbo, sizeof(float)*5, 2, 0x10);

	/*BufInfo_Init(&floorbufInfo);
	BufInfo_Add(&floorbufInfo, floorvbo, sizeof(float)*5, 2, 0x10);
	
	BufInfo_Init(&transparentbufInfo);
	BufInfo_Add(&transparentbufInfo, transparentvbo, sizeof(float)*5, 2, 0x10);*/

	
	cubeTexture = LoadTextureFromFile("romfs:/gfx/marble.png");

	if(cubeTexture == nullptr){

		cout << "An error ocurred loading a texture" << endl;
	}

	/*Tex3DS_Texture floort3x = Tex3DS_TextureImport(metal_t3x, metal_t3x_size, &floorTexture, nullptr, false);
	Tex3DS_Texture transparentt3x = Tex3DS_TextureImport(grass_t3x, grass_t3x_size, &transparentTexture, nullptr, false);*/

	C3D_TexSetFilter(cubeTexture, GPU_LINEAR, GPU_LINEAR);
	C3D_TexSetWrap(cubeTexture, GPU_REPEAT, GPU_REPEAT);
	C3D_TexSetFilterMipmap(cubeTexture, GPU_LINEAR);
	//C3D_TexBind(0, &marble_tex);

	/*C3D_TexSetFilter(&floorTexture, GPU_LINEAR, GPU_LINEAR);
	C3D_TexSetWrap(&floorTexture, GPU_REPEAT, GPU_REPEAT);

	C3D_TexSetFilter(&transparentTexture, GPU_LINEAR, GPU_LINEAR);
	C3D_TexSetWrap(&transparentTexture, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);*/

	cout << "Tex cube size: " << cubeTexture->size << endl;
	/*cout << "Tex floor size: " << floorTexture.size << endl;
	cout << "Tex transparent size: " << transparentTexture.size << endl;*/
	
	
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
			//cout << "Render 10" << endl;
			osTickCounterUpdate(&frametime); 
			//cout << "Render 11" << endl;

			
		}


	/*Tex3DS_TextureFree(cubet3x);
	Tex3DS_TextureFree(floort3x);
	Tex3DS_TextureFree(transparentt3x);*/
	
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