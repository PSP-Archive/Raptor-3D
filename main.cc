

#include <stdlib.h> // needed in order to have "exit" function @@@
#include <stdio.h>
#include <malloc.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>    // Header File For The GLUT Library 

#include <GL/gl.h>	// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library
//#include <unistd.h>     // Header File for sleeping. @@@
#include <pspctrl.h>
#include <pspdebug.h>
#include <math.h>
#include <zlib.h>
#include <oslib.h>
#include "lua\lua.h"
#include "lua\lualib.h"
#include "lua\lauxlib.h"
#include "mikmod.h"
#include "jpeglib.h"
#include <setjmp.h>
#include <psprtc.h>
#include "png.h"
#define RAM_BLOCK      (1024 * 1024)

u32 sceRamLeftMax(void) 
{ 
 u32 size, sizeblock; 
 u8 *ram; 


 // Init variables 
 size = 0; 
 sizeblock = RAM_BLOCK; 

 // Check loop 
 while (sizeblock) 
 { 
  // Increment size 
  size += sizeblock; 

  // Allocate ram 
  ram = (u8 *)malloc(size); 

  // Check allocate 
  if (!(ram)) 
  { 
   // Restore old size 
   size -= sizeblock; 

   // Size block / 2 
   sizeblock >>= 1; 
  } 
  else 
   free(ram); 
 } 

 return size; 
} 

u32 sceRamLeft(void) 
{ 
 u8 **ram, **temp; 
 u32 size, count, x; 


 // Init variables 
 ram = NULL; 
 size = 0; 
 count = 0; 

 // Check loop 
 for (;;) 
 { 
  // Check size entries 
  if (!(count % 10)) 
  { 
   // Allocate more entries if needed 
   void * tmp= realloc(ram,sizeof(u8 *) * (count + 10)); 
   temp = (u8 **)tmp;
   if (!(temp)) break; 
  
   // Update entries and size (size contains also size of entries) 
   ram = temp; 
   size += (sizeof(u8 *) * 10); 
  } 

  // Find max lineare size available 
  x = sceRamLeftMax(); 
  if (!(x)) break; 

  // Allocate ram 
  ram[count] = (u8 *) malloc(x); 
  if (!(ram[count])) break; 

  // Update variables 
  size += x; 
  count++; 
 } 

 // Free ram 
 if (ram) 
 { 
  for (x=0;x<count;x++) free(ram[x]); 
  free(ram); 
 } 

 return size; 
}

//return time in milliseconds 
inline u64 GetTick() 
{ 
     u64 temp; 
     sceRtcGetCurrentTick(&temp); 
     return abs(temp/1000); 
}

class Matrix;
class FontEngine;

FontEngine *FontRenderer;

void QuatToMat(float w,float x,float y,float z,Matrix *mat);


//-----------[ Log Engine ] 

class LogEngine
{
public:
	LogEngine(const char * name,const char *logname,int append = 0)
	{
		_filename = name;
		_file = fopen(_filename,"w");
		Close();	
		Open();
		fprintf(_file,"%s Logger \n",logname);
		Close();
			
	}
	void Open()
	{
		_file = fopen(_filename,"a");
	}
	void Close()
	{
		fclose(_file);
	}
	void Log(const char *txt)
	{
		Open();
		fprintf(_file,txt);
		Close();
	}
	void Log(const char *txt,u32 val)
	{
		Open();
		fprintf(_file,txt,val);
		Close();
	}
	void Log(const char *txt,int ival,float fval)
	{
		Open();
		fprintf(_file,txt,ival,fval);
		Close();
	}
	void Log(const char *txt,const char *s1,const char *s2)
	{
		Open();
		fprintf(_file,txt,s1,s2);
		Close();
	}
	void Log(const char *txt,const char *s)
	{
		Open();
		fprintf(_file,txt,s);
		Close();
	}
	void Log(const char *txt,int val)
	{
		Open();
		fprintf(_file,txt,val);
		Close();
	}
	void Log(const char *txt,float val)
	{
		Open();
		fprintf(_file,txt,val);
		Close();
	}
	~LogEngine()
	{
		Open();
		Log("Logger deleted");
		Close();
	}
	const char *_filename;
	FILE *_file;
};
LogEngine * Logger;

//-------- END OF Logger.


template <class T>
class ListNode
{

public:
	  T &get()
    {
        return object;
    };
    void set(T &object)
    {
        this->object = object;
    };

    ListNode<T> *getNext()
    {
        return nextNode;
    };
    void setNext(ListNode<T> *nextNode)
    {
        this->nextNode = nextNode;
    };

private:
    T object;
    ListNode<T> *nextNode;
};

template <class T>
class List
{

public:
    // Constructor
    List()
    {
        headNode = new ListNode<T>;
        headNode->setNext(NULL);

        currentNode = NULL;
        size = 0;
    };

    // Destructor
    ~List()
    {

        ListNode<T> *pointerToDelete, *pointer = headNode;

        while (pointer != NULL)
        {
            pointerToDelete = pointer;
            pointer = pointer->getNext();
            delete pointerToDelete;
        }
    };

    T &get()
    {

        if (currentNode == NULL)
            start();

        return currentNode->get()
               ;
    };

    void add(T addObject)
    {
    	  ListNode<T> *t = headNode;
    	  while(1)
    	  {
    	  	if( t->getNext() == NULL )	break;
    	  	t = t->getNext();
    	  }
    	    
        ListNode<T> *newNode = new ListNode<T>;
				newNode->setNext(NULL);
        newNode->set(addObject);
	
	       t->setNext(newNode);

        size++;
    };

    void remove()
    {

        lastCurrentNode->setNext(currentNode->getNext());

        delete currentNode;

        currentNode = lastCurrentNode;

        size--;
    };

    void start()
    {
        lastCurrentNode = headNode;
        currentNode = headNode;
    };

    bool next()
    {

        // If the currentNode now points at nothing, we've reached the end
        if (currentNode == NULL)
            return false;

        // Update the last node and current node
        lastCurrentNode = currentNode;
        currentNode = currentNode->getNext();

        // If currentNode points at nothing or there is nothing added, we can immediately return false
        if (currentNode == NULL || size == 0)
            return false;
        else
            return true;
    };

    int getSize()
    {
        return size;
    };

private:
    int size;
    ListNode<T> *headNode;
    ListNode<T> *currentNode, *lastCurrentNode;
};



//--------[ Raptor Profiler ]

class Length
{
public:
	u64 _time;
};

class Call
{
public:
	Call()
	{
		_name = NULL;
		_start = 0;
		_calls =0;
	}
	char * _name;
	u64 _start;
	List<Length *>_times;
	int _calls;
};

List<Call *>calls;

class Profiler
{
public:
	void LogProfile()
	{
		Logger->Log("Profiler Output:\n");
		calls.start();
		while( calls.next() == true )
		{
			Call *call = calls.get();
			int tottime=0;
			call->_times.start();
			while( call->_times.next() == true )
			{
				Length *len = call->_times.get();
				tottime+=len->_time;
			}
			Logger->Log("Function:%s \n",call->_name);
			Logger->Log("Total(Seconds):%f \n",(float)tottime/1000.0f);
			Logger->Log("Avg(Ms):%d \n",tottime/call->_calls);
			Logger->Log("-----------------------");
		}
		Logger->Log("End of profile dump.\n");
	}
	void Leave(char *name)
	{
	
		calls.start();
		while( calls.next() == true )
		{
			Call *call = calls.get();
			if( strcmp( call->_name,name) == 0 )
			{

				Length * len = new Length;
				len->_time = abs( GetTick()-call->_start );
				call->_times.add( len );

			}
		}
		
		return;
		Logger->Log("Unable to leave function %s , does not exist.\n",name);
	}
	void Enter(char *name)
	{
		int found=false;
		calls.start();
		while( calls.next() == true )
		{
			if( found == true) continue;
				
			Call *call = calls.get();
			if( strcmp( call->_name,name ) == 0 )
			{
				call->_start = GetTick();
				call->_calls++;
				found = true;
			}
		}
		if(found==true) return;
		
		Call *call = new Call;
		calls.add( call );
		call->_calls = 1;
		call->_name = name;
		call->_start = GetTick();
		
	}
	
};

Profiler *Profile;

//-----------------END OF Profiler.



static void writepng(FILE *fp, const unsigned char *image, int width, int height)
{
	const unsigned char *rows[height];
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	int row;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		return;
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		goto out_free_write_struct;

	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, width, height,
		     8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	for(row = 0; row < height; row++)
		rows[row] = &image[row * width * 4];

	png_set_rows(png_ptr, info_ptr, (png_byte**)rows);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	png_write_end(png_ptr, info_ptr);

  out_free_write_struct:
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
}


void screenshot(const char *basename)
{
	if (basename == NULL)
		basename = "screenshot";
	char name[6 + strlen(basename) + 4 + 4 + 1];
	int count = 0;
	FILE *fp = NULL;
	unsigned char *image;

	do {
		if (fp)
			fclose(fp);
#if SYS
		sprintf(name, "%s%03d.png", basename, count++);
#else
		sprintf(name, "ms0:/%s%03d.png", basename, count++);
#endif
		fp = fopen(name, "rb");
	} while(fp != NULL && count < 1000);

	if (count == 1000) {
		if (fp)
			fclose(fp);
		return;
	}

	fp = fopen(name, "wb");
	if (fp == NULL)
		return;

	glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
	glPixelStorei(GL_PACK_INVERT_MESA, GL_TRUE);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

	//image = malloc(480*272*4);
	image =(unsigned char *) memalign(64, 480*272*4);
	glReadPixels(0,0, 480,272, GL_RGBA,GL_UNSIGNED_BYTE, image);

	glPopClientAttrib();

	if (glGetError() != 0)
		goto out;

	writepng(fp, image, 480, 272);
	

  out:
	fclose(fp);
	free(image);
}


int window; 

int done = 0;
extern int _mm_errno;
extern BOOL _mm_critical;
extern char *_mm_errmsg[];
int mikModThreadID = -1;
extern UWORD md_mode;
extern UBYTE md_reverb;
extern UBYTE md_pansep;

void my_error_handler(void)
{
	printf("_mm_critical %d\n", MikMod_critical);
	printf("_mm_errno %d\n", MikMod_errno);
	printf("%s\n", MikMod_strerror(MikMod_errno));
	return;
}

static int AudioChannelThread(int args, void *argp)
{
  while (!done)
  {
    MikMod_Update();
    // We have to sleep here to allow other threads a chance to process.
    // with no sleep this thread will take over when the output is disabled via MikMod_DisableOutput()
    // co-operative threading sucks bigtime...
    sceKernelDelayThread(1);
  }
  return (0);
}

class Channel
{
public:
	Channel( int id )
	{
		_id = id;
	};
	Channel()
	{
		_id = 0;
	}
	int _id;
};

class Sound
{
public:
	Sound(char *file)
	{
		_id = NULL;
		_id = Sample_Load(file);
	}
	Channel * Play()
	{
		Channel *chan = new Channel( Sample_Play(_id,0,0) );
		return chan;
	}
	SAMPLE *_id;
};


class Module
{
public:
	Module()
	{
		_id = NULL;
	}
		MODULE *_id; 
};


// I had some issues getting threading to work in C++,
// so you must manually update the soundengine with SE->Update();
class SoundEngine
{
public:
	SoundEngine()
	{
		if (!MikMod_InitThreads())
  	{
  	  printf("MikMod thread init failed\n");
 		}
 	 MikMod_RegisterErrorHandler(my_error_handler);
 	 /* register all the drivers */
 	 MikMod_RegisterAllDrivers();
 	 /* register all the module loaders */
 	 MikMod_RegisterAllLoaders();
 		md_mode = DMODE_16BITS|DMODE_STEREO|DMODE_SOFT_SNDFX|DMODE_SOFT_MUSIC; 
		md_reverb = 0;
		md_pansep = 128;
    if (MikMod_Init(""))
  	{
   	 printf("Could not initialize sound, reason: %s\n", MikMod_strerror(MikMod_errno));
   	 sceKernelExitGame();
  	}
  	MikMod_SetNumVoices(-1, 8);
 		MikMod_EnableOutput();	  
  
  
	}
	void Update()
	{
		MikMod_Update();
	}

};



const float RAD_TO_DEG = 57.2957795130823208767981548141052;


// Blend modes. All inline for speed.
inline void Blend_Mask()
{
	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc( GL_GEQUAL,0.5f );
}

inline void Blend_Solid()
{
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
}

inline void Blend_Alpha()
{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_ALPHA_TEST);
}


enum EntityClass{ Cls_Base,Cls_Entity,Cls_Bone };

class SE_Parameter
{
public:
	
};

class ScriptEngine
{
public:
	ScriptEngine()
	{
		Reset();
	}
	void Reset()
	{
		if(!(_luaState==NULL)) lua_close(_luaState);
		_luaState = luaL_newstate();	//'lua_open()
		
		if( !(_luaState==NULL))
		{
			luaopen_base(_luaState);
			luaopen_table(_luaState);
//'			luaopen_io(_luaState);
			luaopen_string(_luaState);
			luaopen_math(_luaState);
			_started = true;
		}
		else
		{
			_started = false;
		}

	}
	void ReturnString(lua_State * ls,const char *value)
	{
		if(_started==true)
		{
			lua_pushstring(ls,value);
		}
	}
	void returnNumber(lua_State * ls,int value)
	{
		if(_started==true)
		{
			lua_pushnumber(ls,value);
		}
	}
	void AddFunc(const char *name, lua_CFunction func)
	{
		if(_started==true)
		{
			lua_register( _luaState,name,func );
		}
	}
	const char * GetString( lua_State * ls,int index )
	{
		return luaL_checkstring( ls,index );
	}
	int  GetNumber( lua_State * ls,int index )
	{
		return (int)luaL_checkint( ls,index);
	}
	void LoadFile(const char *file)
	{
		luaL_loadfile(_luaState, file);
	}
	void Run()
	{
		if(_started==true)
		{
			lua_pcall(_luaState,0,0,0);
		}
	}
	int _started;
	const char *_source;
	const char *_lastErrorString;
	int _lastErrorNumber;
	lua_State * _luaState;
	List<SE_Parameter *>_paramList;
	const char *_resultString;
	int _resultNumber;
	int _resultBoolean;
	int _resultType;
	
};





/* Image type - contains height, width, and data */
struct Image {
    unsigned long sizeX;
    unsigned long sizeY;
    char *data;
};
typedef struct Image Image;

// quick and dirty bitmap loader...for 24 bit bitmaps with 1 plane only.  
// See http://www.dcs.ed.ac.uk/~mxr/gfx/2d/BMP.txt for more info.
int ImageLoad(char *filename, Image *image) {
    FILE *file;
    unsigned long size;                 // size of the image in bytes.
    unsigned long i;                    // standard counter.
    unsigned short int planes;          // number of planes in image (must be 1) 
    unsigned short int bpp;             // number of bits per pixel (must be 24)
    char temp;                          // temporary color storage for bgr-rgb conversion.

    // make sure the file is there.
    if ((file = fopen(filename, "rb"))==NULL)
    {
	printf("File Not Found : %s\n",filename);
	return 0;
    }
    
    // seek through the bmp header, up to the width/height:
    fseek(file, 18, SEEK_CUR);

    // read the width
    if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
	printf("Error reading width from %s.\n", filename);
	return 0;
    }
    printf("Width of %s: %lu\n", filename, image->sizeX);
    
    // read the height 
    if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
	printf("Error reading height from %s.\n", filename);
	return 0;
    }
    printf("Height of %s: %lu\n", filename, image->sizeY);
    
    // calculate the size (assuming 24 bits or 3 bytes per pixel).
    size = image->sizeX * image->sizeY * 3;

    // read the planes
    if ((fread(&planes, 2, 1, file)) != 1) {
	printf("Error reading planes from %s.\n", filename);
	return 0;
    }
    if (planes != 1) {
	printf("Planes from %s is not 1: %u\n", filename, planes);
	return 0;
    }

    // read the bpp
    if ((i = fread(&bpp, 2, 1, file)) != 1) {
	printf("Error reading bpp from %s.\n", filename);
	return 0;
    }
    if (bpp != 24) {
	printf("Bpp from %s is not 24: %u\n", filename, bpp);
	return 0;
    }
	
    // seek past the rest of the bitmap header.
    fseek(file, 24, SEEK_CUR);

    // read the data. 
    image->data = (char *) memalign(16,size);
    if (image->data == NULL) {
	printf("Error allocating memory for color-corrected image data");
	return 0;	
    }

    if ((i = fread(image->data, size, 1, file)) != 1) {
	printf("Error reading image data from %s.\n", filename);
	return 0;
    }

    for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb)
	temp = image->data[i];
	image->data[i] = image->data[i+2];
	image->data[i+2] = temp;
    }
    fclose(file);
    // we're done.
    return 1;
}


struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

class Img
{
public:
	char *_buf;
	int _w,_h,_c;
};


class JpgLoader
{
public:
	JpgLoader()
	{
	}
	Img * LoadJpg(const char *file)
	{
		struct jpeg_decompress_struct cinfo;
		struct my_error_mgr jerr;
  	/* More stuff */
	  FILE * infile;		/* source file */
 		JSAMPARRAY buffer;		/* Output row buffer */
	  int row_stride;		/* physical row width in output buffer */
  	if ((infile = fopen(file, "rb")) == NULL) {
  	  printf("Unable to load jpg %s\n", file);
  	  return NULL;
  	}
  	cinfo.err = jpeg_std_error(&jerr.pub);
	  jerr.pub.error_exit = my_error_exit;
	  /* Establish the setjmp return context for my_error_exit to use. */
	  if (setjmp(jerr.setjmp_buffer)) {
	    /* If we get here, the JPEG code has signaled an error.
	     * We need to clean up the JPEG object, close the input file, and return.
	     */
	    jpeg_destroy_decompress(&cinfo);
	    fclose(infile);
	    return NULL;
	  }
  	/* Now we can initialize the JPEG decompression object. */
 	   jpeg_create_decompress(&cinfo);

  	/* Step 2: specify data source (eg, a file) */
  	 jpeg_stdio_src(&cinfo, infile);
			
		 (void) jpeg_read_header(&cinfo, TRUE);
		 (void) jpeg_start_decompress(&cinfo);
  	 row_stride = cinfo.output_width * cinfo.output_components;
  	   buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
			char *imgbuf = (char *)memalign(16, cinfo.output_width*cinfo.output_height * cinfo.output_components );
		
			_buf = imgbuf;
			_w = cinfo.output_width;
			_h = cinfo.output_height;
			_c = cinfo.output_components;
			_ln = 0;
			
		while (cinfo.output_scanline < cinfo.output_height) {
    	/* jpeg_read_scanlines expects an array of pointers to scanlines.
    	 * Here the array is only one element long, but you could ask for
    	 * more than one scanline at a time if that's more convenient.
    	 */
    	(void) jpeg_read_scanlines(&cinfo, buffer, 1);
    	/* Assume put_scanline_someplace wants a pointer and sample count. */
    	AddScan((char *)buffer[0], row_stride);
    }
    
    
  (void) jpeg_finish_decompress(&cinfo);
     jpeg_destroy_decompress(&cinfo);
       fclose(infile);
    Img * img = new Img;
    img->_buf = _buf;
    img->_w = _w;
    img->_h = _h;
    img->_c = _c;
    return img;
    	 	 
	}
	void AddScan( char * line,int len)
	{
		char *nb = _buf;
		nb = nb + ( _ln * _w * _c );
		for(int i=0;i<len;i++)
		{
			nb[i] = line[i];
		}
		_ln++;
	}
	
	char *_buf;	 
	int _w,_h,_c,_ln;
};


class Cell
{
public:
	Cell()
	{
		_rgb = NULL;
		_raw = NULL;
		_index=0;
		_loaded=0;
	}
	char *_raw,rl;
	char *_rgb;
	int _index;
	int _loaded;
};

class Frame
{
public:
	Cell *_cell[16][16];
};

class Pva
{
public:
	void Reopen()
	{
			
	}
	Pva( const char * file )
	{
		_file = file;
		FILE *files;		
		files = fopen(file, "rb");
		fread(&_numframes,4,1,files);
		fread(&_numcells,4,1,files);
		_fstr = files;
		_framenum = 1;
		_active = new Frame;
		for(int x=0;x<16;x++)
		{
			for(int y=0;y<16;y++)
			{
				_active->_cell[x][y] = new Cell;
				_active->_cell[x][y]->_index = -1; 
			}
		}
		
		for(int i=0;i<_numcells;i++)
		{
			fread(&_coff[i],4,1,files);
			fread(&_clen[i],4,1,files);
			_cell[ i ] = new Cell;
			_cell[ i ]->_index = -1;
			_cell[ i ]->_loaded = false;
		}
		fseek( files,_coff[_numcells-1]+_clen[_numcells-1],SEEK_SET);
		for(int i=0;i<_numframes;i++)
		{
			Frame * frm = new Frame;
			_frame[ i ] = frm;
			for(int x=0;x<16;x++)
			{
				for(int y=0;y<16;y++)
				{
					frm->_cell[x][y] = new Cell;
					fread( &frm->_cell[x][y]->_index,4,1,files );			
				}
			}
		}
					
		_framergb = (char *)malloc(256*256*3);	
		_fin = false;
	   NextFrame();
	
	}
	Cell * GetCell( int index )
	{
		Cell *c = _cell[index];
		if( c->_loaded == false )
		{
			fseek( _fstr,_coff[ index ],SEEK_SET );
			c->_raw = (char *)malloc( _clen[index] );
			c->_rgb = (char *)malloc( 16 * 16 * 3 );
			fread( c->_raw,1,_clen[ index ],_fstr );
			uLongf dlen = 16 * 16 * 3;
			uncompress( (Bytef *)c->_rgb,&dlen,(Bytef *)c->_raw,(uLongf)_clen[index]);
			c->_loaded = true;
		}
		return c;
	}
	void NextFrame()
	{
		_frameindex++;
		if(_frameindex==_numframes)
		{
			_frameindex=0;
		}
		SetFrame( (int)_frameindex );		
	}
	void SetFrame( int frame )
	{
		if( (frame == _numframes) && (frame > _numframes) ) frame = _numframes-1;
		if( frame<0 ) frame = 0;
		_frameindex = frame;
		Frame * f = _frame[ frame ];
		for(int x=0;x<16;x++)
		{
			for(int y=0;y<16;y++)
			{
				int index = f->_cell[x][y]->_index;
				if(!(index == _active->_cell[x][y]->_index))
				{
					Cell * c = GetCell( f->_cell[x][y]->_index );
					_active->_cell[x][y]->_index = index;
					int rx,ry;
					for(int px=0;px<16;px++)
					{
						rx = x * 16 + px;
						for(int py=0;py<16;py++)
						{
							ry = y * 16 + py;
							int offset = (ry * 256 * 3)+(rx*3);
							int poff =  (py * 16 *3)+(px*3);
							_framergb[ offset ] = c->_rgb[ poff ];
							_framergb[ offset+1 ] = c->_rgb[ poff+1 ];
							_framergb[ offset+2 ] = c->_rgb[ poff+2 ]; 
						}
					}
				}
				
			}
		}
	}
	void CloseFile()
	{	
		fclose(_fstr);
		_fstr = 0;
		_fin = 1;
		//Reopen();
	}
	int Playing()
	{
		return 1-_fin;
	}
	const char *_file;
	Frame *_active;
	int _framenum;	
	FILE *_fstr;
	int _numframes;
	int _numcells;
	Cell *_cell[50000];
	int _clen[50000];
	int _coff[50000];
	Frame *_frame[50000];
	int *_foff[50000];
	char *_framergb;
	int _fin;
	float _frameindex,_speed;
}; 

 
  

class Display
{
public:
	Display(int argc,char **argv)
	{
		glutInit(&argc,argv);
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);  
	  glutInitWindowSize(480, 272); // @@@
  	glutInitWindowPosition(0, 0);  
 		_win = glutCreateWindow("Raptor Engine - Main Window");  
   	_w = 480;
		_h = 272;
    InitGl();
		Draw2D();
	
	}
	void InitGl()
	{
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		// This Will Clear The Background Color To Black
		  glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
		  glDepthFunc(GL_LESS);				// The Type Of Depth Test To Do
		  glEnable(GL_DEPTH_TEST);			// Enables Depth Testing
		  glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading
		
		  glMatrixMode(GL_PROJECTION);
		  glLoadIdentity();				// Reset The Projection Matrix
		
		  gluPerspective(45.0f,(float)_w/(float)_h,0.1f,100.0f);	// Calculate The Aspect Ratio Of The Window
		  glMatrixMode(GL_MODELVIEW);
		  //glEnable(GL_TEXTURE_2D);
	}
	void Draw2D()
	{
		
		
		glViewport( 0,0,480,272 );
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0, 480.0, 0.0, 272.0, -2.0, 2.0 );
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);	
	
	}
	
	int _w,_h;
	int _win;
};


enum ImageFormat{ JPEG,BMP };

class Texture
{
public:
	Texture( int w,int h )
	{
		_coordset=0;
		_w = w;
			_h = h;
			glGenTextures(1, &_gltex);
		  glBindTexture(GL_TEXTURE_2D, _gltex);   // 2d texture (x and y size)
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
		  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
			glBindTexture(GL_TEXTURE_2D,0);
	}
	Texture(char *buf,int w,int h)
	{
			_coordset=0;
			_w = w;
			_h = h;
			glGenTextures(1, &_gltex);
		  glBindTexture(GL_TEXTURE_2D, _gltex);   // 2d texture (x and y size)
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
		  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
			glTexImage2D(GL_TEXTURE_2D, 0, 3, _w, _h, 0, GL_RGB, GL_UNSIGNED_BYTE,buf );
			glBindTexture(GL_TEXTURE_2D,0);
	}
	void Upload( char * buf )
	{
		// Todo:Switch to texsubimage once I've verified it works.
		glTexImage2D(GL_TEXTURE_2D, 0, 3, _w, _h, 0, GL_RGB, GL_UNSIGNED_BYTE,buf );
	}
	void UseCoords( int set )
	{
		_coordset = set;
	}
	Texture(char *file,ImageFormat form)
	{
	
		_coordset=0;
		_filename = file;
		char *tex;
		switch( form )
		{
			case JPEG:
				JpgLoader *jl = new JpgLoader;
				Img * img = jl->LoadJpg( (const char *) file );
				if( img == NULL )
				{
					printf("Unable to load texture\n");
					Logger->Log("Unable to load jpg %s \n",file);
					exit(0);
				}
				else
				{
					Logger->Log("Loaded jpg %s \n",file );
				}
				
				_w = img->_w;
				_h = img->_h;
				glGenTextures(1, &_gltex);
		    glBindTexture(GL_TEXTURE_2D, _gltex);   // 2d texture (x and y size)
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
		   	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
				tex = (char *)malloc(_w*_h*4);
					for(int x=0;x<_w;x++)
					{
						for(int y=0;y<_h;y++)
						{
							char *b = tex+(y*_w*4)+x*4;
							char *a = img->_buf+( ((_h-1)-y)*_w*3)+x*3;
							b[0] = a[0];
							b[1] = a[1];
							b[2] = a[2];
							int targ = (int)b[0] + (int)b[1] + (int)b[2];
							if( targ>0 )
							{
								b[3] = 255;
							}
							else
							{
								b[3] = 0;
							}
							b[3] = targ;
							
						}
					}			
					
					glTexImage2D(GL_TEXTURE_2D, 0, 4, _w, _h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
					glBindTexture(GL_TEXTURE_2D,0);
					_raw = tex;
				break;
			case BMP:
				 Image *image1;
		     image1 = (Image *) malloc(sizeof(Image));
		    	if (image1 == NULL) {
						printf("Error:Unable to allocate memory for texture.\n");
						exit(0);
		    	}
		      if (!ImageLoad(file, image1)) {
						printf("Error:Unable to load image.\n");
						Logger->Log("Could not load texture %s \n",file);
						return;
					}
					Logger->Log("Loaded texture.%s \n",file);
					_w = image1->sizeX;
					_h = image1->sizeY;
		    	glGenTextures(1, &_gltex);
		    	glBindTexture(GL_TEXTURE_2D, _gltex);   // 2d texture (x and y size)
				  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
		   	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST); 
					
					tex = (char *)malloc(image1->sizeX*image1->sizeY*4);
					for(int x=0;x<image1->sizeX;x++)
					{
						for(int y=0;y<image1->sizeY;y++)
						{
							char *b = tex+(y*image1->sizeX*4)+x*4;
							char *a = image1->data+(y*image1->sizeX*3)+x*3;
							b[0] = a[0];
							b[1] = a[1];
							b[2] = a[2];
							int targ = (int)b[0] + (int)b[1] + (int)b[2];
							if( targ>0 )
							{
								b[3] = 255;
							}
							else
							{
								b[3] = 0;
							}
							b[3] = targ;
							
						}
					}			
					
		    	//glTexImage2D(GL_TEXTURE_2D, 0, 4, image1->sizeX, image1->sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
					gluBuild2DMipmaps(GL_TEXTURE_2D, 4, image1->sizeX, image1->sizeY,
 		            GL_RGBA, GL_UNSIGNED_BYTE, tex);
 		      glBindTexture(GL_TEXTURE_2D,0);
					_raw = tex;
			break;
		}
		
			
	}
	void Bind()
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,_gltex);
	//	Logger->Log("Bound %s to texunit.\n",_filename);
	}
	void Unbind()
	{
		glBindTexture(GL_TEXTURE_2D,0);
		glDisable(GL_TEXTURE_2D);
	}
	int _w,_h;
	GLuint _gltex;
	const char *_filename;
	int _coordset;
	char *_raw;
};


int Hundred( int val )
{
	return (int)val/100;
}

int Ten( int val )
{
		
	float fVal = (float)val / 100.0f;
	
	int Tval = (int)fVal * 10;
		
	val = Tval * 10;
	
	return (int)(val-(Hundred(val)*100) )/10;

}

int Digit( int val )
{
	return val - (Hundred(val)*100+Ten(val)*10);
}


class Font
{
public:
	Font(char * file,float charwidth=32,float charheight=32,float charsperline=16,float xinc=10)
	{
		_tex = new Texture(file,BMP);
		_cw = charwidth;
		_ch = charheight;
		_mw = _tex->_w;
		_mh = _tex->_h;
		_xinc = xinc;
		_cpl = charsperline;
	}
	float _cw,_ch;
	float _mw,_mh,_cpl,_xinc;
	
	Texture *_tex;
};

class FontEngine
{
public:
	FontEngine()
	{
		_active = NULL;
		_r = 1;
		_g = 1;
		_b = 1;
	}
	void SetActive(Font *font)
	{
		_active = font;
	}
	void SetColor(float r,float g,float b)
	{
		_r=r;
		_g=g;
		_b=b;
	}
	int TextWidth( char * txt )
	{
		int l = TextLen( txt )*_active->_xinc;
		return l;
	}
	
	int TextHeight()
	{
		return _active->_ch;
	}
	int TextLen( char * txt )
	{
		int c = 0;
		while(1)
		{
			if( txt[c] == 0 ) break;
				c++;
		}
		return c;
	}
	
	void RenderText(float x,float y,char *txt)
	{
		int cc=0;
		int chr;
		Blend_Mask();
		glEnable(GL_TEXTURE_2D);
		_active->_tex->Bind();
	
		float dx,dy;
		dx = x;
		dy = 272-y;
		while(1)
		{
			chr = txt[cc];
			if( chr == 0 ) break;
			int xo,yo;
			yo = (chr/(int)_active->_cpl);
			xo = (chr-(yo *(int) _active->_cpl ));
			float ax,ay;
			ax = xo * _active->_cw;
			ay = yo * _active->_ch;
			float ex,ey;
			ex = ax + _active->_cw;
			ey = ay + _active->_ch;
			
			float u0,v0,u1,v1;
			u0 = ax / _active->_mw;
			v0 = 1-(ay / _active->_mh);
			u1 = ex / _active->_mw;
			v1 = 1-(ey / _active->_mh);
			glBegin(GL_QUADS);
			glTexCoord2f( u0,v0 );
			glVertex2f( dx,dy );
			
			glTexCoord2f( u1,v0 );
			glVertex2f( dx+_active->_cw,dy );
			
			glTexCoord2f( u1,v1 );
			glVertex2f( dx+_active->_cw,dy-_active->_ch );
			
			glTexCoord2f( u0,v1 );
			glVertex2f( dx,dy-_active->_ch );
			glEnd();
			dx += _active->_xinc;
						
			cc++; 
		}
		
		_active->_tex->Unbind();
		glDisable(GL_TEXTURE_2D);
		
	}
	float _r,_g,_b;
	Font *_active;
};

enum BlendMode{ Solid,Add,Mask,Alpha };

class Material
{
public:
	Material()
	{
		_r=_g=_b=_a=1;
		_blend = Solid;
	}
	void AddTexture(Texture *tex)
	{
		_texs.add( tex );
	}
	void SetBlend( BlendMode mode )
	{
		_blend = mode;
	}
	void Bind( int stage = 0 )
	{
		glDisable(GL_TEXTURE_2D);
		glColor4f( _r,_g,_b,_a );
		switch( _blend )
		{
			case Solid:
				Blend_Solid();
				break;
			case Alpha:
				Blend_Alpha();
				break;
			case Mask:
				Blend_Mask();
				break;
			case Add:
				//Do nothing.
				break;
		}
		_texs.start();
		while( _texs.next()==true )
		{
			Texture *tex = _texs.get();
			//Logger->Log("Tex %s \n",tex->_filename);
			if(stage == 0)
			{
				tex->Bind();
			}
			stage--;
		}
	}
	void Unbind( int stage = 0)
	{
		_texs.start();
		while( _texs.next()==true )
		{
			if(stage == 0)
			{
				Texture *tex = _texs.get();
				tex->Unbind();
			}
			stage--;
		}
	}
	float _r,_g,_b,_a;
	BlendMode _blend;
	List<Texture *>_texs;
};

const float PI = 3.14159265;

inline float DegToRad(float ang)
{

	return ang*PI/180.0f;
}

inline float RadToDeg(float rad)
{

	return rad * 180.0f / PI;
}

ScePspFMatrix4 *matrix4mul(ScePspFMatrix4 *out, const ScePspFMatrix4 *in1, const ScePspFMatrix4 *in2) 
{ 
   asm volatile ( 
      "ulv.q         c500, 0 + %1\n"        
      "ulv.q         c510, 16 + %1\n"    
      "ulv.q         c520, 32 + %1\n"        
      "ulv.q         c530, 48 + %1\n"  
      "ulv.q         c600,  0 + %2\n"        
      "ulv.q         c610, 16 + %2\n" 
      "ulv.q         c620, 32 + %2\n"    
      "ulv.q         c630, 48 + %2\n" 
      "vmmul.q      e400, e500, e600\n"    
      "usv.q         c400,  0 + %0\n"    
      "usv.q         c410, 16 + %0\n" 
      "usv.q         c420, 32 + %0\n"        
      "usv.q         c430, 48 + %0\n" 
      : "=m"(*out) 
      : "m"(*in1), "m"(*in2) : "memory" 
   ); 
   return out; 
}

class Matrix
{
public:
		
	void LoadIdentity()
	{
		grid[0][0]=1.0;
		grid[1][0]=0.0;
		grid[2][0]=0.0;
		grid[3][0]=0.0;
		grid[0][1]=0.0;
		grid[1][1]=1.0;
		grid[2][1]=0.0;
		grid[3][1]=0.0;
		grid[0][2]=0.0;
		grid[1][2]=0.0;
		grid[2][2]=1.0;
		grid[3][2]=0.0;
		
		grid[0][3]=0.0;
		grid[1][3]=0.0;
		grid[2][3]=0.0;
		grid[3][3]=1.0;
	}
	
	
		/*

		*/
	
	void Multiply2(Matrix *i)
	{
		
		
		Matrix r;
		r.grid[0][0] = grid[0][0] * i->grid[0][0] + grid[0][1] * i->grid[1][0] + grid[0][2] * i->grid[2][0] + grid[0][3] * i->grid[3][0];
		r.grid[0][1] = grid[0][0] * i->grid[0][1] + grid[0][1] * i->grid[1][1] + grid[0][2] * i->grid[2][1] + grid[0][3] * i->grid[3][1];
		r.grid[0][2] = grid[0][0] * i->grid[0][2] + grid[0][1] * i->grid[1][2] + grid[0][2] * i->grid[2][2] + grid[0][3] * i->grid[3][2];
		r.grid[0][3] = grid[0][0] * i->grid[0][3] + grid[0][1] * i->grid[1][3] + grid[0][2] * i->grid[2][3] + grid[0][3] * i->grid[3][3];
		
		r.grid[1][0] = grid[1][0] * i->grid[0][0] + grid[1][1] * i->grid[1][0] + grid[1][2] * i->grid[2][0] + grid[1][3] * i->grid[3][0];
		r.grid[1][1] = grid[1][0] * i->grid[0][1] + grid[1][1] * i->grid[1][1] + grid[1][2] * i->grid[2][1] + grid[1][3] * i->grid[3][1];
		r.grid[1][2] = grid[1][0] * i->grid[0][2] + grid[1][1] * i->grid[1][2] + grid[1][2] * i->grid[2][2] + grid[1][3] * i->grid[3][2];
		r.grid[1][3] = grid[1][0] * i->grid[0][3] + grid[1][1] * i->grid[1][3] + grid[1][2] * i->grid[2][3] + grid[1][3] * i->grid[3][3];
		
		r.grid[2][0] = grid[2][0] * i->grid[0][0] + grid[2][1] * i->grid[1][0] + grid[2][2] * i->grid[2][0] + grid[2][3] * i->grid[3][0];
		r.grid[2][1] = grid[2][0] * i->grid[0][1] + grid[2][1] * i->grid[1][1] + grid[2][2] * i->grid[2][1] + grid[2][3] * i->grid[3][1];
		r.grid[2][2] = grid[2][0] * i->grid[0][2] + grid[2][1] * i->grid[1][2] + grid[2][2] * i->grid[2][2] + grid[2][3] * i->grid[3][2];
		r.grid[2][3] = grid[2][0] * i->grid[0][3] + grid[2][1] * i->grid[1][3] + grid[2][2] * i->grid[2][3] + grid[2][3] * i->grid[3][3];
		
		r.grid[3][0] = grid[3][0] * i->grid[0][0] + grid[3][1] * i->grid[1][0] + grid[3][2] * i->grid[2][0] + grid[3][3] * i->grid[3][0];
		r.grid[3][1] = grid[3][0] * i->grid[0][1] + grid[3][1] * i->grid[1][1] + grid[3][2] * i->grid[2][1] + grid[3][3] * i->grid[3][1];
		r.grid[3][2] = grid[3][0] * i->grid[0][2] + grid[3][1] * i->grid[1][2] + grid[3][2] * i->grid[2][2] + grid[3][3] * i->grid[3][2];
		r.grid[3][3] = grid[3][0] * i->grid[0][3] + grid[3][1] * i->grid[1][3] + grid[3][2] * i->grid[2][3] + grid[3][3] * i->grid[3][3];
		grid[0][0] = r.grid[0][0];
		grid[0][1] = r.grid[0][1];
		grid[0][2] = r.grid[0][2];
		grid[0][3] = r.grid[0][3];
		grid[1][0] = r.grid[1][0];
		grid[1][1] = r.grid[1][1];
		grid[1][2] = r.grid[1][2];
		grid[1][3] = r.grid[1][3];
		grid[2][0] = r.grid[2][0];
		grid[2][1] = r.grid[2][1];
		grid[2][2] = r.grid[2][2];
		grid[2][3] = r.grid[2][3];
		grid[3][0] = r.grid[3][0];
		grid[3][1] = r.grid[3][1];
		grid[3][2] = r.grid[3][2];
		grid[3][3] = r.grid[3][3]; 
	}
	
	void MultiplyVFPU(Matrix *mat)
	{
			
		 ScePspFMatrix4 m1; 
     ScePspFMatrix4 m2; 
     ScePspFMatrix4 out; 
			
		 m1.x.x = grid[0][0];
		 m1.x.y = grid[0][1];
		 m1.x.z = grid[0][2];
		 m1.x.w = grid[0][3];
		 
		 m1.y.x = grid[1][0];
		 m1.y.y = grid[1][1];
		 m1.y.z = grid[1][2];
		 m1.y.w = grid[1][3];
		 
		 m1.z.x = grid[2][0];
		 m1.z.y = grid[2][1];
		 m1.z.z = grid[2][2];
		 m1.z.w = grid[2][3];
		 
		 m1.w.x = grid[3][0];
		 m1.w.y = grid[3][1];
		 m1.w.z = grid[3][2];
		 m1.w.w = grid[3][3];
		 
		 m2.x.x = mat->grid[0][0];
		 m2.x.y = mat->grid[0][1];
		 m2.x.z = mat->grid[0][2];
		 m2.x.w = mat->grid[0][3];
		 
		 m2.y.x = mat->grid[1][0];
		 m2.y.y = mat->grid[1][1];
		 m2.y.z = mat->grid[1][2];
		 m2.y.w = mat->grid[1][3];
		 
		 m2.z.x = mat->grid[2][0];
		 m2.z.y = mat->grid[2][1];
		 m2.z.z = mat->grid[2][2];
		 m2.z.w = mat->grid[2][3];
		 
		 m2.w.x = mat->grid[3][0];
		 m2.w.y = mat->grid[3][1];
		 m2.w.z = mat->grid[3][2];
		 m2.w.w = mat->grid[3][3];
		
			
     matrix4mul( &out,&m1,&m2);
		
		grid[0][0] = out.x.x;
		grid[0][1] = out.x.y;
		grid[0][2] = out.x.z;
		grid[0][3] = out.x.w;
		
		grid[1][0] = out.y.x;
		grid[1][1] = out.y.y;
		grid[1][2] = out.y.z;
		grid[1][3] = out.y.w;
		
		grid[2][0] = out.z.x;
		grid[2][1] = out.z.y;
		grid[2][2] = out.z.z;
		grid[2][3] = out.z.w;
		
		grid[3][0] = out.w.x;
		grid[3][1] = out.w.y;
		grid[3][2] = out.w.z;
		grid[3][3] = out.w.w;
		
		
	
	}
	
	void Multiply(Matrix *mat)
	{
		
		//Logger->Log("About to mult mat.\n");
		
	
		Matrix new_mat;
	
		
		new_mat.grid[0][0]=(grid[0][0]*mat->grid[0][0]) + (grid[1][0]*mat->grid[0][1]) + (grid[2][0]*mat->grid[0][2]) + (grid[3][0]*mat->grid[0][3]); 
		new_mat.grid[0][1]=(grid[0][1]*mat->grid[0][0]) + (grid[1][1]*mat->grid[0][1]) + (grid[2][1]*mat->grid[0][2]) + (grid[3][1]*mat->grid[0][3]); 
		new_mat.grid[0][2]=(grid[0][2]*mat->grid[0][0]) + (grid[1][2]*mat->grid[0][1]) + (grid[2][2]*mat->grid[0][2]) + (grid[3][2]*mat->grid[0][3]);
		new_mat.grid[0][3]=(grid[0][3]*mat->grid[0][0]) + (grid[1][3]*mat->grid[0][1]) + (grid[2][3]*mat->grid[0][2]) + (grid[3][3]*mat->grid[0][3]); 

		new_mat.grid[1][0]=(grid[0][0]*mat->grid[1][0]) + (grid[1][0]*mat->grid[1][1]) + (grid[2][0]*mat->grid[1][2]) + (grid[3][0]*mat->grid[1][3]); 
		new_mat.grid[1][1]=(grid[0][1]*mat->grid[1][0]) + (grid[1][1]*mat->grid[1][1]) + (grid[2][1]*mat->grid[1][2]) + (grid[3][1]*mat->grid[1][3]); 
		new_mat.grid[1][2]=(grid[0][2]*mat->grid[1][0]) + (grid[1][2]*mat->grid[1][1]) + (grid[2][2]*mat->grid[1][2]) + (grid[3][2]*mat->grid[1][3]); 
		new_mat.grid[1][3]=(grid[0][3]*mat->grid[1][0]) + (grid[1][3]*mat->grid[1][1]) + (grid[2][3]*mat->grid[1][2]) + (grid[3][3]*mat->grid[1][3]); 

		new_mat.grid[2][0]=(grid[0][0]*mat->grid[2][0]) + (grid[1][0]*mat->grid[2][1]) + (grid[2][0]*mat->grid[2][2]) + (grid[3][0]*mat->grid[2][3]); 
		new_mat.grid[2][1]=(grid[0][1]*mat->grid[2][0]) + (grid[1][1]*mat->grid[2][1]) + (grid[2][1]*mat->grid[2][2]) + (grid[3][1]*mat->grid[2][3]); 
		new_mat.grid[2][2]=(grid[0][2]*mat->grid[2][0]) + (grid[1][2]*mat->grid[2][1]) + (grid[2][2]*mat->grid[2][2]) + (grid[3][2]*mat->grid[2][3]); 
		new_mat.grid[2][3]=(grid[0][3]*mat->grid[2][0]) + (grid[1][3]*mat->grid[2][1]) + (grid[2][3]*mat->grid[2][2]) + (grid[3][3]*mat->grid[2][3]); 

		new_mat.grid[3][0]=(grid[0][0]*mat->grid[3][0]) + (grid[1][0]*mat->grid[3][1]) + (grid[2][0]*mat->grid[3][2]) + (grid[3][0]*mat->grid[3][3]); 
		new_mat.grid[3][1]=(grid[0][1]*mat->grid[3][0]) + (grid[1][1]*mat->grid[3][1]) + (grid[2][1]*mat->grid[3][2]) + (grid[3][1]*mat->grid[3][3]); 
		new_mat.grid[3][2]=(grid[0][2]*mat->grid[3][0]) + (grid[1][2]*mat->grid[3][1]) + (grid[2][2]*mat->grid[3][2]) + (grid[3][2]*mat->grid[3][3]); 
		new_mat.grid[3][3]=(grid[0][3]*mat->grid[3][0]) + (grid[1][3]*mat->grid[3][1]) + (grid[2][3]*mat->grid[3][2]) + (grid[3][3]*mat->grid[3][3]); 

	
		grid[0][0] = new_mat.grid[0][0];
		grid[0][1] = new_mat.grid[0][1];
		grid[0][2] = new_mat.grid[0][2];
		grid[0][3] = new_mat.grid[0][3];
		grid[1][0] = new_mat.grid[1][0];
		grid[1][1] = new_mat.grid[1][1];
		grid[1][2] = new_mat.grid[1][2];
		grid[1][3] = new_mat.grid[1][3];
		grid[2][0] = new_mat.grid[2][0];
		grid[2][1] = new_mat.grid[2][1];
		grid[2][2] = new_mat.grid[2][2];
		grid[2][3] = new_mat.grid[2][3];
		grid[3][0] = new_mat.grid[3][0];
		grid[3][1] = new_mat.grid[3][1];
		grid[3][2] = new_mat.grid[3][2];
		grid[3][3] = new_mat.grid[3][3]; 
		
	
	}
	
	Matrix * Copy()
	{
		Matrix *mat = new Matrix;
		mat->grid[0][0]=grid[0][0];
		mat->grid[1][0]=grid[1][0];
		mat->grid[2][0]=grid[2][0];
		mat->grid[3][0]=grid[3][0];
		mat->grid[0][1]=grid[0][1];
		mat->grid[1][1]=grid[1][1];
		mat->grid[2][1]=grid[2][1];
		mat->grid[3][1]=grid[3][1];
		mat->grid[0][2]=grid[0][2];
		mat->grid[1][2]=grid[1][2];
		mat->grid[2][2]=grid[2][2];
		mat->grid[3][2]=grid[3][2];
		mat->grid[0][3]=grid[0][3];
		mat->grid[1][3]=grid[1][3];
		mat->grid[2][3]=grid[2][3];
		mat->grid[3][3]=grid[3][3];
		return mat;
	}
	
		
	void Overwrite(Matrix *mat)
	{
		grid[0][0]=mat->grid[0][0];
		grid[1][0]=mat->grid[1][0];
		grid[2][0]=mat->grid[2][0];
		grid[3][0]=mat->grid[3][0];
		grid[0][1]=mat->grid[0][1];
		grid[1][1]=mat->grid[1][1];
		grid[2][1]=mat->grid[2][1];
		grid[3][1]=mat->grid[3][1];
		grid[0][2]=mat->grid[0][2];
		grid[1][2]=mat->grid[1][2];
		grid[2][2]=mat->grid[2][2];
		grid[3][2]=mat->grid[3][2];
		grid[0][3]=mat->grid[0][3];
		grid[1][3]=mat->grid[1][3];
		grid[2][3]=mat->grid[2][3];
		grid[3][3]=mat->grid[3][3];
		
	}
	
	void Transform(float x,float y,float z)
	{
	/*
	return Vector3(
		_11 * r.x + _12 * r.y + _13 * r.z + _14, 
		_21 * r.x + _22 * r.y + _23 * r.z + _24,
		_31 * r.x + _32 * r.y + _33 * r.z + _34);
	*/
		float rx,ry,rz;
		rx = grid[0][0] * x + grid[0][1] * y + grid[0][2] * z + grid[0][3];
		ry = grid[1][0] * x + grid[1][1] * y + grid[1][2] * z + grid[1][3];
		rz = grid[2][0] * x + grid[2][1] * y + grid[2][2] * z + grid[2][3];
		grid[3][0] = rx;
		grid[3][1] = ry;
		grid[3][2] = rz;
	}
	
	void Translate(float x,float y,float z)
	{
	
		float fx,fy,fz,fw; 
		fx=(grid[0][0]*x) + (grid[1][0]*y) + (grid[2][0]*z) + grid[3][0]; 
		fy=(grid[0][1]*x) + (grid[1][1]*y) + (grid[2][1]*z) + grid[3][1]; 
		fz=(grid[0][2]*x) + (grid[1][2]*y) + (grid[2][2]*z) + grid[3][2]; 
		fw=(grid[0][3]*x) + (grid[1][3]*y) + (grid[2][3]*z) + grid[3][3]; 
		grid[3][0] = fx;
		grid[3][1] = fy;
		grid[3][2] = fz;
		grid[3][3] = fw;
		

	}
	void Rotate( float ax,float ay,float az )
	{
		RotateYaw( ay );
		RotatePitch( ax );
		RotateRoll( az );
	}
	void RotatePYR( float ax,float ay,float az)
	{
		RotatePitch( ax );
		RotateYaw( ay );
		RotateRoll( az );
	}
	void RotateI( float ax,float ay,float az )
	{
		RotatePitch( az );
		RotateRoll( az );
		RotateYaw( ay );
	}
	void RotateRPY( float ax,float ay,float az )
	{
		RotateRoll( az );
		RotatePitch( ax );
		RotateYaw( ay );
	}
	void RotateYaw( float ang )
	{
		
		Matrix *mat=new Matrix;
		ang = DegToRad( ang );
		mat->grid[0][0]=cos(ang);
		mat->grid[1][0]=0;
		mat->grid[2][0]=sin(ang);
		mat->grid[3][0]=0;
		mat->grid[0][1]=0;
		mat->grid[1][1]=1;
		mat->grid[2][1]=0;
		mat->grid[3][1]=0;
		mat->grid[0][2]=-sin(ang);
		mat->grid[1][2]=0;
		mat->grid[2][2]=cos(ang);
		mat->grid[3][2]=0;
		
		mat->grid[0][3]=0;
		mat->grid[1][3]=0;
		mat->grid[2][3]=0;
		mat->grid[3][3]=1;
		
		Multiply(mat);
		delete mat;
	

	}
	void RotateRoll( float ang )
	{
		
		Matrix *mat=new Matrix;
		ang = DegToRad( ang );
		
		mat->grid[0][0]=cos(ang);
		mat->grid[1][0]=-sin(ang);
		mat->grid[2][0]=0;
		mat->grid[3][0]=0;
		mat->grid[0][1]=sin(ang);
		mat->grid[1][1]=cos(ang);
		mat->grid[2][1]=0;
		mat->grid[3][1]=0;
		mat->grid[0][2]=0;
		mat->grid[1][2]=0;
		mat->grid[2][2]=1;
		mat->grid[3][2]=0;
		
		mat->grid[0][3]=0;
		mat->grid[1][3]=0;
		mat->grid[2][3]=0;
		mat->grid[3][3]=1;
		
		Multiply(mat);
		delete mat;		
	}
	void RotatePitch( float ang )
	{
		Matrix *mat=new Matrix;
		ang = DegToRad( ang );
		mat->grid[0][0]=1;
		mat->grid[1][0]=0;
		mat->grid[2][0]=0;
		mat->grid[3][0]=0;
		mat->grid[0][1]=0;
		mat->grid[1][1]=cos(ang);
		mat->grid[2][1]=-sin(ang);
		mat->grid[3][1]=0;
		mat->grid[0][2]=0;
		mat->grid[1][2]=sin(ang);
		mat->grid[2][2]=cos(ang);
		mat->grid[3][2]=0;
		
		mat->grid[0][3]=0;
		mat->grid[1][3]=0;
		mat->grid[2][3]=0;
		mat->grid[3][3]=1;
		
		Multiply(mat);
		delete mat;
	}
	
	
	float grid[4][4] __attribute__((aligned(16)));
};

#define true 1
#define false 0

const int Typ_Ent = 1;
const int Typ_Piv = 2;
const int Typ_Bas = 3;
const int Typ_Lgt = 4;

class Base;



void AnimateMesh(Base *ent1,float time,int first,int last);

class Base
{
public:
	Base()
	{
		_sx=1;
		_sy=1;
		
		_sz=1;
		_parent = NULL;
		_px=0;
		_py=0;
		_pz=0;
		_qw=0;
		_anim_dir = 1;
		_qx=0;
		_qy=0;
		_qz=0;
		_hidden = false;
		_order = 0;
		_mat = new Matrix;
		_anim = false;
		
	}
	Base( Base *parent )
	{
		_anim = false;
		_sx=1;
		_sy=1;
		_sz=1;
		_parent = NULL;
		_px=0;
		_py=0;
		_pz=0;
		_name = NULL;
		_qw=0;
		_qx=0;
		_qy=0;
		_anim_dir = 1;
		_qz=0;
		_hidden = false;
		_order = 0;
		_mat = new Matrix;
		_type = Cls_Base; 
	}
	void SetAnimTime(float time,int seq=0)
	{
	
		_anim_mode=-1; //' use a mode of -1 for setanimtime
		_anim_speed=0;
		_anim_seq=seq;
		_anim_trans=0;
		_anim_time=time;
		_anim_update=false; // set anim_update to false so UpdateWorld won't animate entity
			
		int first=_anim_seqs_first[_anim_seq];
		int last=_anim_seqs_last[_anim_seq];
		int first2last=_anim_seqs_last[_anim_seq]-_anim_seqs_first[_anim_seq];
		
		

		if( time>last )
		{
			do
			{
				time=time-first2last;
			} while( time>last );
		}
		if( time<first )
		{
			do
			{
				time=time+first2last;
			} while( time<first );
		}
	
		//Logger->Log("Time: %f ",time);
	//	Logger->Log("First: %d ",first);
	//	Logger->Log("Last: %d	\n",last);

		AnimateMesh(this,time,first,last);

			

	}
	List<Base *>_childs;
	Base *_parent;
	Matrix *_mat;
	float _px,_py,_pz;
	float _sx,_sy,_sz;
	float _rx,_ry,_rz;
	float _qw,_qx,_qy,_qz;
	int _order;
	int _hidden;
	const char *_name;
	float _ed;
	float _pit,_yaw;
	int _anim_render;
	float _anim_time;
	float _anim_speed;
	int _anim_mode;
	int _anim_seq;
	int _anim_trans;
	int _anim_dir;
	int _anim;
	int _anim_seqs_first[255];
	int _anim_seqs_last[255];
	int _no_seqs;
	int _anim_update;
	EntityClass _type;
	void AddParent( Base * parent )
	{

		if( parent == NULL ) 
		{
			return;
		}
	
		_parent=parent;

		if( !(parent==NULL))
		{
			_mat->Overwrite(parent->_mat);
			parent->_childs.add( this );
		}
	}
	int CountChildren()
	{
		_childs.start();
		int c =0;
		while( _childs.next() == true )
		{
			c++;
		}
		return c;
	}
	Base * GetChild( int index = 0 )
	{
		Base *ret=NULL;
		_childs.start();
		int c = 0;
		while( _childs.next() == true )
		{
			if( c == index )
			{
				ret = _childs.get();
			}
			c++;
		}
		return ret;
	}
	void PointAt( Base *tar )
	{
		PointAt( tar->XPos(false),tar->YPos(false),tar->ZPos(false) );
	}
	void PointAt( float tx,float ty,float tz,float roll=0 )
	{
		
	
	//	printf("Reached Function\n");
		float x=tx;
		float y=ty;
		float z=tz;

		float xdiff=XPos(true)-x;
		float ydiff=YPos(true)-y;
		float zdiff=ZPos(true)-z;

		float dist22=sqrt((xdiff*xdiff)+(zdiff*zdiff));
		float pitch= RadToDeg(atan2(ydiff,dist22));// *RAD_TO_DEG;
		float yaw= RadToDeg(atan2(xdiff,-zdiff));//  *RAD_TO_DEG;
		
		//printf("Reached Print\n");
		//printf(" Pit:%d Yaw:%d \n",(int)pitch,(int)yaw);
		_pit = pitch;
		_yaw = yaw;
		Rotate( pitch,yaw,roll,true );
	}
	
	
	
	void Position( float x,float y,float z,int global = false )
	{
		_px=x;
		_py=y;
		_pz=-z;
		
		if( (global==1) && (!(_parent==NULL)) )
		{
			
		
			_px=_px-_parent->XPos(true);
			_py=_py-_parent->YPos(true);
			_pz=_pz+_parent->ZPos(true);
		//	Logger->Log("Set.\n");
		 			
			float ax=Pitch(true);
			float ay=Yaw(true);
			float az=Roll(true);
					
			Matrix *new_mat=new Matrix;
			
			new_mat->LoadIdentity();
			new_mat->Rotate(-ax,-ay,-az);
			new_mat->Translate(_px,_py,_pz);

			_px=new_mat->grid[3][0];
			_py=new_mat->grid[3][1];
			_pz=new_mat->grid[3][2];
			
			delete new_mat;
			
		}
	
		if( !(_parent==NULL) )
		{
			
			_mat->Overwrite(_parent->_mat);
			UpdateMat();
			
		}

		if( _parent==NULL ) 
		{
			
			UpdateMat(true);
			
		}
		
		//UpdateChildren(this);
		
		
	}	
	

	void Rotate( float pitch,float yaw,float roll,int global = false)
	{
		
		_rx=-pitch;
		_ry=yaw;
		_rz=roll;
		
		if( (global==true) && (!(_parent==NULL )) )
		{
			_rx=_rx-_parent->Pitch(true);
			_ry=_ry-_parent->Yaw(true);
			_rz=_rz-_parent->Roll(true);
		}
				
		if( !(_parent==NULL) )
		{		
			_mat->Overwrite(_parent->_mat);
			UpdateMat();
		}
		if( _parent==NULL ) 
		{
			
			UpdateMat(true);
		}
		
		UpdateChildren(this);


	}
	

	
	float XPos(int global)
	{
		if( global == false )
				return _px;
		return _mat->grid[3][0];
	}
	float YPos(int global)
	{
		if( global == false )
			return _py;
		
		return _mat->grid[3][1];
	}
	float ZPos(int global)
	{
		if(global == false)
			return -_pz;
		return -_mat->grid[3][2];
	}
	
	float Roll(int global=false)
	{
		if(global == false)
		{
			return _rz;
		}
		float a=_mat->grid[0][1];
		float b=_mat->grid[1][1];
		if( fabs(a)<0.01 ) a=0;
		if( fabs(b)<0.01 ) b=0;
			
		return RadToDeg( atan2(a,b) );
			

	}
	
	
	float Yaw(int global=false)
	{
		if(global == false)
		{
			return _ry;
		}
		float a=_mat->grid[2][0];
		float b=_mat->grid[2][2];
		if( fabs(a)<0.01 ) a=0;
		if( fabs(b)<0.01 ) b=0;
		return RadToDeg( atan2(a,b) );
	}
	float Pitch(int global=false)
	{
		if(global == false)
		{
			return -_rx;
		}
	
		float ang = atan2( _mat->grid[2][1],sqrt( _mat->grid[2][0]*_mat->grid[2][0]+_mat->grid[2][2]*_mat->grid[2][2] ) );
		ang = RadToDeg( ang );
		if(fabs(ang)<0.01) ang=0;
		return ang;
	}
	
	void UpdateChildren(Base *from)
	{
		if( from == NULL ) return;
	
		from->_childs.start();
		
		while( from->_childs.next()==true )
		{
			Base *ent = from->_childs.get();
			switch(ent->_type)
			{
				case Cls_Bone:
					break;
				default:
					ent->_mat->Overwrite( from->_mat );
					ent->UpdateMat();
					UpdateChildren( ent );
					break;
			}
		}
		
		//Logger->Log("Left.\n");
		
		
	}
	
	void UpdateMat(int loadid = false)
	{
		
		if(loadid==true) _mat->LoadIdentity();
		_mat->Translate(_px,_py,_pz);								
		_mat->RotateYaw(_ry);
		_mat->RotatePitch(_rx);
		_mat->RotateRoll(_rz);
		//_mat.Scale(sx,sy,sz);
		
					
	}
	
	
};

List<Base *>entlist;


class Coordset
{
public:
	Coordset(int verts)
	{
		_uv = (float *)malloc( verts * 3 * 4 );
	}
	float *_uv;
};


class Visualizer
{
public:
	void SetData(float *verts,float *norms,int *cols,int *tris,int vertc,int tric,Material *mat)
	{
		_verts = verts;
		_tris = tris;
		_cols = cols;
		_norms = norms;
		_vertc = vertc;
		_tric = tric;
		_mat = mat;
		CreateResources();
		SyncData();
	}
	void SetCoords(int index,Coordset *coords)
	{
		_coords[index] = coords;
	}
	virtual void CreateResources() = 0;
	virtual void SyncData() = 0;
	virtual void Bind() = 0;
	virtual void Unbind() = 0;
	virtual void Render() = 0;
	virtual Visualizer * RequestNew() = 0;
	float *_verts;
	int *_cols;
	float *_norms;
	int *_tris;
	int _vertc;
	int _tric;
	
	Material *_mat;
	Coordset *_coords[3];
};

class VL_VertexBufferObject : public Visualizer
{
public:
	virtual Visualizer * RequestNew()
	{
		return (Visualizer *) new VL_VertexBufferObject;
	}
	virtual void CreateResources()
	{
		Logger->Log("Creating VBO resources.\n");
		glGenBuffersARB( 1, &_vbuf );
		glGenBuffersARB( 1, &_tbuf );
		glGenBuffersARB( 1, &_uvbuf );
		Logger->Log("Created.\n");
	}
	
	virtual void SyncData()
	{
		Logger->Log("Syncing data.\n");
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, _vbuf );	
		Logger->Log("Sending data.\n");
		glBufferDataARB( GL_ARRAY_BUFFER_ARB, _vertc*3*sizeof(float), (void *)_verts, GL_STATIC_DRAW_ARB );
		Logger->Log("Sent.\n");	
		_mat->_texs.start();
		int stage = 0;
		while( _mat->_texs.next() == true )
		{
			if( stage == 0 )
			{
				Texture *tex = _mat->_texs.get();
				Logger->Log("Binding uvbuf\n");
				glBindBufferARB( GL_ARRAY_BUFFER_ARB,_uvbuf );	
				Logger->Log("Bound.\n");
				Logger->Log("Sending data.\n");
				glBufferDataARB( GL_ARRAY_BUFFER_ARB, _vertc*3*sizeof(float),(void *)_coords[tex->_coordset]->_uv, GL_STATIC_DRAW_ARB );
				Logger->Log("Sent.\n");
			}
			stage++;
		}		
		Logger->Log("Synced.\n");
	}
	
	virtual void Bind()
	{
		Logger->Log("Binding VBO\n");
		glEnableClientState( GL_VERTEX_ARRAY );	
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
						
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, _vbuf );
		glVertexPointer( 3,GL_FLOAT,0,(char *)NULL);
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, _vbuf );
		glTexCoordPointer( 3,GL_FLOAT,0,(char *)NULL);
		Logger->Log("Bound.\n");
	}
	
	virtual void Unbind()
	{
		Logger->Log("Unbinding.\n");
		glDisableClientState( GL_VERTEX_ARRAY );	
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		Logger->Log("Unbound.\n");	
	}
	virtual void Render()
	{
		Logger->Log("Rendering VBO \n");
		glDrawElements(GL_TRIANGLES,_tric*3,GL_UNSIGNED_INT,_tris);
		Logger->Log("Rendered.\n");
	}
	GLuint _vbuf;
	GLuint _tbuf;
	GLuint _uvbuf;
};


class VL_VertexArray : public Visualizer
{
public:
	virtual Visualizer * RequestNew()
	{
		return (Visualizer *) new VL_VertexArray;
	}
	virtual void SyncData()
	{
		//Nothing to do.
	}
	virtual void CreateResources()
	{
		//Nothing to do.
	}
	virtual void Bind()
	{
	//	Profile->Enter("Bind\n");
		glEnableClientState(GL_VERTEX_ARRAY);
	//	glEnableClientState(GL_COLOR_ARRAY);
//		glEnableClientState(GL_NORMAL_ARRAY);
		glVertexPointer(3,GL_FLOAT,0,_verts);
		//glColorPointer(4, GL_UNSIGNED_BYTE,4, _cols);
		
		//glNormalPointer(GL_FLOAT,0,surface.norms#)
		_mat->Bind();
		_mat->_texs.start();
		while( _mat->_texs.next() == true )
		{
			Texture *tex = _mat->_texs.get();
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(3,GL_FLOAT,0,_coords[tex->_coordset]->_uv);
		}
	//	Profile->Leave("Bind\n");
	}
	virtual void Unbind()
	{
		//Profile->Enter("Unbind\n");
		glDisableClientState(GL_VERTEX_ARRAY);
//		glDisableClientState(GL_COLOR_ARRAY);
	
	//	glDisableClientState(GL_NORMAL_ARRAY);
		glVertexPointer(3,GL_FLOAT,0,NULL);
		//glcolorpointer(4,gl_float,0,Null
		//glnormalpointer GL_FLOAT,0,Null
		_mat->Unbind();
		_mat->_texs.start();
		while( _mat->_texs.next() == true )
		{
			Texture *tex = _mat->_texs.get();
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(3,GL_FLOAT,0,NULL);
		}	
	//	Profile->Leave("Unbind\n");
	}
	virtual void Render()
	{
		//Profile->Enter("VL_Render\n");
		glDrawElements(GL_TRIANGLES,_tric*3,GL_UNSIGNED_INT,_tris);
		//Profile->Leave("VL_Render\n");
	}
	
	
};


	unsigned int MakeCol( float r,float g,float b,float a )
{
	unsigned int col = (((int)(a*255.f)) << 24) +  (((int)(b*255.f)) << 16) +  (((int)(g*255.f)) << 8) +  ((int)(r*255.f));
	return col;
}




// Todo:Add some dynamic sizing so you can safelty build undefined meshes.
class Surface
{
public:
	Surface(int verts,int tris)
	{
		_verts = (float *)malloc( verts*3*4 );
		_cols = (int *)malloc( verts*4 );
		_tris = (int *)malloc( tris*3*4 );
		_vertm = verts;
		_trim = tris;
		_vertc=_tric=0;
		for(int i=0;i<3;i++)
		{
			_coords[i] = new Coordset(verts);
		}
		_visual = NULL;
	}
	void SetMaterial( Material * mat )
	{
		_mat = mat;
	}
	void Scale( float sx,float sy,float sz )
	{
		for(int i=0;i<_vertc;i++)
		{
		 _verts[ i * 3 ] *= sx;
		 _verts[ i * 3 + 1 ] *= sy;
		 _verts[ i * 3 + 2 ] *= sz;
	  }
	}
	void MoveVertex( int vertex,float x,float y,float z )
	{
		_verts[ vertex * 3 ] = x;
		_verts[ vertex * 3 + 1 ] = y;
		_verts[ vertex * 3 + 2 ] = z;
	}
	void SetCoords( int coordset,int vertex,float u,float v,float w = 1 )
	{
		_coords[coordset]->_uv[ vertex * 3] = u;
		_coords[coordset]->_uv[ vertex * 3 + 1 ] = v;
		_coords[coordset]->_uv[ vertex * 3 + 2 ] = w;
	}
	float VertexU(int coordset,int vertex)
	{
		return _coords[coordset]->_uv[ vertex * 3 ];
	}
	float VertexV(int coordset,int vertex)
	{
		return _coords[coordset]->_uv[ vertex * 3 +1];
	}
	float VertexW(int coordset,int vertex)
	{
		return _coords[coordset]->_uv[ vertex * 3 +2];
	}
	int AddVertex( float x,float y,float z,float u = 0,float v = 0,float w = 1 )
	{
		_verts[ _vertc*3 ] = x;
		_verts[ _vertc*3+1 ] = y;
		_verts[ _vertc*3+2 ] = z;
		_vertc++;
		SetCoords( 0,_vertc-1,u,v,w );
		VertexColor( _vertc-1,1,1,1,1 );
		return _vertc-1; 
	}
	void VertexColor( int vertex,float r,float g,float b,float a = 1 )
	{
		_cols[ vertex ] = MakeCol( r,g,b,a );
	}
	
	void SetTri( int tri,int v0,int v1,int v2 )
	{
		_tris[ tri *3 ] = v0;
		_tris[ tri *3 +1 ] = v1;
		_tris[ tri *3 +2 ] = v2;
	}
	int AddTriangle( int v0,int v1,int v2 )
	{
		_tris[ _tric*3 ] = v0;
		_tris[ _tric*3+1 ] = v1;
		_tris[ _tric*3+2 ] = v2;
		_tric++;
		return _tric-1;
	}
	float VertexX( int index )
	{
		return _verts[ index * 3 ];
	}
	float VertexY( int index )
	{
		return _verts[ index * 3 + 1 ] ; 
	}
	float VertexZ( int index )
	{
		return _verts[ index * 3 + 2 ] ;
	}
	int TriVertex( int tri,int vertex )
	{
		return _tris[ tri*3+vertex ];
	}
	void Render()
	{
		_visual->Render();
	}
	void RenderNative()
	{
		_mat->Bind(0);
		glColor4f(1,1,1,1);
		glBegin(GL_TRIANGLES);
		for(int j=0;j<_tric;j++)
		{
			for(int k=0;k<3;k++)
			{
				float u = VertexU( 0,TriVertex( j,k ) );
				float v =1-VertexV( 0,TriVertex( j,k ) );
				glTexCoord2f( u,v );
				glVertex3f( VertexX( TriVertex( j,k ) ),VertexY( TriVertex( j,k ) ),VertexZ( TriVertex(j,k) ) );
			//	Logger->Log( "X:%f ",VertexX( TriVertex(j,k) ));
		//		Logger->Log( "Y:%f ",VertexY( TriVertex(j,k) ));
			//	Logger->Log( "Z:%f \n",VertexZ( TriVertex(j,k) ));
			}
		}
		glEnd();
		_mat->Unbind(0);
	}
	void Bind()
	{
		_visual->Bind();
	}
	void Unbind()
	{
		_visual->Unbind();
	}
	void SyncVisualizer( )
	{
		_visual->SetData( _verts,NULL,_cols,_tris,_vertc,_tric,_mat);
		_visual->SyncData();
	}
	void SetVisualizer( Visualizer *visual,int SyncData = true )
	{
		_visual = visual;
		if( SyncData == true )
		{
			_visual->SetData( _verts,NULL,_cols,_tris,_vertc,_tric,_mat);
			for(int i=0;i<3;i++)
			{
				_visual->SetCoords( i,_coords[i] );
			}
			_visual->SyncData();
		}
	}
	int Vertices()
	{
		return _vertc;
	}
	int Triangles()
	{
		return _tric;
	}
	int _vertc,_tric;
	int _vertm,_trim;
	float *_verts;
	int *_cols;
	int *_tris;
	Coordset *_coords[3];
	Material *_mat;
	Visualizer *_visual;
};

class Keys 
{
public:
	int frames;
	int *flags;
	float *px;
	float *py;
	float *pz;
	float *sx;
	float *sy;
	float *sz;
	float *qw;
	float *qx;
	float *qy;
	float *qz;
};

class Bone : public Base
{
public:
	float _n_px,_n_py,_n_pz,_n_sx,_n_sy,_n_sz,_n_rx,_n_ry,_n_rz,_n_qw,_n_qx;
	float _n_qy,_n_qz;
	int no_verts;
	int * verts_id;
	float * verts_w;
	Keys * keys;
	Matrix *inv_mat;
	float _kx,_ky,_kz,_kqw,_kqx,_kqy,_kqz; 
	
};



class Entity : public Base
{
public:
	Entity(Base * parent)
	{
		_parent = parent;
	}

	void CopyTo(Entity *to)
	{
		_surf.start();
		while( _surf.next() == true )
		{
			to->_surf.add( _surf.get() );
		}
		
	}
	int CountSurfaces()
	{
		int sc=0;
		_surf.start();
		while( _surf.next() == true )
		{
			sc++;
		}
		return sc;
	}
	Surface *GetAnimSurface(int index)
	{
		Surface *ret = NULL;
		_anim_surf.start();
		while( _anim_surf.next() == true )
		{
			if( index == 0) ret = _anim_surf.get();
			index--; 
		}
		return ret;
	}
	Surface *GetSurface(int index)
	{
		Surface *ret = NULL;
		_surf.start();
		while( _surf.next() == true )
		{
			if( index == 0) ret = _surf.get();
			index--; 
		}
		return ret;
	}
	void AddSurface( Surface *in)
	{
		_surf.add( in );
	}
	void SyncVisualizer()
	{
		_surf.start();
		while( _surf.next() == true )
		{
			Surface *surf = _surf.get();
			surf->SyncVisualizer();
		}
		_anim_surf.start();
		while( _anim_surf.next() == true )
		{
			Surface *surf = _anim_surf.get();
			surf->SyncVisualizer();
		}
		_childs.start();
		while( _childs.next() == true )
		{
			Base * ent = (Base *)_childs.get();
			switch( ent->_type )
			{
				case Cls_Entity:
					Entity *nent = (Entity *)ent;
					nent->SyncVisualizer();
				break;
				case Cls_Bone:
					
				break;
				case Cls_Base:
					
				break;
			}
		}
	}
	void SetVisualizer( Visualizer *visual)
	{
		
		_surf.start();
		while( _surf.next() == true )
		{
			Surface *surf = _surf.get();
			surf->SetVisualizer( visual->RequestNew() );
		}
		_anim_surf.start();
		while( _anim_surf.next() == true )
		{
			Surface *surf = _anim_surf.get();
			surf->SetVisualizer( visual->RequestNew() );
		}
		_childs.start();
		while( _childs.next() == true )
		{
			Base * ent = (Base *)_childs.get();
			switch( ent->_type )
			{
				case Cls_Entity:
					Entity *nent = (Entity *)ent;
					nent->SetVisualizer( visual );
				break;
				case Cls_Bone:
					
				break;
				case Cls_Base:
					
				break;
			}
		}
	}
	void SetMaterial( Material *mat )
	{
		_surf.start();
		while( _surf.next() == true )
		{
			Surface * surf = _surf.get();
			surf->SetMaterial( mat );
		}
	}
	void ScaleMesh( float sx,float sy,float sz )
	{
		_surf.start();
		while( _surf.next() == true )
		{
			Surface * surf = _surf.get();
			surf->Scale( sx,sy,sz );
		}
	}
	void ClearSurfaces()
	{
		_surf.start();
		while( _surf.next()==true );
		{
			_surf.remove();
		}
		_surf.start();
	}
	
	void Render()
	{
		//Logger->Log("Entered render.\n");
		if( _anim_render == true )
		{
			_anim_surf.start();
			while( _anim_surf.next()==true )
			{
				Surface *surf = _anim_surf.get();
				if( surf->_visual == NULL)
				{
					break;
				}
			
				surf->Bind();
				surf->Render();
				surf->Unbind();
			}
		}
		else
		{
			_surf.start();
			while( _surf.next()==true )
			{
				Surface *surf = _surf.get();
				if(surf->_visual == NULL)
				{
					break;
				}
			
				surf->Bind();
				surf->Render();
				surf->Unbind();

			}
		}
	}
	void Cycle()
	{
		//Logger->Log("Entered Cycle.\n");
		if( _hidden == true ) 
			return ;
		glDisable(GL_TEXTURE_2D);
		_surf.start();
		glPushMatrix();
		glMultMatrixf( (GLfloat *)_mat->grid );
		//float * fp = (float *)_mat->grid;
		//	Logger->Log("Matrix Output:\n");
		//	for(int j=0;j<16;j++)
		//{
		//	Logger->Log("Mat:%d Val:%f \n",j,fp[j]);
		//}
		//Logger->Log("About to call render.\n");
		Render();
		//Logger->Log("Called.\n");
		glPopMatrix();
	//	Logger->Log("Left Cycle.\n");
	}
	void DebugBones()
	{
		Logger->Log("Bone debug.\n");
		_bone_list.start();
		while( _bone_list.next() == true )
		{
			Bone * b = _bone_list.get();
			Logger->Log("Verts:%d \n",b->no_verts);
		}
	}
	List<Surface *>_surf;
	List<Surface *>_anim_surf;
	int _surfc;
	int _pickable;
	int _anim;
	List<Bone *>_bone_list;
};

int Slerp(float Ax,float Ay,float Az,float Aw,float Bx,float By,float Bz,float Bw,float *Cx,float *Cy,float *Cz,float *Cw,float t)
{
	
	if( (fabs(Ax-Bx)<0.001) && (fabs(Ay-By)<0.001) && (fabs(Az-Bz)<0.001) && (fabs(Aw-Bw)<0.001) ) 
	{
		Cx[0]=Ax;
		Cy[0]=Ay;
		Cz[0]=Az;
		Cw[0]=Aw;
		return true;
	}
	
	float cosineom=Ax*Bx+Ay*By+Az*Bz+Aw*Bw;
	float scaler_w;
	float scaler_x;
	float scaler_y;
	float scaler_z;
	
	if( cosineom < 0.001 )
	{
		cosineom=-cosineom;
		scaler_w=-Bw;
		scaler_x=-Bx;
		scaler_y=-By;
		scaler_z=-Bz;
	}
	{
		scaler_w=Bw;
		scaler_x=Bx;
		scaler_y=By;
		scaler_z=Bz;
	}
	
	float scale0;
	float scale1;
	
	if( (1.0 - cosineom)>0.0001 )
	{
		float omega=acos(DegToRad(cosineom));
		float sineom=sin(DegToRad(omega));
		scale0=sin( DegToRad( (1.0-t )*omega ) )/sineom;
		scale1=sin( DegToRad(t*omega) )/sineom;
	}
	else
	{
		scale0=1.0-t;
		scale1=t;
	}
		
	Cw[0]=scale0*Aw+scale1*scaler_w;
	Cx[0]=scale0*Ax+scale1*scaler_x;
	Cy[0]=scale0*Ay+scale1*scaler_y;
	Cz[0]=scale0*Az+scale1*scaler_z;

}


void VertexDeform(Entity *ent1,Bone * bent,int *vm)
{
	
		int multi_surfs=false;
		Surface * surf = NULL;
		Surface *anim_surf = NULL;

		
		if( ent1->CountSurfaces()>1 )
		{
			
			multi_surfs=true;
	
		}
		else
		{
	
			surf=ent1->GetSurface(0);
			anim_surf=ent1->GetAnimSurface(0);
		}

				
			float * overt = (float *)surf->_verts;
		float * nvert = (float *)anim_surf->_verts;																																																																													
		//' get transform mat by multiplying current pose matrix with reference pose inverse matrix
		Matrix *tform_mat=bent->_mat->Copy();
		tform_mat->Multiply( bent->inv_mat );
		
		Matrix * new_mat = tform_mat->Copy();
		float mx,my,mz;
		mx = new_mat->grid[3][0];
		my = new_mat->grid[3][1];
		mz = new_mat->grid[3][2];
		//' cycle through all vertices attached to bone
		
		for( int ix =0;ix<bent->no_verts;ix++)
		{
			int vid=bent->verts_id[ix];
			int vid2 = vid *3;	//' get vertex id
		
			float weight=bent->verts_w[ix]; // ' get vertex weight
		
			float ovx=overt[ vid2 ] ;
			float ovy=overt[ vid2+1 ];
			float ovz=-overt[ vid2+2 ];
		
			new_mat->grid[3][0]= mx;
			new_mat->grid[3][1] = my;
			new_mat->grid[3][2] = mz;
				
			ovz = -ovz;
			
			float fx,fy,fz; 
			fx=(new_mat->grid[0][0]*ovx) + (new_mat->grid[1][0]*ovy) + (new_mat->grid[2][0]*ovz) + new_mat->grid[3][0]; 
			fy=(new_mat->grid[0][1]*ovx) + (new_mat->grid[1][1]*ovy) + (new_mat->grid[2][1]*ovz) + new_mat->grid[3][1]; 
			fz=(new_mat->grid[0][2]*ovx) + (new_mat->grid[1][2]*ovy) + (new_mat->grid[2][2]*ovz) + new_mat->grid[3][2];
				
			float vx;
			float vy;
			float vz;
			int vertex_moved=vm[vid]; //' a simple true/false check to see whether vertex has been moved already
			if( vertex_moved==false ) //' if vertex not moved yet, we don't add current vertex position
			{
		
				vx=0;
				vy=0;
				vz=0;
		
			}
			else
			{
				
				vx=nvert[ vid2 ];//anim_surf->VertexX(vid);
				vy=nvert[ vid2 + 1];//anim_surf->VertexY(vid);
				vz=-nvert[ vid2 + 2];//anim_surf->VertexZ(vid);
	
			}
				
							
			nvert[ vid2] =vx+(fx)*weight;
			nvert[ vid2+1]=vy+(fy)*weight;
			nvert[ vid2+2]=vz+(fz)*weight;
		
			vm[vid]=true;
				
	
			}
			delete tform_mat;
			delete new_mat;
}

void AnimateMesh(Base *ent1,float framef,int start_frame,int end_frame)
{

	if( ent1->_type == Cls_Entity )
	{
	
		Entity * entm = (Entity *)ent1;
		if( entm->_anim==false) return; //' mesh contains no anim data
		
		entm->_anim_render=true;
	
		Surface * surf = entm->GetSurface(0);

		int *vm =(int *) malloc( surf->Vertices()*4 );
		for(int i=0;i<surf->Vertices();i++)
		{
			vm[i]=0;
		}
		if( framef>end_frame ) framef=end_frame;
		if( framef<start_frame) framef=start_frame;
			
		int frame=(int)framef; // ' float to int

		entm->_bone_list.start();
		
		while( entm->_bone_list.next() == true )
		{		

			Bone *bent = entm->_bone_list.get();
			int i=0;
			int ii=0;
			float fd1=0;// ' anim time since last key
			float fd2=0;// ' anim time until next key
			int found=false;
			int no_keys=false;
			float w1;
			float x1;
			float y1;
			float z1;
			float w2;
			float x2;
			float y2;
			float z2;
			
			int flag=0;
			
			//' position
					
			//' backwards
			i=frame+1;
			//Logger->Log("Loop 1.\n");
			//Logger->Log("I:%d \n",i);
			while(1)
			{
				i=i-1;
				
				if( bent->keys->flags[i]&1 )
				{
				//	Logger->Log("Found.\n");
					//Logger->Log("Frame:%d \n",i);
					x1=bent->keys->px[i];
					y1=bent->keys->py[i];
					z1=bent->keys->pz[i];
				//	Logger->Log("X:%f ",x1);
				//	Logger->Log("Y:%f ",y1);
				//	Logger->Log("Z:%f \n",z1);
					fd1=framef-i;
					found=true;
				}
				if( i<start_frame+1 ) 
				{
					i=end_frame+1;
					ii=ii+1;
				}
				if(found == true) break;
				if( ii>1 ) break;
			}		

			if( found==false ) no_keys=true;
			found=false;
			ii=0;
			
			//Logger->Log("Loop 2 \n");
			i=frame;
			while(1)
			{
				i=i+1;
				if( i>end_frame ) 
				{
					i=start_frame;
					ii=ii+1;
				}
				flag=bent->keys->flags[i]&1;
				if( flag == true )
				{
					//Logger->Log("Found.\n");
					//	Logger->Log("Frame:%d \n",i);
					x2=bent->keys->px[i];
					y2=bent->keys->py[i];
					z2=bent->keys->pz[i];
					//Logger->Log("X:%f ",x2);
					//Logger->Log("Y:%f ",y2);
					//Logger->Log("Z:%f \n",z2);
					fd2=i-framef;
					found=true;
				}
				if( found == true ) break;
				if( ii>1 ) break;
			}
			if( found==false ) no_keys=true;
			found=false;
			ii=0;
	
			float px3=0;
			float py3=0;
			float pz3=0;
			if( no_keys==true )
			{
				//Logger->Log("No keys is true.\n");
			//		Logger->Log("Frame:%d \n",i);
					
				px3=bent->_n_px;
				py3=bent->_n_py;
				pz3=-bent->_n_pz;
				//Logger->Log("Px:%f ",px3);
				//Logger->Log("Py:%f ",py3);
				//Logger->Log("Pz:%f \n",pz3);
				
			}
			else
			{
				if( (fd1+fd2)==0.0 )// ' one keyframe
				{
			//		' if only one keyframe, fd1+fd2 will equal 0 resulting in division error and garbage positional values (which can affect children)
				//	' so we check for this, and if true then positional values equals x1,y1,z1 (same as x2,y2,z2)
					//Logger->Log("One Key frame.\n");
					px3=x1;
					py3=y1;
					pz3=z1;
				} //' more than one keyframe
				else
				{
					//Logger->Log("Two keyframes.\n");
					px3=(((x2-x1)/(fd1+fd2))*fd1)+x1;
					py3=(((y2-y1)/(fd1+fd2))*fd1)+y1;
					pz3=(((z2-z1)/(fd1+fd2))*fd1)+z1;
					//Logger->Log("Px:%f ",px3);
				//	Logger->Log("Py:%f ",py3);
				//	Logger->Log("Pz:%f \n",pz3);
					
				}
			}
			no_keys=false;
			
			//' store current keyframe for use with transtions
			bent->_kx=px3;
			bent->_ky=py3;
			bent->_kz=pz3;
			
		
			//' rotation
			//Logger->Log("Loop 3.\n");
			i=frame+1;
			while(1)
			{
			
				i=i-1;
				flag=bent->keys->flags[i]&4;
				//Logger->Log("Flag:%d \n",bent->keys->flags[i]);
				if(bent->keys->flags[i]&4)
				{

					//Logger->Log("Found.\n");
					//Logger->Log("Frame:%d \n",i);
					w1=bent->keys->qw[i];
					x1=bent->keys->qx[i];
					y1=bent->keys->qy[i];
					z1=bent->keys->qz[i];
					//Logger->Log("W:%f ",w1);
				//	Logger->Log("X:%f ",x1);
				//	Logger->Log("Y:%f ",y1);
				//	Logger->Log("Z:%f \n",z1);
					fd1=framef-i;
					found=true;
				}
				if( i<start_frame+1 ) 
				{
		
					i=end_frame+1;
					ii=ii+1;
				}
				if( found == true ) break;
				if( ii>1 ) break;
			}
			if( found==false ) no_keys=true;
			found=false;
			ii=0;
			
			
			//Logger->Log("Loop 4\n");
			i=frame;
			while(1)
			{
				i=i+1;
				//Logger->Log("Checking:%d \n",i);
				if (i>(end_frame-1)) 
				{
					//Logger->Log("II inced.\n");
					i=start_frame;
					ii=ii+1;
					//Logger->Log("II:%d \n",ii);
				}
				flag=bent->keys->flags[i]&4;
				if( bent->keys->flags[i]&4 )
				{
					//Logger->Log("Found.\n");
					//Logger->Log("Frame:%d \n",i);
					w2=bent->keys->qw[i];
					x2=bent->keys->qx[i];
					y2=bent->keys->qy[i];
					z2=bent->keys->qz[i];
					//Logger->Log("W:%f ",w2);
					//Logger->Log("X:%f ",x2);
					//Logger->Log("Y:%f ",y2);
					//Logger->Log("Z:%f \n",z2);
					
					fd2=i-framef;
					found=true;
				}
				if( found == true ) break;
				if(ii>1) break;
								
			} 
			if( found==false ) no_keys=true;
		
			found=false;
			ii=0;

	

			float w3=0;
			float x3=0;
			float y3=0;
			float z3=0;
			if( no_keys==true ) //' no keyframes
			{
				w3=bent->_n_qw;
				x3=bent->_n_qx;
				y3=bent->_n_qy;
				z3=bent->_n_qz;
				//Logger->Log("No keyframe.");
			//	Logger->Log("W3:%f ",w3);
			//	Logger->Log("X3:%f ",x3);
			//	Logger->Log("Y3:%f ",y3);
			//	Logger->Log("Z3:%f \n",z3);
				
			}
			else
			{
				if( (fd1+fd2)==0.0f )//' one keyframe
				{
					//' if only one keyframe, fd1+fd2 will equal 0 resulting in division error and garbage rotational values (which can affect children)
					//' so we check for this, and if true then rotational values equals w1,x1,y1,z1 (same as w2,x2,y2,z2)
					w3=w1;
					x3=x1;
					y3=y1;
					z3=z1;
					//Logger->Log("One keyframe \n");
				}
				else
				{ // ' more than one keyframe
					//Logger->Log("Two keyframes.\n");
					float t=(1.0/(fd1+fd2))*fd1;
					Slerp(x1,y1,z1,w1,x2,y2,z2,w2,&x3,&y3,&z3,&w3,t);// ' interpolate between prev and next rotations
					//Logger->Log("Slerp:\n");
					//Logger->Log("X:%f ",x3);
					//Logger->Log("Y:%f ",y3);
					//Logger->Log("Z:%f ",z3);
					//Logger->Log("W:%f \n",w3);
					
				}
			}
			no_keys=false;
			
			//' store current keyframe for use with transtions
			bent->_kqw=w3;
			bent->_kqx=x3;
			bent->_kqy=y3;
			bent->_kqz=z3;

		//	'Local pitch#
			
		//	'Local yaw#
		//	'Local roll#
		//	'QuatToEuler(-w3#,x3#,y3#,-z3#,pitch#,yaw#,roll#)
	
		//	'TBone(bent).rx#=pitch#
		//	'TBone(bent).ry#=yaw#
		//	'TBone(bent).rz#=roll#
	
			QuatToMat(w3,x3,y3,z3,bent->_mat);

			bent->_mat->grid[3][0]=px3;
			bent->_mat->grid[3][1]=py3;
			bent->_mat->grid[3][2]=pz3;
				
			//' copy parent's matrix and multiply by self's matrix
			if( !(bent->_parent==NULL) && !(bent->_parent==ent1) )
			{
			 	//Logger->Log("Copied and multiplied\n");
			 	Matrix *new_mat = bent->_parent->_mat->Copy();
				new_mat->Multiply(bent->_mat);
				bent->_mat->Overwrite(new_mat);
				delete new_mat;
			}
			

			//' *** vertex deform ***
			VertexDeform( (Entity *)ent1,(Bone *)bent,vm);
			
										
		}
		
		free((void *)vm);
		
	}
		
	
}



Matrix * MatInverse( Matrix *mat )
{
	Matrix * new_mat=new Matrix;

	float tx=0;
	float ty=0;
	float tz=0;
	
	new_mat->grid[0][0] = mat->grid[0][0];
  new_mat->grid[1][0] = mat->grid[0][1];
  new_mat->grid[2][0] = mat->grid[0][2];

	new_mat->grid[0][1] = mat->grid[1][0];
	new_mat->grid[1][1] = mat->grid[1][1];
	new_mat->grid[2][1] = mat->grid[1][2];

	new_mat->grid[0][2] = mat->grid[2][0];
	new_mat->grid[1][2] = mat->grid[2][1];
	new_mat->grid[2][2] = mat->grid[2][2];

	new_mat->grid[0][3] = 0;
	new_mat->grid[1][3] = 0;
	new_mat->grid[2][3] = 0;
	new_mat->grid[3][3] = 1;
	
	tx = mat->grid[3][0];
	ty = mat->grid[3][1];
	tz = mat->grid[3][2];


	new_mat->grid[3][0] = -( (mat->grid[0][0] * tx) + (mat->grid[0][1] * ty) + (mat->grid[0][2] * tz) );
	new_mat->grid[3][1] = -( (mat->grid[1][0] * tx) + (mat->grid[1][1] * ty) + (mat->grid[1][2] * tz) );
	new_mat->grid[3][2] = -( (mat->grid[2][0] * tx) + (mat->grid[2][1] * ty) + (mat->grid[2][2] * tz) );
	
	return new_mat;
	
}


class Camera : public Base
{
public:
	float _viewx,_viewy,_vwidth,_vheight;
	float _clsr,_clsg,_clsb;
	float _zDepth,_z2Depth;
	float _zoom;
	Camera()
	{
		_viewx=0;
		_viewy=0;
		_vwidth=480;
		_vheight=272;
		_clsr=0;
		_clsg=0;
		_clsb=0;
		_zDepth=0.1;
		_z2Depth = 1000;
		_zoom = 1;
	}	
	Camera(Base *parent)
	{
		_viewx=0;
		_viewy=0;
		_vwidth=480;
		_vheight=272;
		_clsr=0;
		_clsg=0;
		_clsb=0;
		_zDepth=0.1;
		_z2Depth = 1000;
		_zoom = 1;
		_parent = parent;
		
	}
void  Other(float ax,float ay,float az)
	{

		
		float mx=ax;
		float my=ay;
		float mz=-az;

		Matrix *new_mat=new Matrix;
		new_mat->LoadIdentity();
		new_mat->Rotate(_rx,_ry,_rz);
		new_mat->Translate(mx,my,mz);
	
		mx=new_mat->grid[3][0];
		my=new_mat->grid[3][1];
		mz=new_mat->grid[3][2];
		
		_px=_px+mx;
		_py=_py+my;
		_pz=_pz+mz;

		if( !(_parent==NULL) )
		{
			_mat->Overwrite(_parent->_mat);
			UpdateMat();
		}

		if( _parent==NULL )
		{
			UpdateMat(true);
		}
		
		UpdateChildren(this);
		
		delete new_mat;
		
		
	}
	void Draw3D()
	{
		SetViewPort3D();
		glShadeModel(GL_SMOOTH);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
	}
	
	void SetViewPort3D()
	{
		glViewport(_viewx,_viewy, _vwidth, _vheight);
  	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = ((float)_vwidth /(float)_vheight);
		gluPerspective(  RadToDeg( atan( (1.0/(_zoom*aspect))))*2.0,aspect,_zDepth,_z2Depth);
		glMatrixMode(GL_MODELVIEW);
	}
	
	void Viewport( int x,int y,int w,int h)
	{
		_viewx = x;
		_viewy = 272 - h-y;
		_vwidth = w;
		_vheight = h;
	}
	
	void Zoom(float zoom)
	{
		_zoom = zoom;
	}
	void ClsColor(float r,float g,float b)
	{
		_clsr = r;
		_clsg = g;
		_clsb = b;
	}
	void Range(float z1,float z2)
	{
		_zDepth = z1;
		_z2Depth = z2;
	}
	void Cycle()
	{
		glDepthMask( GL_TRUE );
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		Draw3D();
		Matrix *new_mat = MatInverse( _mat );
		glLoadMatrixf( (GLfloat *)new_mat->grid );
		delete new_mat;
	}
	
};



class EngineFactory
{
public:
	Entity * ProduceText( char * txt,Base * parent )
	{
		
		Entity *out = ProduceEntity( parent );
		float dx,dy;
		float tw;
	
		tw = FontRenderer->TextWidth( txt );

		dx = -tw/2;
		dy = -(FontRenderer->TextHeight()/2);

		int tl = FontRenderer->TextLen( txt );
		
		Font *af = FontRenderer->_active;
		Surface *surf = new Surface( tl*8,tl*4+2 );
	
		out->AddSurface( surf );
		Material *mat = new Material;
		mat->AddTexture( af->_tex );
		mat->SetBlend( Mask );
		out->SetMaterial( mat );

		int vc=0;
		for( int j=0;j<tl;j++)
		{
	
			int c = txt[j];
			int xo,yo;
			yo = (c/(int)af->_cpl);
			xo = (c-(yo *(int) af->_cpl ));
			float ax,ay;
			ax = xo * af->_cw;
			ay = yo * af->_ch;
			float ex,ey;
			ex = ax + af->_cw;
			ey = ay + af->_ch;
			
			float u0,v0,u1,v1;
			u0 = ax / af->_mw;
			v0 = 1-(ay / af->_mh);
			u1 = ex / af->_mw;
			v1 = 1-(ey / af->_mh);
			//mesh.addquad( dx,dy,activefont.cwidth,activefont.cheight,u0,v1,u1,v0 )
			
			int vt0,vt1,vt2,vt3;
			vt0=surf->AddVertex( dx,dy,0,u0,v0 );
			vt1=surf->AddVertex( dx+af->_cw,dy,0,u1,v0 );
			vt2=surf->AddVertex( dx+af->_cw,dy+af->_ch,0,u1,v1 );
			vt3=surf->AddVertex( dx,dy+af->_ch,0,u0,v1 );
		
			surf->AddTriangle( vt0,vt1,vt2 );
			surf->AddTriangle( vt2,vt3,vt0 );
			//surf->AddTriangle( vt2,vt1,vt0 );
			//surf->AddTriangle( vt0,vt3,vt2 );
	
																
			dx+=af->_xinc;
			
		}

		out->SetVisualizer( new VL_VertexArray );

		return out;
	
	}
	
	Entity * ProduceTerrain(Texture *tex,float width=100,float height=100,float subDiv=8,float maxHeight = 30,Base *parent = NULL)
	{
		Entity *out = ProduceEntity(parent);
		
		
		Material *pmat = new Material;
	
	
		pmat->AddTexture( tex );
		float xsiz,ysiz;
		xsiz = width/subDiv;
		ysiz = height/subDiv;
		
		int vgrid[(int)subDiv+1][(int)subDiv+1];
		char *buf = tex->_raw;
		
		Surface *surf = new Surface(subDiv*subDiv*3,subDiv*subDiv*6);
		out->AddSurface( surf );
		out->SetMaterial( pmat );	
		
		for( float x=0;x<subDiv+1;x++)
		{
		for( float y=0;y<subDiv+1;y++)
		{
			float vx,vy;
			vx = x *xsiz;
			vy = y *ysiz;
			float tx,ty;
			tx = vx / width;
			ty = vy / height;
			tx = tx * tex->_w;
			ty = ty * tex->_h;
			if( tx>tex->_w-1) tx = tex->_w-1;
			if( ty>tex->_h-1) ty = tex->_h-1;
			char *b = buf;
			b+=((int)ty*tex->_w*4)+(int)tx*4;
			
			float cv;
						
			cv = b[0]+b[1]+b[2];
			cv = cv / 3;
			
			float ci = cv / 255.0;
			
			float verty = maxHeight * ci; 
			
			vgrid[(int)x][(int)y] = surf->AddVertex( -width/2+vx,verty,-height/2+vy );
			float ui,vi;
			
			ui = vx / width;
			vi = vy / height;
			
			surf->SetCoords( 0,vgrid[(int)x][(int)y],ui,vi );
		
		}
		}
		
		float qx,qy;
		while(1)
		{
			surf->AddTriangle( vgrid[(int)qx][(int)qy],vgrid[(int)qx+1][(int)qy],vgrid[(int)qx+1][(int)qy+1] );
			surf->AddTriangle( vgrid[(int)qx+1][(int)qy+1],vgrid[(int)qx][(int)qy+1],vgrid[(int)qx][(int)qy] );
			qx++;
			if( qx == subDiv )
			{
				qx=0;
				qy++;
				if( qy = subDiv )
				{
					break;
				}
			}
			
		}
		out->SetVisualizer( new VL_VertexArray );
		return out;
		
	}
	
	Entity * ProduceEntity(Base *parent=NULL)
	{
		Entity *out = new Entity(parent);
		if( !(out->_parent==NULL))
		{
			out->_mat->Overwrite( out->_parent->_mat );
			out->UpdateMat();
		}
		else
		{
			out->UpdateMat(true);
		}
		out->_type = Cls_Entity;
		entlist.add( (Base *)out ); 
		return out;
	}
	
	Camera * ProduceCamera(Base *parent=NULL)
	{
		Camera *out = new Camera(parent);
		if( !(out->_parent==NULL))
		{
			out->_mat->Overwrite(out->_parent->_mat);
			out->UpdateMat();
		}else
		{
			out->UpdateMat(true);
		}
		return out;
	}
};

class RenderEngine
{
public:
	int CountEntities()
	{
		int ec=0;
		entlist.start();
		while( entlist.next() == true )
		{
			ec++;
		}
		return ec;
	}
	Entity * GetEntity(int index=0)
	{
		Entity *ent=NULL;
		entlist.start();
		int ec=0;
		while( entlist.next() == true)
		{
			if( ec == index )
			{
				ent = (Entity *)entlist.get();
			}
			ec++;
			
		}
		return ent;
		
	}
	void RenderScene()
	{
		if(_ActiveCam==NULL) return;
		_ActiveCam->Cycle();
		entlist.start();
	//	Logger->Log("Render Cycle Begin.\n");
		while( entlist.next()==true )
		{
			//glTranslatef(0,0,-40);
		//	Logger->Log("Rendering entity.\n");
			Entity *ent =(Entity *)entlist.get();
			ent->Cycle();
	//		Logger->Log("Rendered.\n");
			
		}	
		
	}
	Camera *_ActiveCam;
};

class StringEngine
{
public:
	char * Num( char *str,int num )
	{
		char * buf = (char *)malloc(100);
		sprintf(buf,"%i",num);
		buf = strcat( str,buf );
		return buf;
	}
	char * Num( int num )
	{
		char * buf = (char *)malloc(100);
		sprintf(buf,"%i \n",num);
		return buf;
	}
	char * Num( u64 num )
	{
		char * buf = (char *)malloc(100);
		sprintf(buf,"%i \n",num);
		return buf;
	}
	char * Num( float num )
	{
		char *buf = (char *)malloc(100);
		sprintf(buf,"%f \n",num );
		return buf;
	}
	char * Append( char *s1,char *s2 )
	{
		int sl=0;
		sl = Len( s1 ) + Len( s2 ) + 1;
		int c=0;
		char * n =(char *) malloc( sl );
		while(1)
		{
			if( s1[c] == 0 ) break;
			n[c] = s1[c];
			c++;
		}
		int nc=0;
		while(1)
		{
			if( s2[nc] == 0 ) break;
			n[c] = s2[nc];
			c++;
			nc++;
		}
		return n;
	}
	char * ToLower( char *str )
	{
		char *buf = Mid( str,0,-1 );
		char *ptr;
		for(ptr=buf;*ptr;ptr++)
   	{
       *ptr=tolower(*ptr);
  	}
  	return buf;
	}
	char * FixPath( char *str )
	{
		char * buf = (char *)malloc(1);
		buf[0] = 92;
		char * ret = Replace( str,buf,"/" );
		free( (void *)buf );
		return ret;
	}
	char * Replace( char *str,char *s1,char *s2 )
	{
		char *buf = (char *)malloc( Len(str)+1 );
		int cc=0;
		while(1)
		{
			char c = str[cc];
		//	if( c == s1[0] ) c = s2[0];
		  buf[cc] = c;
		  if( c == 0 ) break;
			cc++;
		}
		return buf;
	}
	
	char * StripDirForward( char *str )
	{
		char *fixedpath = FixPath( str );
		int i  = FindLast( fixedpath,"/" );
		if( i == -1 ) return str;
		char *ret = Mid( fixedpath,i+1,-1 );
		free( (void *)fixedpath );
		return ret;
	}
	
	char * ExtractType( char * str )
	{
		int i = FindLast( str,"." );
		if( i == - 1 ) return str;
		char *ret = ToLower( Mid( str,i+1,-1 ) );
		return ret;	
	}	
	
	char * Mid(char *str,int from,int len = 1 )
	{
		if( len == - 1 )
		{
			len = Len( str ) - from;
		}
		char *buf = (char *)malloc( len + 1 );
		int bc=0;
		while(1)
		{
			buf[bc] = str[ from ];
			if( buf[bc] == 0 ) 
			{
				break;
			}
			from++;
			bc++;
			if( bc == len ) break;
			
		}
		buf[bc] = 0;
		return buf;
		
	}
	int Find(char *str,char *c,int startfrom = 0)
	{
		int len = Len( str );
		while(1)
		{
			if( str[startfrom] == c[0] )
			{
				return startfrom;
			}			
			startfrom++;
			if( startfrom == len ) break;
		}
		return -1;
	}
	int FindLast( char *str,char *c )
	{
		int lc=-1;
		int nc=-1;
		while(1)
		{
			lc = nc;
			nc = Find( str,c,nc+1 );
			if( nc == -1 )
			{
				return lc;
			}
		}
	}
	int Len( char * str )
	{
		int c = 0;
		while(1)
		{
			if( str[c] == 0 )
				break;
			c++;
		}
		return c;
	}
	
};
class PrefabFactory;

RenderEngine *Renderer;

EngineFactory *Factory;
StringEngine * StringUtil;
PrefabFactory * Prefab;



void CloseRaptor()
{
	Profile->LogProfile();
	delete Profile;
	delete Logger;
	delete StringUtil;
	delete FontRenderer;
	delete Renderer;
	delete Factory;
}



class PrefabFactory
{
public:
	int AddQuad(Surface *surf,float w,float h)
	{
		int v0,v1,v2,v3;
		float w2,h2;
		w2 = w/2;
		h2 = h/2;
		v0 = surf->AddVertex( -w2,-h2,0,0,0 );
		v1 = surf->AddVertex( w2,-h2,0,1,0 );
		v2 = surf->AddVertex( w2,h2,0,1,1 );
		v3 = surf->AddVertex( -w2,h2,0,0,1 );
		surf->VertexColor( v0,0.5,0.5,0.5,0.5 );
		surf->VertexColor( v1,0.5,0.5,0.5,0.5 );
		surf->VertexColor( v2,0.5,0.5,0.5,0.5 );
		surf->VertexColor( v3,0.5,0.5,0.5,0.5 );
		
		surf->AddTriangle( v0,v1,v2 );
		surf->AddTriangle( v2,v3,v0 );
		
		//surf->AddTriangle( v2,v1,v0 );
		//surf->AddTriangle( v0,v3,v2 );
		
		return v0;
	}
	Entity * CreateQuad(float w,float h,Base *parent = NULL,int doublesided = true)
	{
		Entity *out = Factory->ProduceEntity(parent);
		int tric = 2;
		if( doublesided == true ) tric = 4;
		Surface *surf = new Surface(4,tric);
		out->AddSurface( surf );
		int v0,v1,v2,v3;
		
		int mw,mh;
		mw = w/2;
		mh = h/2;
		
		v0 = surf->AddVertex( -mw,-mh,0,0,0 );
		v1 = surf->AddVertex( mw,-mh,0,1,0 );
		v2 = surf->AddVertex( mw,mh,0,1,1 );
		v3 = surf->AddVertex( -mw,mh,0,0,1 );
		
		surf->AddTriangle( v0,v1,v2 );
		surf->AddTriangle( v2,v3,v0 );
		
		if(doublesided == true )
		{
			surf->AddTriangle( v2,v1,v0 );
			surf->AddTriangle( v0,v3,v2 );
		}
		
		return out;
		
	}
	
	Entity * CreateCube(float size,Base *parent = NULL)
	{
		Entity *out = Factory->ProduceEntity(parent);
		Surface *surf = new Surface(8,12);
		size = size / 2.0;
		int v0,v1,v2,v3,v4,v5,v6,v7,v8;
		
		v0 = surf->AddVertex( -size,-size,-size,0,0);
		v1 = surf->AddVertex( size,-size,-size,1,0 );
		v2 = surf->AddVertex( size,size,-size,1,1 );
		v3 = surf->AddVertex( -size,size,-size,0,1 );
		
		v4 = surf->AddVertex( -size,-size,size,1,0);
		v5 = surf->AddVertex( size,-size,size,0,0);
		v6 = surf->AddVertex( size,size,size,0,1);
		v7 = surf->AddVertex( -size,size,size,1,1);
		
		surf->AddTriangle( v0,v1,v2 );
		surf->AddTriangle( v2,v3,v0 );
		
		surf->AddTriangle( v4,v5,v6 );
		surf->AddTriangle( v6,v7,v4 );
		
		surf->AddTriangle( v0,v4,v3 );
		surf->AddTriangle( v4,v7,v3 );
		
		surf->AddTriangle( v1,v5,v2 );
		surf->AddTriangle( v5,v6,v2 );
		
		surf->AddTriangle( v0,v1,v4 );
		surf->AddTriangle( v1,v5,v4 );
		
		surf->AddTriangle( v3,v2,v7 );
		surf->AddTriangle( v2,v6,v7 );
		
		out->AddSurface( surf );
		//entlist.add( (Base *)out ); 		
		return out;
		
	}
};

void InitRaptor()
{
	Factory = new EngineFactory;
	Renderer = new RenderEngine;
	FontRenderer = new FontEngine;
	StringUtil = new StringEngine;
	Logger = new LogEngine("RaptorSystemLog.txt","System");
	Profile = new Profiler;
	Prefab = new PrefabFactory;
}

void QuatToEuler(float w,float x,float y,float z,float *pitch,float *yaw ,float *roll)
{
	float q[4];
	q[0]=w;
	q[1]=x;
	q[2]=y;
	q[3]=z;
	
	float xx=q[1]*q[1];
	float yy=q[2]*q[2];
	float zz=q[3]*q[3];
	float xy=q[1]*q[2];
	float xz=q[1]*q[3];
	float yz=q[2]*q[3];
	float wx=q[0]*q[1];
	float wy=q[0]*q[2];
	float wz=q[0]*q[3];

	Matrix *mat=new Matrix;
	
	mat->grid[0][0]=1-2*(yy+zz);
	mat->grid[0][1]=  2*(xy-wz);
	mat->grid[0][2]=  2*(xz+wy);
	mat->grid[1][0]=  2*(xy+wz);
	mat->grid[1][1]=1-2*(xx+zz);
	mat->grid[1][2]=  2*(yz-wx);
	mat->grid[2][0]=  2*(xz-wy);
	mat->grid[2][1]=  2*(yz+wx);
	mat->grid[2][2]=1-2*(xx+yy);
	mat->grid[3][3]=1;

	for( int iy=0;iy<4;iy++)
	{
		for( int ix=0;ix<4;ix++)
		{
			xx=mat->grid[ix][iy];
			if( fabs(xx)<0.0001) xx = 0;
			mat->grid[ix][iy]=xx;
			
		}
	}

	pitch[0]=atan2( mat->grid[2][1],sqrt( mat->grid[2][0]*mat->grid[2][0]+mat->grid[2][2]*mat->grid[2][2] ) );
	yaw[0]=atan2(mat->grid[2][0],mat->grid[2][2]);
	roll[0]=atan2(mat->grid[0][1],mat->grid[1][1]);
				
			
};

class Particle
{
public:
	Particle( float size )
	{
		_x=0;
		_y=0;
		_z=0;
		_xi=0;
		_yi=0;
		_zi=0;
		_alpha = 1;
		_grav=0;
		_tv=-6;
		int s2 = size/2;
		_vx[0] = -s2;
		_vx[1] = s2;
		_vx[2] = s2;
		_vx[3] = -s2;
		
		_vy[0] = -s2;
		_vy[1] = -s2;
		_vy[2] = s2;
		_vy[3] = s2;
		
	}
	
	int Update()
	{
		_alpha-=0.1;
	//	_x+=_xi;
	//		_y+=_yi;
	//		_z+=_zi;
		if( !(_grav==0))
		{
			if( _yi>_tv )
			{
				_yi-=_grav;
			}
		}
		if( _alpha < 0 )
		{
			return false;
		}
		return false;
	}
	float _grav,_tv;
	float _x,_y,_z;
	float _xi,_yi,_zi;
	float _index;
	float _alpha;
	float _vx[4];
	float _vy[4];
};

class ParticleEngine
{
public:
	ParticleEngine(Texture *tex,int MaxParticles = 100 )
	{
		Logger->Log("In pe.\n");
		_gravity = 0;
		_tex = tex;
		_vis = Factory->ProduceEntity();
		_osurf = new Surface( MaxParticles * 4,MaxParticles * 4 );
		_nsurf = new Surface( MaxParticles * 4,MaxParticles * 4 );
		_vis->AddSurface( _nsurf );
		Material *mat = new Material;
		mat->SetBlend( Alpha );
		_vis->SetMaterial( mat );
		if( !(tex == NULL) )
		{
			mat->AddTexture( tex );
		}
		_vis->SetVisualizer( new VL_VertexArray );
		_piv = new Entity(NULL);
		Logger->Log("Out pe.\n");
	}

	
	Particle * AddParticle( float size )
	{
		//int index = Prefab->AddQuad( _osurf,size,size );
		//Prefab->AddQuad( _nsurf,size,size );
		Particle * par = new Particle( size );
		_pool.add( par );
		return par;
	}
	Particle * Emit( float x,float y,float z,float xi,float yi,float zi,float size = 5)
	{
		Particle *par = AddParticle( size );
		if( (xi==0) && (zi == 0) )
		{
			xi=1;
		}
		par->_x = x;
		par->_y = y;
		par->_z = z;
		par->_xi = xi;
		par->_yi = yi;
		par->_zi = zi;
		par->_grav = _gravity;
		return par;
	}
	void Update()
	{
		_vis->SyncVisualizer();
		_pool.start();
		Matrix *mat = _piv->_mat;
		_nsurf->_vertc=0;
		_nsurf->_tric=0;
		float *uv = _nsurf->_coords[0]->_uv;
		float u[4];
		float v[4];
		u[0] = 0;
		u[1] = 1;
		u[2] = 1;
		u[3] = 0;
		v[0] = 0;
		v[1] = 0;
		v[2] = 1;
		v[3] = 1;
		float *verts = _nsurf->_verts;
		int *tris = _nsurf->_tris;
		int * cols = _nsurf->_cols;
		int pn=0;
		while( _pool.next() == true )
		{
			pn++;
			Particle *p = _pool.get();
			
			if( p->Update()==true )
			{
				_pool.remove();
				continue;
			}
						
			float x,y,z,i;
			x = p->_x;
			y = p->_y;
			z = p->_z;
			i = p->_index;
			_piv->Position( x,y,z );
			_piv->PointAt( Renderer->_ActiveCam );
			int vc = _nsurf->_vertc;
			int tc = _nsurf->_tric;
		  int vert[4];
			for(int j=0;j<4;j++)
			{
				float px = p->_vx[j];
				float py = p->_vy[j];
			
				float fx,fy,fz; 
				fx=(mat->grid[0][0]*px) + (mat->grid[1][0]*py) +  mat->grid[3][0]; 
				fy=(mat->grid[0][1]*px) + (mat->grid[1][1]*py) +  mat->grid[3][1]; 
				fz=(mat->grid[0][2]*px) + (mat->grid[1][2]*py) +  mat->grid[3][2];
								
				//vert[j] = _nsurf->AddVertex( fx,fy,fz,u[j],v[j] );
				verts[ vc * 3 ] = fx;
				verts[ vc * 3 + 1] = fy;
				verts[ vc * 3 + 2] = fz;
				uv[ vc * 3 ] = u[j];
				uv[ vc * 3 + 1] = v[j];
				
				vert[j] = vc;
				
				cols[ vc ] = (((int)(p->_alpha*255.f)) << 24) +  (((int)(255)) << 16) +  (((int)(255)) << 8) +  ((int)(255));
				//_nsurf->VertexColor( vert[j],1,1,1,p->_alpha );
				
					vc++;
				_nsurf->_vertc++;
				
				
					
			}

			tris[ tc * 3 ] = vert[0];
			tris[ tc * 3 + 1 ] = vert[1];
			tris[ tc * 3 + 2 ] = vert[2];
			tc++;
			tris[ tc * 3 ] = vert[2];
			tris[ tc * 3 + 1 ] = vert[3];
			tris[ tc * 3 + 2 ] = vert[0];
			tc++;
			_nsurf->_tric += 2;
		}
		//Logger->Log("Particles:%d \n",pn);
	}
	void SetGravity( float grav )
	{
		_gravity = grav;
	}
	
	List<Particle *>_pool;
	Texture *_tex;
	Entity *_vis;
	Surface *_osurf;
	Surface *_nsurf;
	Entity * _piv;
	float _gravity;
};


int FileSize( FILE *file )
{
	fseek(file, 0, SEEK_END);
	int ret = ftell(file);
	fseek(file,0,SEEK_SET);
	return ret;
}

inline char ReadByte(FILE *file)
{
	char ret;
	fread( &ret,1,1,file );
	return ret;
}

inline char * ReadString( FILE *file )
{
	char * buf = (char *)malloc(255);
	int c=0,i=0;
	while(true)
	{
		c = ReadByte( file );
	 if( c==92 ) c = 47;
			
		buf[ i ] = c;
		if( c == 0 ) break;
		i++;
	}
	return buf;
}


inline int ReadInt( FILE *file )
{
	int ret ;
	fread( &ret,4,1,file );
	return ret;
}

inline long ReadLong( FILE *file )
{
	long ret ;
	fread( &ret,sizeof(long),1,file );
	return ret;
}

inline float ReadFloat( FILE *file )
{
	float ret;
	fread( &ret,4,1,file );
	return ret;
}


inline char * ReadTag(FILE *file)
{
	int fp = ftell(file);
	char * buf =(char *) malloc( 5 );
	fread(buf, 1, 4, file);
	buf[4] = 0;
	fseek(file,fp,SEEK_SET);
	return buf;
}

inline int NewTag(char *tag)
{
	if( strcmp("TEXS",tag) == 0 ) return true;
	if( strcmp("BRUS",tag) == 0 ) return true;
	if( strcmp("NODE",tag) == 0 ) return true;
	if( strcmp("ANIM",tag) == 0 ) return true;
	if( strcmp("MESH",tag) == 0 ) return true;
	if( strcmp("VRTS",tag) == 0 ) return true;
	if( strcmp("TRIS",tag) == 0 ) return true;
	if( strcmp("BONE",tag) == 0 ) return true;
	if( strcmp("KEYS",tag) == 0 ) return true;
	return false;
}


void QuatToMat(float w,float x,float y,float z,Matrix *mat)
{
	float q[4];
	q[0]=w;
	q[1]=x;
	q[2]=y;
	q[3]=z;
	
	float xx=q[1]*q[1];
	float yy=q[2]*q[2];
	float zz=q[3]*q[3];
	float xy=q[1]*q[2];
	float xz=q[1]*q[3];
	float yz=q[2]*q[3];
	float wx=q[0]*q[1];
	float wy=q[0]*q[2];
	float wz=q[0]*q[3];

	mat->grid[0][0]=1-2*(yy+zz);
	mat->grid[0][1]=  2*(xy-wz);
	mat->grid[0][2]=  2*(xz+wy);
	mat->grid[1][0]=  2*(xy+wz);
	mat->grid[1][1]=1-2*(xx+zz);
	mat->grid[1][2]=  2*(yz-wx);
	mat->grid[2][0]=  2*(xz-wy);
	mat->grid[2][1]=  2*(yz+wx);
	mat->grid[2][2]=1-2*(xx+yy);
	mat->grid[3][3]=1;
	
	for( int iy=0;iy<4;iy++)
	{
		for(int ix=0;ix<4;ix++)
		{
			xx=mat->grid[ix][iy];
			
			if(fabs(xx)<0.0001) xx=0;
			mat->grid[ix][iy]=xx;
		}
	}

}

void CopyArray( int *from,int *to,int elements)
{
	for(int i=0;i<elements;i++)
	{
		to[i] = from[i];
	}
}

void CopyArray( float *from,float *to,int elements)
{
	for(int i=0;i<elements;i++)
	{
		to[i] = from[i];
	}
}


void Dumpent(Base *ent,int lev=0)
{
	Logger->Log("Entity lev;%d \n",lev);
	Logger->Log("Name:%s \n",ent->_name);
	
	int ec = ent->CountChildren();
	Logger->Log("Child Count:%d \n",ec);
	for(int i=0;i<ec;i++)
	{
		Base *c = (Base *)ent->GetChild(i);
		Logger->Log("Child %d \n",i);
		Dumpent( c,lev+1 );	
	}
};


inline Entity * LoadB3D(const char *filename,Base * parent_ent_ext)
{
	
	
	FILE *file;
	Logger->Log("Loading b3d %s \n",filename);
	file = fopen(filename, "rb");
	printf("opened file.\n");
	int fileSize = FileSize( file );
	printf("Checked filesize");
	char *tag;
	char *prev_tag;
	char *new_tag;
	tag = ReadTag(file);
	ReadInt(file);
	ReadLong(file);
	int vno = ReadInt(file);
	Logger->Log("Tag=%s \n",tag);
	Logger->Log(tag);
	if(!(strcmp(tag,"BB3D")==0))
	{
		Logger->Log("Tag %s not b3d",tag );
		printf("Tag %s does not match\n",tag);
		sceKernelExitGame();
	}	
	
	if( (vno/100)>0 ) 
	{
		Logger->Log("Vno doesn't match %d \n",vno);
		while(1)
		{
		
		}
		return false;
	}
	int size;
	int node_level=-1;
	int old_node_level=-1;
	int node_pos[100];
	for(int i=0;i<100;i++)
	{
		node_pos[i] = 0;
	}
	// tex local vars
	Texture * tex[255];
	char * te_file;
	int te_flags;
	int te_blend;
	int te_coords;
	float te_u_pos;
	float te_v_pos;
	float te_u_scale;
	float te_v_scale;
	float te_angle;
	
	
	//' brush local vars
	//Local brush:TBrush[1]
	Material * mat[255];
	int b_no_texs;
	Bone *bo_bone = NULL;	
	char *b_name;
	float b_red;
	float b_green;
	float b_blue;
	float b_alpha;
	float b_shine;
	int b_blend;
	int b_fx;
	int b_tex_id;
	
	
	
	
	char * n_name;
	float n_px=0;
	float n_py=0;
	float n_pz=0;
	float n_sx=0;
	float n_sy=0;
	float n_sz=0;
	float n_rx=0;
	float n_ry=0;
	float n_rz=0;
	float n_qw=0;
	float n_qx=0;
	float n_qy=0;
	float n_qz=0;
	
//	' mesh local vars
	Entity * mesh = NULL;
	int m_brush_id;

	//' verts local vars
	Entity * v_mesh = NULL;
	Surface * v_surf = NULL;
	int v_flags;
	int v_tc_sets;
	int v_tc_size;
	
	int v_sz;
	float v_x;
	float v_y;
	float v_z;
	float v_nx;
	float v_ny;
	float v_nz;
	float v_r;
	float v_g;
	float v_b;
	float v_u;
	float v_v;
	float v_w;
	float v_a;	
	int v_id;
	
	//' tris local vars
	Surface *surf = NULL;
	
	int tr_brush_id;
	int tr_sz;
	int tr_vid;
	int tr_vid0;
	int tr_vid1;
	int tr_vid2;
	float tr_x;
	float tr_y;
	float tr_z;
	float tr_nx;
	float tr_ny;
	float tr_nz;
	float tr_r;
	float tr_g;
	float tr_b;
	float tr_u;
	float tr_v;
	float tr_w;
	float tr_a;	
	int tr_no;
	
	//' anim local vars
	int a_flags;
	int a_frames;
	int a_fps;
					

	// bo_bone:TBone
//	Local bo_vert_id
//	Local bo_vert_w#
	
	//' key local vars	
	int k_flags;
	int k_frame;
	float k_px;
	float k_py;
	float k_pz;
	float k_sx;
	float k_sy;
	float k_sz;
	float k_qw;
	float k_qx;
	float k_qy;
	float k_qz;
	int bo_vert_id;
	float bo_vert_w;
					
	Base * parent_ent = NULL;// ' parent_ent - used to keep track of parent entitys within model, separate to parent_ent_ext paramater which is external to model
	Base * root_ent=NULL;
		
	int read = false;
	Base * last_ent=NULL; //' last created entity, used for assigning parent ent in node code
	//printf("Entering Main Loop\m");
	
	while( ftell( file )<fileSize )
	{
		//printf("Reading new tag\n");
		new_tag = ReadTag(file);
	//	Logger->Log("New Tag:%s \n",new_tag);
	// printf("Read new Tag:%s \n",new_tag);
		read = false;
		if( NewTag(new_tag)==true )
		{
			//Logger->Log("In new tag.\n");
			//printf("In new tag if.\n");
			prev_tag = tag;
			tag = new_tag;
			ReadInt(file);
			size = ReadInt(file);
			old_node_level = node_level;
			if( strcmp(tag,"NODE")==0 )
			{
			//	Logger->Log("Enter pre flight node.\n");
				
			//	printf("In strcmp Node construct.\n");
				node_level++;
				if(node_level>0)
				{
					//Logger->Log("Node bigger than zero.\n");	
					
					int fd = 90;
					
					while(!(fd<8))
					{
						
						fd = ftell(file)-node_pos[node_level-1];
					//	Logger->Log("Fd:%d \n",fd);
				//		Logger->Log("Node Level:%d \n",node_level);
				//		Logger->Log("Ftell:%d \n",(int)ftell(file) );
				//		Logger->Log("Np:%d \n",node_pos[node_level-1]);
						if( fd>7 )
						{
							node_level = node_level - 1;
						}
						
					}
										
				}
				
				node_pos[ node_level ] = ftell( file )+size;
				
			}
			//Logger->Log("S 2.\n");
			
			if( node_level > old_node_level )
			{
			//	Logger->Log("In node level>Old node \n");
				if( node_level> 0 )
				{
					parent_ent = last_ent;
				}
				else
				{
					parent_ent = NULL;
				}
			//	Logger->Log("Out.\n");
			}
			
			if( node_level < old_node_level )
			{
			
			//	Logger->Log("In node level < Old node level.\n");
				Base * tent = root_ent;
				
				if( node_level > 1 )
				{
					int cc;
					for(int levs=1;levs<node_level-1;levs++)
					{
						cc = tent->CountChildren();
						//Logger->Log("cc:%d \n",cc);
						tent = tent->GetChild( cc-1 );
						if( tent == NULL )
						{
							//Logger->Log("Tent is null.\n");
						}
						else
						{
							//Logger->Log("Tent is active.\n");
						}	
					}
					
					cc = tent->CountChildren();
					//Logger->Log("F CC:%d \n",cc);
					tent = tent->GetChild(cc-1);
					if( tent == NULL )
						{
							//Logger->Log("Tent is null.\n");
						}
						else
						{
						//	Logger->Log("Tent is active.\n");
						}	
					parent_ent = tent;
				//	Logger->Log("Out.\n");
				}
				
				if( node_level == 1 ) parent_ent = root_ent;
				if( node_level == 0 ) parent_ent = NULL;
				
			}
		}
		else
		{
			//printf("Changed %s \n",tag );
			tag = "";
		}
	//	printf("Entering Checks\n");
		read = false;
		//printf("TAG: %s \n",tag );
		//Logger->Log("Tag:%s \n",tag);
		if( strcmp(tag,"TEXS") == 0 )
		{
			//Logger->Log("Entered Tex.\n");
			//printf("Entered Tex check \n");
			int tex_no=0;
			new_tag=ReadTag(file);
	
			read = true;
				while( !(NewTag(new_tag)==true)  && (ftell(file)<fileSize) )
				{
				//	printf("Entered texture while loop.\n");
					te_file=ReadString(file);
				//	printf("Read String.\\n");
					te_flags=ReadInt(file);
					te_blend=ReadInt(file);
					te_u_pos=ReadFloat(file);
					te_v_pos=ReadFloat(file);
					te_u_scale=ReadFloat(file);
					te_v_scale=ReadFloat(file);
					te_angle=ReadFloat(file);
					//printf("blend:%d",te_blend);
				//	printf("Read texture\n");
				
					if( te_flags&65536 )
					{
						te_coords=1;
					}
					else
					{
						te_coords=0;
					}
									
					Texture *t;
				
					//printf(te_file );	
					
					char *new_file = StringUtil->StripDirForward( te_file );
				//	Logger->Log("Tex File:%s \n",new_file);
					char *ftyp = StringUtil->ExtractType( new_file );
						
					if( strcmp( ftyp,"bmp" ) == 0 )
					{
						t = new Texture( new_file,BMP );
					}
					if( strcmp( ftyp,"jpg" ) == 0 )
					{
						t = new Texture( new_file,BMP );
					}	
					if( strcmp( ftyp,"tif" ) == 0 )
					{
						t = new Texture( new_file,BMP );
					}
					//t = new Texture(256,256);
					//Logger->Log("Loaded.\n");
					tex[tex_no] = t;
													
					tex_no=tex_no+1;
					t->_coordset = te_coords;
					new_tag=ReadTag(file);
				
				}
		//	Logger->Log("Exited Tex.\n");
		}
		if( strcmp( tag,"BRUS" ) == 0 )
		{
			Logger->Log("Entered Brush.\n");
			//printf("Entered Brus check.\n");
			read = true;
			int brush_no = 0;
			b_no_texs=ReadInt(file);

			new_tag = ReadTag( file );
			
			while(  !(NewTag(new_tag)==true) && (ftell(file)<fileSize) )
			{
					//printf(" In loop.\n");
					b_name=ReadString(file);
					//printf("Name:%s \n",b_name);
					b_red=ReadFloat(file);
					b_green=ReadFloat(file);
					b_blue=ReadFloat(file);
					b_alpha=ReadFloat(file);
					b_shine=ReadFloat(file);
					b_blend=ReadInt(file);
					b_fx=ReadInt(file);
					mat[brush_no] = new Material;
					
					//mat[brush_no].diffuse( b_red*255,b_green*255,b_blue*255 )
					//mat[brush_no].color( b_red*255,b_green*255,b_blue*255,b_alpha )
					//mat[brush_no].shininess( b_shine )
					/*
					Select b_blend
						Case 1
							mat[brush_no].blend( blend_normal )
						Case 2
							mat[brush_no].blend( blend_mod )
						Case 3
							mat[brush_no].blend( blend_add )
						Default
							mat[brush_no].blend( blend_normal )
					End Select
					
					'brush[brush_no].fx=b_fx
					*/
					
					for( int ix=0;ix<b_no_texs;ix++)
					{
					
						b_tex_id=ReadInt(file);
						
						if( b_tex_id>-1 )
						{
							
							mat[brush_no]->AddTexture( tex[b_tex_id] );
						}
						else
						{
							mat[brush_no]->AddTexture( NULL );
						}
		
					}
	
					brush_no=brush_no+1;
					new_tag=ReadTag(file);
				
			}
			//printf("End loop \n");
			//Logger->Log("Left Brush\n");
		}
		if( strcmp( tag,"NODE" ) == 0 )
		{
//			Logger->Log("Entered Node.\n");
				//printf("Entered Node check\n");
				read = true;
				new_tag=ReadTag(file);
				n_name=ReadString(file);
				n_px=ReadFloat(file);
				n_py=ReadFloat(file);
				n_pz=(ReadFloat(file))*-1;
				n_sx=ReadFloat(file);
				n_sy=ReadFloat(file);
				n_sz=ReadFloat(file);
				n_qw=ReadFloat(file);
				n_qx=ReadFloat(file);
				n_qy=ReadFloat(file);
				n_qz=ReadFloat(file);
				float pitch=0;
				float yaw=0;
				float roll=0;
				QuatToEuler(-n_qw,n_qx,n_qy,-n_qz,&pitch,&yaw,&roll);
				n_rx=-pitch;
				n_ry=yaw;
				n_rz=roll;

				new_tag=ReadTag(file);
								
				if(  ( strcmp( new_tag,"NODE") == 0) || (strcmp(new_tag,"ANIM") == 0) )
				{
	
					//' make 'piv' entity a mesh, not a pivot, as B3D does
					Entity *piv=Factory->ProduceEntity();
				//	Logger->Log("Creating number 1.\n");
					char * nam = StringUtil->Num( node_level );
					piv->_name = nam;
				//	Logger->Log("Created.\n");
					
				//	piv.name$=n_name$ ; 'DebugLog n_name$
					piv->_px=n_px;
					piv->_py=n_py;
					piv->_pz=n_pz;
					piv->_sx=n_sx;
					piv->_sy=n_sy;
					piv->_sz=n_sz;
					piv->_rx=n_rx;
					piv->_ry=n_ry;
					piv->_rz=n_rz;
					piv->_qw=n_qw;
					piv->_qx=n_qx;
					piv->_qy=n_qy;
					piv->_qz=n_qz;
							
					//'piv.UpdateMat(True)
					//piv.EntityListAdd(TEntity.entity_list)
					
					last_ent=piv;
		
					//' root ent?
					
					if( root_ent==NULL)
					{
						root_ent=piv;
					//	Logger->Log("Set Root ent.\n");
					}
				//	' if ent is root ent, and external parent specified, add parent
					if( root_ent==piv) piv->AddParent(parent_ent_ext);
							
					//' if ent nested then add parent
					if( node_level>0 ) 
					{
						//Logger->Log("Calling addparent.\n");
						piv->AddParent(parent_ent);
					}
					QuatToMat(-n_qw,n_qx,n_qy,-n_qz,piv->_mat);
					
									
					piv->_mat->grid[3][0]=n_px;
					piv->_mat->grid[3][1]=n_py;
					piv->_mat->grid[3][2]=n_pz;
						
					if( !(piv->_parent == NULL ))
					{
						
						Matrix * new_mat=piv->_parent->_mat->Copy();
						new_mat->Multiply(piv->_mat);
						piv->_mat->Overwrite(new_mat);//'.Multiply(mat)
					}				
			
				}
		
			//Logger->Log("Left Node.\n");
		}
		if( strcmp( tag,"MESH" ) == 0 )
		{
			//Logger->Log("Entered mesh.\n");
				//printf("Entered mesh check\n");
				read = true;
				m_brush_id=ReadInt(file);
				
				mesh=Factory->ProduceEntity();
				//Logger->Log("Creating number 2.\n");
				char *nam = StringUtil->Num( node_level );
				mesh->_name = nam;
				//Logger->Log("Created.\n");
				
				mesh->_px=n_px;
				mesh->_py=n_py;
				mesh->_pz=n_pz;
				mesh->_sx=n_sx;
				mesh->_sy=n_sy;
				mesh->_sz=n_sz;
				mesh->_rx=n_rx;
				mesh->_ry=n_ry;
				mesh->_rz=n_rz;
				mesh->_qw=n_qw;
				mesh->_qx=n_qx;
				mesh->_qy=n_qy;
				mesh->_qz=n_qz;
				
				last_ent=mesh;
				
				if( root_ent==NULL )
				{
					root_ent=mesh;
					//Logger->Log("Set Root ent.\n");
				}
				//' if ent is root ent, and external parent specified, add parent
				if( root_ent==mesh ) 
				{
					//Logger->Log("Calling addparent.\n");
					mesh->AddParent(parent_ent_ext);
				}
				
				//' if ent nested then add parent
				
				if( node_level>0 ) 
				{
				//	Logger->Log("Calling addparent.\n");
					mesh->AddParent(parent_ent);
				}
				QuatToMat(-n_qw,n_qx,n_qy,-n_qz,mesh->_mat);
								
								
				mesh->_mat->grid[3][0]=n_px;
				mesh->_mat->grid[3][1]=n_py;
				mesh->_mat->grid[3][2]=n_pz;
							
				if( !(mesh->_parent==NULL) )
				{
					Matrix * new_mat=mesh->_parent->_mat->Copy();
					new_mat->Multiply(mesh->_mat);
					mesh->_mat->Overwrite(new_mat);//'.Multiply(mat)
				
				}	
				if( mesh == NULL )
				{
					//printf("Mesh set to nothing \n");
					
				}else
				{
				//	printf("Mesh set correctly.\n");
				}
			//Logger->Log("Left Mesh.\n");
		}
		if( strcmp( tag,"VRTS" )==0 )
		{
			//Logger->Log("Entered Vrts.\n");
			//printf("Verts check\n");
			read = true;
			if( !(v_mesh == NULL )) v_mesh = NULL;
			if( !(v_surf == NULL )) v_surf = NULL;
			
			//v_mesh = Factory->ProduceEntity();
			v_surf = new Surface(5000,5000);
			//v_mesh->AddSurface( v_surf );
			v_flags=ReadInt(file);
			v_tc_sets=ReadInt(file);
			v_tc_size=ReadInt(file);
				
			v_sz=12+v_tc_sets*v_tc_size*4;
			if( v_flags & 1 ) v_sz=v_sz+12;
			if( v_flags & 2 ) v_sz=v_sz+16;

			new_tag=ReadTag(file);
			//printf("New tag:%s",new_tag);
			int vertc=0;
			
			while( !(NewTag(new_tag)==true) && (ftell(file)<fileSize) )
			{
			
					v_x=ReadFloat(file);
					v_y=ReadFloat(file);
					v_z=ReadFloat(file);
					vertc++;
					if(v_flags&1)
					{
						v_nx=ReadFloat(file);
						v_ny=ReadFloat(file);
						v_nz=ReadFloat(file);
					}
										
					if(v_flags&2)
					{
						v_r=ReadFloat(file)*255.0;//' *255 as VertexColor requires 0-255 values
						v_g=ReadFloat(file)*255.0;
						v_b=ReadFloat(file)*255.0;
						v_a=ReadFloat(file);
					}
					v_r=255;
					v_g=255;
					v_b=255;
					v_a = 255;
					
					v_id=v_surf->AddVertex(v_x,v_y,v_z);
				//	v_surf.VertexColor(v_id,v_r,v_g,v_b,v_a)
				//	v_surf.VertexNormal(v_id,v_nx,v_ny,v_nz)
					
					//'read tex coords...
					for( int j=0;j<v_tc_sets;j++)// ' texture coords per vertex - 1 for simple uv, 8 max
					{
						for( int k=1;k<v_tc_size+1;k++ ) //' components per set - 2 for simple uv, 4 max
						{
							if( k==1 ) v_u=ReadFloat(file);
							if( k==2 ) v_v=ReadFloat(file);
							if( k==3 ) v_w=ReadFloat(file);
						}
					 v_surf->SetCoords(j,v_id,v_u,1-v_v,v_w);

					}
					
						
					new_tag=ReadTag(file);
														
				}
				printf("Read %d verts\n",vertc );
			//	Logger->Log("Read %d verts.\n",vertc);
	//		Logger->Log("Left Vrts.\n");
		}
		if( strcmp( tag,"TRIS" ) == 0 )
		{
		//	Logger->Log("Entered Tris.\n");
			//printf("Entered Tri check\n");
			
			read = true;
			int old_tr_brush_id=tr_brush_id;
			int tr_brush_id=ReadInt(file);
			//printf("Reached set up.\n");
			if( !(strcmp( prev_tag,"TRIS") == 0) || !(tr_brush_id==old_tr_brush_id) )
			{
					//printf("New surf\n");
					surf = new Surface(5000,5000);
					//printf("Add surf\n");
					if( mesh == NULL )
					{
						printf("No mesh to set surface to.\n");
			//			Logger->Log("No mesh to set surfce to.\n");
					}
					mesh->AddSurface( surf );
					printf("Set verts \n");
					surf->_verts=v_surf->_verts;
					for(int j=0;j<3;j++)
					{
						surf->_coords[j] = v_surf->_coords[j];
					}
										
					//surf.cols=v_surf.cols[..]
					//surf.norms=v_surf.norms[..]
					//surf.tnorm = v_surf.tnorm[..]
					//for( int j=0;j<v_surf.coordsets
					//	surf.coords[j] = v_surf.coords[j]
					//Next
					
									
					//surf.vert_tex_coords0=v_surf.vert_tex_coords0[..]
					//'surf.vert_tex_coords1=v_surf.vert_tex_coords1[..]
					surf->_vertc=v_surf->_vertc;
					//printf("prev tag str peaked.\n");
					
				}

				tr_sz=12;
				new_tag=ReadTag(file);
			//	printf("New tag:%s \n",new_tag);
				int tric=0;
				while(  !(NewTag(new_tag)==true) && (ftell(file)<fileSize) )
				{
					
					tr_vid0=ReadInt(file);
					tr_vid1=ReadInt(file);
					tr_vid2=ReadInt(file);
					tric++;
			
					//' Find out minimum and maximum vertex indices - used for animation so we know which vertex belongs to which
					//' surf (if vertex>surf_min and vertex<surf_max then this is correct surf) and also to prevent us calculating
					//' info for verts which lie outside current range
					
					/*		
					' ***note*** A possible optimisation to save on memory could be to slice the vert arrays for each surf
					' so they only contain vertex for triangles which belong to surf. This may be tricky however as it can only
					' be done when vertex min and max values are known, and the tri data has been read by then. Temp store
					' needed.
					' Shouldn't make a difference in terms of program speed.
					*/
					
					surf->AddTriangle(tr_vid0,tr_vid1,tr_vid2);
					
					new_tag=ReadTag(file);
				

			}
			printf("Read %d Tris \n",tric );
			//Logger->Log("Read %d tris.\n",tric );
				if( !(m_brush_id==-1)) mesh->SetMaterial(mat[m_brush_id]);
				if( !(tr_brush_id==-1)) surf->SetMaterial(mat[tr_brush_id]);
				
			//	Print "Updating Normals"
			//	If v_flags&1=0 Then mesh.UpdateNormals() ' if no normal data supplied then update normals
		//		Print "Done."
	//	Logger->Log("Left Tris.\n");
		}
		if( strcmp( tag,"ANIM" ) == 0 )
		{
//			Logger->Log("Entered Anim.\n");
				//printf("Entered anim check\n");
				read = true;
				a_flags=ReadInt(file);
				a_frames=ReadInt(file);
				a_fps=ReadFloat(file);
				
				if( !(mesh==NULL) )
				{
								
					mesh->_anim=true;
				
					//mesh.frames=a_frames
					mesh->_anim_seqs_first[0]=0;
					mesh->_anim_seqs_last[0]=a_frames;
					
					
					//' create anim surfs, copy vertex coords array, add to anim_surf_list
					mesh->_surf.start();
					
					while( mesh->_surf.next() == true )
					{
						Surface *anim_surf = new Surface(5000,5000);
												
						//anim_surf->verts=surf.verts;
						CopyArray( (float *)surf->_verts,(float *)anim_surf->_verts,surf->_vertc*3 );
						for(int i=0;i<3;i++)
						{
							CopyArray( (float *)surf->_coords[i]->_uv,(float *)anim_surf->_coords[i]->_uv,surf->_vertc*3 );
						}
						anim_surf->_tris = surf->_tris;
						anim_surf->_vertc = surf->_vertc;
						anim_surf->_tric = surf->_tric;
						anim_surf->_mat = surf->_mat;
						mesh->_anim_surf.add( anim_surf );
						
						
					}
	

			 }
			//Logger->Log("Left Anim.\n");
		}
		if( strcmp( tag,"BONE" ) == 0 )
		{
				//Logger->Log("Entered bone.\n");
				//p/rintf("Entered bone check");
				Logger->Log("In bone.\n");
				read = true;
				new_tag=ReadTag(file);

				int * bo_verts_id = (int *)malloc( 5000 * 4 );
				float * bo_verts_w = (float *)malloc( 5000 * 4 );
				int bo_no_verts=0;
			//	Logger->Log("Entering loop.\n");
				while(  !(NewTag(new_tag)==true) && (ftell(file)<fileSize) )
				{
					
					bo_vert_id=ReadInt(file);
					bo_vert_w=ReadFloat(file);
					
					
					bo_verts_id[bo_no_verts]=bo_vert_id;	
					bo_verts_w[bo_no_verts]=bo_vert_w;

					bo_no_verts++;
								
					new_tag=ReadTag(file);
				
				}
				
			//	Logger->Log("Leaving loop.\n");
				//bo_bone:TBone=New TBone
				bo_bone = new Bone;
				bo_bone->_type = Cls_Bone;
				bo_bone->_name = StringUtil->Num( node_level );
							
				bo_bone->no_verts=bo_no_verts;
				Logger->Log("Verts:%d \n",bo_no_verts);
				bo_bone->verts_id=bo_verts_id;
				bo_bone->verts_w=bo_verts_w;
				
			//	bo_bone.class$="BONE"
			//	bo_bone.name$=n
				
				bo_bone->_px=n_px;
				bo_bone->_py=n_py;
				bo_bone->_pz=n_pz;
				bo_bone->_sx=n_sx;
				bo_bone->_sy=n_sy;
				bo_bone->_sz=n_sz;
				bo_bone->_rx=n_rx;
				bo_bone->_ry=n_ry;
				bo_bone->_rz=n_rz;
				bo_bone->_qw=n_qw;
				bo_bone->_qx=n_qx;
				bo_bone->_qy=n_qy;
				bo_bone->_qz=n_qz;
				
				bo_bone->_n_px=n_px;
				bo_bone->_n_py=n_py;
				bo_bone->_n_pz=n_pz;
				bo_bone->_n_sx=n_sx;
				bo_bone->_n_sy=n_sy;
				bo_bone->_n_sz=n_sz;
				bo_bone->_n_rx=n_rx;
				bo_bone->_n_ry=n_ry;
				bo_bone->_n_rz=n_rz;
				bo_bone->_n_qw=n_qw;
				bo_bone->_n_qx=n_qx;
				bo_bone->_n_qy=n_qy;
				bo_bone->_n_qz=n_qz;
			//	Logger->Log("Creating Keys.\n");
				bo_bone->keys=new Keys;
				bo_bone->keys->frames=a_frames;
			//	Logger->Log("Set Vars.\n");
				
				bo_bone->keys->flags=(int *) malloc( (a_frames+1)*4 );
				for(int i=0;i<a_frames;i++)
				{
					bo_bone->keys->flags[i] = 0;
				}
				bo_bone->keys->px=(float *) malloc( (a_frames+1)*4 );
				bo_bone->keys->py=(float *) malloc( (a_frames+1)*4 );
				bo_bone->keys->pz=(float *) malloc( (a_frames+1)*4 );
				bo_bone->keys->sx=(float *) malloc( (a_frames+1)*4 );
				bo_bone->keys->sy=(float *) malloc( (a_frames+1)*4 );
				bo_bone->keys->sz=(float *) malloc( (a_frames+1)*4 );
				bo_bone->keys->qw=(float *) malloc( (a_frames+1)*4 );
				bo_bone->keys->qx=(float *) malloc( (a_frames+1)*4 );
				bo_bone->keys->qy=(float *) malloc( (a_frames+1)*4 );
				bo_bone->keys->qz=(float *) malloc( (a_frames+1)*4 );
				
				
			 	
			// Logger->Log("Checking root.\n");
				if( root_ent==NULL )  root_ent=bo_bone;
			//	Logger->Log("Checked.\n");
				//' if ent nested then add parent
			//	Logger->Log("Checking node level.\n");
				if( node_level>0 ) 
				{
		//			Logger->Log("In node if.\n");
					if( bo_bone == NULL )
					{
		//				Logger->Log("Bone is null.");
					}
					else
					{
	//					Logger->Log("Bone not null.\n");
					}
					bo_bone->AddParent(parent_ent);
				}
			//	Logger->Log("Checked.\n");
			//	Logger->Log("Quat to mat.\n");
				
				QuatToMat(bo_bone->_n_qw,bo_bone->_n_qx,bo_bone->_n_qy,bo_bone->_n_qz,bo_bone->_mat);
				Logger->Log("Called.\n");
				bo_bone->_mat->grid[3][0]=bo_bone->_n_px;
				bo_bone->_mat->grid[3][1]=bo_bone->_n_py;
				bo_bone->_mat->grid[3][2]=-bo_bone->_n_pz;
				
				if( !(bo_bone->_parent==NULL) ) 
				{
					Matrix * new_mat=bo_bone->_parent->_mat->Copy();
					new_mat->Multiply(bo_bone->_mat);
					bo_bone->_mat->Overwrite(new_mat);
				}

				bo_bone->inv_mat=MatInverse(bo_bone->_mat);
				
				if( !(strcmp( new_tag,"KEYS")==0) ) 
				{
					mesh->_bone_list.add( bo_bone ); 
					last_ent=bo_bone;
				}
				
		}
		
		
		if( strcmp( tag,"KEYS" ) == 0 )
		{	
				//Logger->Log("Entered Keys.\n");
				//printf("Entered keys check\n");
				read = true;
				k_flags=ReadInt(file);
			
				new_tag=ReadTag(file);

				
				while( !(NewTag(new_tag)==true) && (ftell(file)<fileSize) )
				{
					
						k_frame=ReadInt(file);
						
					if(k_flags&1)
					{
						k_px=ReadFloat(file);
						k_py=ReadFloat(file);
						k_pz=ReadFloat(file);
					}
					if(k_flags&2)
					{
						k_sx=ReadFloat(file);
						k_sy=ReadFloat(file);
						k_sz=ReadFloat(file);
					}
					if(k_flags&4)
					{
						k_qw=ReadFloat(file);
						k_qx=ReadFloat(file);
						k_qy=ReadFloat(file);
						k_qz=ReadFloat(file);
					}
					
					bo_bone->keys->flags[k_frame]+=k_flags;
					
					
					if(k_flags&1)
					{
						bo_bone->keys->px[k_frame]=k_px;
						bo_bone->keys->py[k_frame]=k_py;
						bo_bone->keys->pz[k_frame]=k_pz;
				
					}
					if(k_flags&2)
					{
						bo_bone->keys->sx[k_frame]=k_sx;
						bo_bone->keys->sy[k_frame]=k_sy;
						bo_bone->keys->sz[k_frame]=k_sz;
			
					}
					if(k_flags&4)
					{
						bo_bone->keys->qw[k_frame]=k_qw;
						bo_bone->keys->qx[k_frame]=k_qx;
						bo_bone->keys->qy[k_frame]=k_qy;
						bo_bone->keys->qz[k_frame]=k_qz;
						
					}
					
					new_tag=ReadTag(file);
						
				}
				
				
				if( !(strcmp(new_tag,"KEYS")==0) ) 
				{
					Logger->Log("Added.\n");
					 mesh->_bone_list.add( bo_bone );
					 last_ent=bo_bone;
				}
			//Logger->Log("Left Keys.\n");	
		}
		
		
		
	//	printf("Passed checks\n");
		if( read == false )
		{
	
		
		//		printf("Reading byte\n");
				ReadByte(file);
				
		}
		//Logger->Log("End of Loop.\n");
	}
	Logger->Log("Finished loading b3d.\n");
	
	fclose(file);
	printf("Done.\n");
	
	
	
	return (Entity *)root_ent;

}




/*
  For all 2d drawing operations. 
*/
class Pen
{
public:
	Pen()
	{
		_r=1;
		_g=1;
		_b=1;
		_a=1;
		_tex = NULL;
	}
	void Bind()
	{
		//Todo:Make a note of current color value.(If possible with PspGL??)
		glColor4f( _r,_g,_b,_a );
		if(!(_tex==NULL))
		{
			glEnable(GL_TEXTURE_2D);
			_tex->Bind();
		}
	}
	void Unbind()
	{
		//Todo:Return color to previous value.
		if(!(_tex==NULL))
		{
			glDisable(GL_TEXTURE_2D);
			_tex->Unbind();
		}
	}
	
	void SetColor( float r,float g,float b,float a)
	{
		_r=r;
		_g=g;
		_b=b;
		_a=a;
	}
	void SetColor( float r,float g,float b )
	{
		_r=r;
		_g=g;
		_b=b;
		_a=1; //Or leave it at it's present color?
	}
	void Rect( float x,float y,float w,float h )
	{
		glBegin(GL_QUADS);
		glTexCoord2f( 0,1 );
		glVertex2f( x,272.0f-y );
		glTexCoord2f( 1,1 );
		glVertex2f( x+w,272.0f-y );
		glTexCoord2f( 1,0 );
		glVertex2f( x+w,272.0f-y-h );
		glTexCoord2f( 0,0 );
		glVertex2f( x,272.0f-y-h );
		glEnd();
	}
	
	void Rect( float x,float y,float w,float h,float u0,float v0,float u1,float v1 )
	{
		v0 = 1-v0;
		v1 = 1-v1;
		
		glBegin(GL_QUADS);
		glTexCoord2f( u0,v1 );
		glVertex2f( x,272.0f-y );
		glTexCoord2f( u1,v1 );
		glVertex2f( x+w,272.0f-y );
		glTexCoord2f( u1,v0 );
		glVertex2f( x+w,272.0f-y-h );
		glTexCoord2f( u0,v0 );
		glVertex2f( x,272.0f-y-h );
		glEnd();
	}
	
	void Line( float x1,float y1,float x2,float y2 )
	{
		glBegin(GL_LINES);
		glVertex2f( x1,y1 );
		glVertex2f( x2,y2 );
		glEnd();
	}
	
	void RectHollow( float x1,float y1,float w,float h )
	{
		glBegin(GL_LINES);
		
		glVertex2f( x1,y1 );
		glVertex2f( x1+w,y1 );
		
		glVertex2f( x1+w,y1 );
		glVertex2f( x1+w,y1+h );
		
		glVertex2f( x1+w,y1+h );
		glVertex2f( x1,y1+h );
		
		glVertex2f( x1,y1 );
		glVertex2f( x1,y1+h );
		
		glEnd();
		
		
	}
	void SetTexture(Texture *tex)
	{
		_tex = tex;
	}
	Texture *_tex;
	float _r,_g,_b,_a;
};

class Joypad
{
public:
	Joypad()
	{
  	sceCtrlSetSamplingCycle(0);
		sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);	  
	}
	void Update()
	{
		_select = _square = _circle = _cross = _ltrigger = _rtrigger = _start = _left = _right = _up = _down = 0;
		sceCtrlReadBufferPositive(&pad, 1); 
  	float xi,yi;
  	xi = (float)pad.Lx / 255.0f;
  	yi = (float)pad.Ly / 255.0f;
  	_x = (xi-0.5f)*2.0f;
  	_y = ( (yi-0.5f)*2.0f);
  	if( fabs(_x)<0.2 ) _x=0;
  	if( fabs(_y)<0.2 ) _y=0;
 		if (pad.Buttons != 0){
			if (pad.Buttons & PSP_CTRL_SQUARE){
				_square = 1;
			}
			if (pad.Buttons & PSP_CTRL_TRIANGLE){
				_triangle = 1;
			} 
			if (pad.Buttons & PSP_CTRL_CIRCLE){
				_circle = 1;
			} 
			if (pad.Buttons & PSP_CTRL_CROSS){
				_cross = 1;
			} 

			if (pad.Buttons & PSP_CTRL_UP){
				_up = 1;
			} 
			if (pad.Buttons & PSP_CTRL_DOWN){
				_down = 1;
			} 
			if (pad.Buttons & PSP_CTRL_LEFT){
				_left = 1;
			} 
			if (pad.Buttons & PSP_CTRL_RIGHT){
				_right = 1;
			}      

			if (pad.Buttons & PSP_CTRL_START){
				_start = 1;
			}
			if (pad.Buttons & PSP_CTRL_SELECT){
				_select = 1;
			}
			if (pad.Buttons & PSP_CTRL_LTRIGGER){
				_ltrigger = 1;
			}
			if (pad.Buttons & PSP_CTRL_RTRIGGER){
				_rtrigger = 1;
			}      
		}
  }
  int _cross,_square,_triangle,_circle;
  int _left,_right,_up,_down;
  int _ltrigger,_rtrigger;
  int _select,_start;
  float _x,_y;
	SceCtrlData pad;
};

class Cursor
{
public:
	Cursor(Texture *tex)
	{
		_size = 32;
		_tex = tex;
		_style = new Pen();
		_style->SetTexture( _tex );
	//	_x = 480/2.0f;
//		_y = 272/2.0f;
		_x=0;
		_y=0;
	}
	void Move( float x,float y)
	{
		_x+=x;
		_y+=y;
		if(_x<0)_x=0;
		if(_y<0)_y=0;
		if(_x>480)_x=480;
		if(_y>272)_y=272;
	}
	void Render()
	{
		_style->Bind();
		_style->Rect(_x,_y,_size,_size );
		_style->Unbind();
	}
	void SetSize(float size)
	{
		_size = size;
	}
	Pen *_style;
	float _x,_y;
	float _size;
	Texture *_tex;
};

class Point2D
{
public:
	Point2D( float x,float y )
	{
		_x = x;
		_y = y;
	}
	float _x,_y;
};

class Style
{
public:
	Style()
	{
		_mr=_mg=_mb=0.5f;
		_hr=_hg=_hb=1.0f;
		_lr=_lg=_lb=0.2f;
	}
	// Not all derived styles use a skin. 
	virtual void SetSkins(Texture *butskin=NULL,Texture *frmskin = NULL)
	{
		_bskin = butskin;
		_fskin = frmskin;
	}

	virtual void DrawHIdent( int x1,int x2,int y ) = 0;
	virtual void DrawTexture( int x,int y,int w,int h,Texture *tex,int flipv = false ) = 0;
	virtual void DrawBody(int x,int y,int w,int h,char *name) = 0;
	virtual void DrawButton(int x,int y,int w,int h,char *txt = NULL,int pressed = false,Texture *img) = 0;
  virtual void DrawText(int x,int y,char *text) = 0;
  Texture *_bskin;
  Texture *_fskin;
  
  float _mr,_mg,_mb;
	float _hr,_hg,_hb;
	float _lr,_lg,_lb;
};

inline int swapY( int val )
{
	return 272-val;
}

inline int swapYH( int val,int h )
{
	return 272-val-h;
}

class Skin9Style : public Style 
{
public:
	float c[9];
	Pen *_pen;
	Skin9Style()
	{
		c[0] = 0;
			
		c[1] = 0.25;
		
		c[2] = 0.75;
		
		c[3] = 1;
		_pen = new Pen;
	}
	virtual void DrawHIdent( int x1,int x2,int y )
	{
		y = swapY( y );
		
		glDisable(GL_TEXTURE_2D);
		Blend_Solid();
		glBegin(GL_LINES);
			glColor3f( _hr,_hg,_hb );
			glVertex2f( x1,y );
			glVertex2f( x2,y );
			glColor3f( _lr,_lg,_lb );
			glVertex2f( x1,y-1 );
			glVertex2f( x2,y-1 );
		glEnd();
		
	}
	virtual void DrawText( int x,int y,char * text)
	{
		Blend_Mask();
		FontRenderer->RenderText( x,y,text );
	}
	virtual void DrawTexture( int x,int y,int w,int h,Texture *tex,int flipv = false )
	{
		int y1 = swapY( y );
		int y2 = swapYH( y,h );
		glEnable(GL_TEXTURE_2D);
		tex->Bind();
		glColor3f( 1,1,1 );
		if( flipv == true )
		{
				glBegin(GL_QUADS);
				glTexCoord2f( 0,0 );
				glVertex2f( x,y1 );
				glTexCoord2f( 1,0 );
				glVertex2f( x+w,y1 );
				glTexCoord2f( 1,1 );
				glVertex2f( x+w,y2 );
				glTexCoord2f( 0,1 );
				glVertex2f( x,y2 );
			glEnd();	
		}
		else
		{
			glBegin(GL_QUADS);
				glTexCoord2f( 0,0 );
				glVertex2f( x,y1 );
				glTexCoord2f( 1,0 );
				glVertex2f( x+w,y1 );
				glTexCoord2f( 1,1 );
				glVertex2f( x+w,y2 );
				glTexCoord2f( 0,1 );
				glVertex2f( x,y2 );
			glEnd();
		}
		tex->Unbind();
		glDisable(GL_TEXTURE_2D);
	}
	 virtual void DrawBody(int x,int y,int w,int h,char *txt = NULL)
  {
  	int y1,y2;
  	y1 = swapY( y );
  	y2 = swapYH( y,h );
  	glBegin(GL_QUADS);
			glColor3f( 0.0,0.5,0.5 );
			glVertex2f( x,y1 );
			glVertex2f( x+w,y1 );
			glColor3f( 1,1,1 );
			glVertex2f( x+w,y2 );
			glVertex2f( x,y2 );
		glEnd();
		
		glBegin(GL_LINES);
			
			glColor3f( _hr,_hg,_hb );
			
			glVertex2f( x,y1 );
			glVertex2f( x,y2 );
			glVertex2f( x,y1 );
			glVertex2f( x+w,y1 );
			
			glVertex2f( x+w+1,y1 );
			glVertex2f( x+w+1,y2+1 );
			glVertex2f( x,y2+1 );
			glVertex2f( x+w+1,y2+1 );
			glEnd();
			glBegin(GL_LINES);
			glColor3f( _lr,_lg,_lb );
			
			glVertex2f( x-1,y1-1 );
			glVertex2f( x+w,y1-1 );
			
			glVertex2f( x-1,y1-1 );
			glVertex2f( x-1,y2 );
			
			glVertex2f( x+w,y1-1 );
			glVertex2f( x+w,y2 );
			
			glVertex2f( x,y2+2 );
			glVertex2f( x+w,y2+2 );
			
		glEnd();
		glColor3f( 1,1,1 );
		FontRenderer->RenderText( x+10,y-FontRenderer->TextHeight()/2,txt);
				
  }
	virtual void DrawButton(int x,int y,int w,int h,char *txt = NULL,int pressed = false,Texture *img)
	{

		  	 
			Blend_Solid();
		  _bskin->Bind();
		  
		  _pen->Rect( x,y,w,h );		  
		  
		 _bskin->Unbind();
		 
		 
		 Blend_Mask();
		 
		 int mx = x+w/2;
		int my = y+h/2;
		int tw,th;
		glColor3f( 0,0,0 );
		int ox,oy;
		ox = 0;
		oy = 0;
		if( pressed == true )
		{
			ox+=2;
			oy+=2;
		}
		tw = FontRenderer->TextWidth( txt );
		th = FontRenderer->TextHeight();
		FontRenderer->RenderText( mx-tw/2-10+ox,my-th/2+oy,txt );
		glColor3f( 1,1,1 );
		FontRenderer->RenderText( mx-tw/2-12+ox,my-th/2-2+oy,txt );
		
		 
	}
	
	void HTile( float x,float y,float x2,int w,int h )
	{
			
	}
	
};



class GradientStyle : public Style
{
public:
	virtual void DrawHIdent( int x1,int x2,int y )
	{
		y = swapY( y );
		
		glDisable(GL_TEXTURE_2D);
		Blend_Solid();
		glBegin(GL_LINES);
			glColor3f( _hr,_hg,_hb );
			glVertex2f( x1,y );
			glVertex2f( x2,y );
			glColor3f( _lr,_lg,_lb );
			glVertex2f( x1,y-1 );
			glVertex2f( x2,y-1 );
		glEnd();
		
	}
	virtual void DrawText( int x,int y,char * text)
	{
		Blend_Mask();
		FontRenderer->RenderText( x,y,text );
	}
	virtual void DrawTexture( int x,int y,int w,int h,Texture *tex,int flipv = false )
	{
		int y1 = swapY( y );
		int y2 = swapYH( y,h );
		glEnable(GL_TEXTURE_2D);
		tex->Bind();
		glColor3f( 1,1,1 );
		if( flipv == true )
		{
				glBegin(GL_QUADS);
				glTexCoord2f( 0,0 );
				glVertex2f( x,y1 );
				glTexCoord2f( 1,0 );
				glVertex2f( x+w,y1 );
				glTexCoord2f( 1,1 );
				glVertex2f( x+w,y2 );
				glTexCoord2f( 0,1 );
				glVertex2f( x,y2 );
			glEnd();	
		}
		else
		{
			glBegin(GL_QUADS);
				glTexCoord2f( 0,0 );
				glVertex2f( x,y1 );
				glTexCoord2f( 1,0 );
				glVertex2f( x+w,y1 );
				glTexCoord2f( 1,1 );
				glVertex2f( x+w,y2 );
				glTexCoord2f( 0,1 );
				glVertex2f( x,y2 );
			glEnd();
		}
		tex->Unbind();
		glDisable(GL_TEXTURE_2D);
	}
  virtual void DrawBody(int x,int y,int w,int h,char *txt = NULL)
  {
  	int y1,y2;
  	y1 = swapY( y );
  	y2 = swapYH( y,h );
  	glBegin(GL_QUADS);
			glColor3f( 0.0,0.5,0.5 );
			glVertex2f( x,y1 );
			glVertex2f( x+w,y1 );
			glColor3f( 1,1,1 );
			glVertex2f( x+w,y2 );
			glVertex2f( x,y2 );
		glEnd();
		
		glBegin(GL_LINES);
			
			glColor3f( _hr,_hg,_hb );
			
			glVertex2f( x,y1 );
			glVertex2f( x,y2 );
			glVertex2f( x,y1 );
			glVertex2f( x+w,y1 );
			
			glVertex2f( x+w+1,y1 );
			glVertex2f( x+w+1,y2+1 );
			glVertex2f( x,y2+1 );
			glVertex2f( x+w+1,y2+1 );
			glEnd();
			glBegin(GL_LINES);
			glColor3f( _lr,_lg,_lb );
			
			glVertex2f( x-1,y1-1 );
			glVertex2f( x+w,y1-1 );
			
			glVertex2f( x-1,y1-1 );
			glVertex2f( x-1,y2 );
			
			glVertex2f( x+w,y1-1 );
			glVertex2f( x+w,y2 );
			
			glVertex2f( x,y2+2 );
			glVertex2f( x+w,y2+2 );
			
		glEnd();
		glColor3f( 1,1,1 );
		FontRenderer->RenderText( x+10,y-FontRenderer->TextHeight()/2,txt);
				
  }
	virtual void DrawButton(int x,int y,int w,int h,char *txt = NULL,int pressed = false,Texture *img)
	{
		int y1,y2;
		y1 = swapY( y );
		y2 = swapYH( y,h );
		
		glBegin(GL_QUADS);
			glColor3f( 0.0,0.5,0.5 );
			glVertex2f( x,y1 );
			glVertex2f( x+w,y1 );
			glColor3f( 1,1,1 );
			glVertex2f( x+w,y2 );
			glVertex2f( x,y2 );
		glEnd();
		
		int mx = x+w/2;
		int my = y+h/2;
		int tw,th;
		glColor3f( 0,0,0 );
		int ox,oy;
		ox = 0;
		oy = 0;
		if( pressed == true )
		{
			ox+=2;
			oy+=2;
		}
		tw = FontRenderer->TextWidth( txt );
		th = FontRenderer->TextHeight();
		FontRenderer->RenderText( mx-tw/2-10+ox,my-th/2+oy,txt );
		glColor3f( 1,1,1 );
		FontRenderer->RenderText( mx-tw/2-12+ox,my-th/2-2+oy,txt );
		
				
		glBegin(GL_LINES);
		 if(pressed == true )
		 	{
				glColor3f( _lr,_lg,_lb );
		 	}
		 	else
		 	{
				glColor3f( _hr,_hg,_hb );
			}
			glVertex2f( x,y1 );
			glVertex2f( x+w,y1 );
			glVertex2f( x,y1 );
			glVertex2f( x,y2 );
			if( pressed == true )
			{
				glColor3f( _hr,_hg,_hb );
			}
			else
			{
				glColor3f( _lr,_lg,_lb );
			}
		
			glVertex2f( x,y2 );
			glVertex2f( x+w,y2 );
			glVertex2f( x+w,y1 );
			glVertex2f( x+w,y2 );
		glEnd();
		
		if(!(img == NULL))
		{
			DrawTexture( x+1+ox,y+1+oy,w-2,h-2,img );
		}
	}
};

Style *DefaultStyle;

inline void SetDefaultStyle(Style *style)
{
	DefaultStyle = style;
}



// Virtual Mouse - Lmb = X Rmb = 0 M(middle)mb = Square.
// Z up (Mousewheel) = Dpad up Z down = (Mousewheel) = dpad down 
class Mouse

{
public:
	Mouse()
	{
		_x=0;
		_y=0;
		_z=0;
		
	}
	void Update( Joypad *joy )
	{
		_lmbclick = _rmbclick = _mmbclick = 0;
		float lx,ly,lz;
		lx = _x;
		ly = _y;
		lz = _z;
		_xi = joy->_x*2.0f*(1+joy->_ltrigger);
		_yi = joy->_y*2.0f*(1+joy->_rtrigger);
		_x += joy->_x*2.0f*(1+joy->_ltrigger);
		_y += joy->_y*2.0f*(1+joy->_ltrigger);
		if(_x<0)_x=0;
		if(_y<0)_y=0;
		if(_x>480)_x=480;
		if(_y>272)_y=272;
		if(joy->_up) _z++;
		if(joy->_down) _z--;
		_zi = _z-lz;
		if(_lmbdown == false)
		{
			if( joy->_cross == true )
			{
				_lmbclick = true;
			}
		}
		if(_rmbdown == false)
		{
			if( joy->_circle == true )
			{
				_rmbclick = true;
			}
		}
				if(_mmbdown == false)
		{
			if( joy->_square == true )
			{
				_mmbclick = true;
			}
		}
		_lmbdown = joy->_cross;
		_rmbdown = joy->_circle;
		_mmbdown = joy->_square;
	
	}
	
	float _x,_y,_z;
	float _xi,_yi,_zi;
	float _lmbclick,_rmbclick,_mmbclick;
	float _lmbdown,_rmbdown,_mmbdown;
	
};


enum EventType{ Mouse_Moved,Mouse_Click,Mouse_Down,Gadget_Action ,Mouse_Enter,Mouse_Leave };



class Gadget;


class Event
{
public:
	EventType _id;
	Gadget *_from;
	int _x,_y,_z;
	int _xi,_yi,_zi;
	int _extra;
	char *_data;
};


class Gadget
{
	public:
	
	Gadget()
	{
		_inarea = 0;
		_x = _y = _w = _h = 0;
		_style = DefaultStyle;
		_pressed = false;
		_checkpress = false;
		_relayevents = 0;
	}
	
	void DoScrollIf( int x,int y )
	{
		if( _xscroll == true )
		{
			int nx = _x + x;
			if( nx<_minx ) nx = _minx;
			if( nx>_maxx ) nx = _maxx;
			_x = nx;
		}
		
		if( _yscroll == true )
		{
			int ny = _y + y;
			if( ny<_miny ) ny = _miny;
			if( ny>_maxy ) ny = _maxy;
			_y = ny;
		}
	}
	
	void SetStyle( Style *style )
	{
		_style = style;
	}
	
	void Attach(Gadget *gad)
	{
		_attached.add( gad );
	}
	
	void RenderAll(int x,int y)
	{
		_attached.start();
		while( _attached.next()==true )
		{
			Gadget *gad = _attached.get();
			gad->Render(x+_x,y+_y);
		}
	}
	
	int UpdateAll(int x,int y,Mouse *mouse)
	{
		int did = false;
		_attached.start();
		while( _attached.next()==true )
		{
			Gadget *gad = _attached.get();
			if( gad->Update(x+_x,y+_y,mouse) == true)
			{
				did = true;
			}
		}
		return did;
	}
	void SetBounds( int x1,int y1,int x2,int y2 )
	{
		_minx = x1;
		_miny = y1;
		_maxx = x2;
		_maxy = y2;
	}
	
	void SetScroll( int xscroll,int yscroll )
	{
		_xscroll = xscroll;
		_yscroll = yscroll;
	}
	virtual void Callback( Event *evt ) = 0;
	virtual int Update( int x,int y,Mouse *mouse ) = 0;
	virtual void Render(int x,int y) = 0;
	
	List<Gadget *>_attached;
	int _x,_y,_w,_h;
	char *_txt[2555];
	int _txtcnt;
	Style *_style;
	int _inarea;
	int _pressed;
	int _checkpress;
	int _relayevents;
	int _xscroll,_yscroll;
	int _minx,_miny,_maxx,_maxy;
};




List<Event *>event_queue;

inline void AddEvent( Event *evnt )
{
	
	event_queue.add( evnt );

}

inline Event * Event_Create( int x,int y, Mouse * mouse,Gadget *from,EventType evnt,int extra=0,char * data=NULL)
{

	Event * out = new Event;

	out->_x = mouse->_x-x;
	out->_y = mouse->_y-y;
	out->_z = mouse->_z;
	out->_xi = mouse->_xi;
	out->_yi = mouse->_yi;
	out->_zi = mouse->_zi;
	out->_from = from;
	out->_id = evnt;
	out->_data = data;
	out->_extra = extra;

	return out;
}


inline int InRect( int x,int y,int x1,int y1,int w,int h)
{
	if( x>x1 )
	{
		if( y>y1 )
		{
			if( x<x1+w )
			{
				if( y<y1+h )
				{
					return true;
				}
			}
		}
	}
	return false;
}

class Body : public Gadget
{
public:
	Body()
	{
	}
	Body( int x,int y,int w,int h,char *name=NULL)
	{
		_x = x;
		_y = y;
		_w = w;
		_h = h;
		_txt[0]=name;
		_txtcnt = 1;
	}
	virtual void Callback( Event * evt )
	{
	}
	virtual void Render(int x,int y)
	{
		_style->DrawBody(x+_x,y+_y,_w,_h,_txt[0]);
		RenderAll(x,y);
	}
	virtual int Update( int x,int y,Mouse *mouse )
	{
		if( UpdateAll(x,y,mouse)==true ) return true;
		if( InRect( mouse->_x,mouse->_y,_x+x,_y+y,_w,_h)==true )
		{
			AddEvent( Event_Create( _x+x,_y+y,mouse,(Gadget *)this,Mouse_Moved) );
			if(_inarea == false)
			{
				_inarea = true;
				AddEvent( Event_Create( _x+x,y+_y,mouse,(Gadget *)this,Mouse_Enter) );
			}
			return true;
		}
		else
		{
			if(_inarea == true)
			{
				_inarea = false;
				AddEvent( Event_Create( _x+x,y+_y,mouse,(Gadget *)this,Mouse_Leave));
			}
			return false;
		}
	} 
};



class ImageList : public Gadget
{
public:
	ImageList(int x,int y,int w,int h)
	{
		_x = x;
		_y = y;
		_w = w;
		_h = h;
		_index=0;
		_lc=0;
		
	}
	virtual void Callback( Event *e )
	{
	}
	virtual int Update( int x,int y,Mouse *mouse )
	{
		_viewy += mouse->_zi * 3;
		
		_viewx += mouse->_zi * 3;
		return true;	
	}
	virtual void Render( int x,int y )
	{
		
		float iw = _w/2.2;
		float ih = _h/4.0f;
		float px = _x + x+5 + _viewx;
		float py = _y + y+5 + _viewy;
		float my = py + _h/2;
		float sx = x+_x;
		float sy = y+_y;
		float bx = x+_x+iw;
		float by = _y+y+_h;
		int ind = 0;
		float dw,dh;
		while(1)
		{
			float yd = fabs(my-(py+ih/2));
		
			dw = iw;
			dh = ih;
			if( (py>sy) && (py<by-dh) )
			{
			
			_style->DrawTexture( px,py,dw,dh,_item[ ind ] );
			glColor3f(1,1,1);
			FontRenderer->RenderText( (int)px+dw,(int)py+dh/2,_itemname[ ind ] );
			
			}
			ind++;
			py+=dh+4;
			if(ind==_lc) break;
		}
		
		
		
		RenderAll( x,y );
	}
	int AddItem( Texture *tex,char * name,char * data = NULL )
	{
		_itemname[_lc] = name;
		_itemdata[_lc]=data;
		_item[_lc]=tex;
		_lc++;
		return _lc-1;
	}
	char *_itemname[2555];
	char *_itemdata[2555];
	Texture *_item[2555];
	int _lc;
	int _index;
	float _viewy;
	float _viewx;
};



// Window class referes to a 'window of objects', not 
// a full window implentation with resize gadgets etc.
// you can build one, but instead use the various 
// gadgets as building blocks into a new extended class.
// let the tree grow..
class Window : public Gadget
{
public:
	Window()
	{
	}
	Window(int x,int y,int w,int h,char *name=NULL)
	{
		_x = x;
		_y = y;
		_w = w;
		_h = h;
		_txt[0] = name;
		_txtcnt = 1;
	}
	virtual void Callback( Event *e )
	{
	}
	virtual void Render(int x,int y)
	{
		//Windows are sightless containers.
		//_style->DrawButton( _x+x,_y+y,_w,_h,NULL);
		RenderAll(x,y);
	}
	virtual int Update( int x,int y,Mouse * mouse )
	{
	
		if( UpdateAll(x,y,mouse)==true ) return true;
	
		
		if( InRect( mouse->_x,mouse->_y,_x+x,_y+y,_w,_h)==true )
		{
		
			AddEvent( Event_Create( _x+x,_y+y,mouse,(Gadget *)this,Mouse_Moved) );
	
			if(_inarea == false)
			{
	
				_inarea = true;
				AddEvent( Event_Create( _x+x,y+_y,mouse,(Gadget *)this,Mouse_Enter) );
		
			}

			return true;
		}
		else
		{
			if(_inarea == true)
			{
			
				_inarea = false;
				AddEvent( Event_Create( _x+x,y+_y,mouse,(Gadget *)this,Mouse_Leave));
				
			}

			return false;
			
		}
	
	}
};

class Label : public Gadget
{
public:
	Label(int x,int y, char *text )
	{
		_x = x;
		_y = y;
		_w = 20;
		_h = 20;
		_txt[0] = text;
		_txtcnt = 1;
	}
	char * GetText()
	{
		return _txt[0];
	}
	void SetText(char *str)
	{
		//free((void *)_txt[0]);
		_txt[0] = str;
	}
	virtual void Callback( Event * evt )
	{
		
	}
	virtual int Update( int x,int y,Mouse *mouse )
	{
		if(UpdateAll(x,y,mouse)==true) return true;
	}
	virtual void Render(int x,int y)
	{
		_style->DrawText( _x+x,_y+y, _txt[0] );
	}
	
};


class Button : public Gadget
{
public:
	Button(int x,int y,int w,int h, char *name=NULL)
	{
		_x = x;
		_y = y;
		_w = w;
		_h = h;
		_txt[0] = name;
		_txtcnt = 1;
		_img = NULL;
	}
	void SetImage( Texture *img )
	{
		_img = img;
	}
	virtual void Callback( Event * evt )
	{
	}
	virtual void Render(int x,int y)
	{
		_style->DrawButton(_x+x,_y+y,_w,_h,_txt[0],_pressed,_img);
		RenderAll(x,y);
		
	}
	virtual int Update(int x,int y, Mouse *mouse )
	{
		if(UpdateAll(x,y,mouse)==true) return true;
					
		if( InRect( mouse->_x,mouse->_y,_x+x,_y+y,_w,_h)==true )
		{
			if(_inarea == false)
			{
				_inarea = true;
				AddEvent( Event_Create( _x+x,y+_y,mouse,(Gadget *)this,Mouse_Enter) );
			}
			if( mouse->_lmbclick==true )
			{
				AddEvent( Event_Create( _x+x,y+_y,mouse,(Gadget *)this,Gadget_Action,0) );
				_checkpress = true;
			}
			if(_checkpress == true)
			{
				if( mouse->_lmbdown==true )
				{
					AddEvent( Event_Create( _x+x,y+_y,mouse,(Gadget *)this,Mouse_Down,0) );
					_pressed = true;
					DoScrollIf( mouse->_xi,mouse->_yi );
				}
				else
				{
					_pressed = false;
				}
			}
			else
			{
				_pressed = false;
			}
			return true;
		}
		else
		{
			
			if( mouse->_lmbdown == false )
			{
				_checkpress = false;
				_pressed = false;
			}
			else
			{
				if( _checkpress == true )
				{
					DoScrollIf( mouse->_xi,mouse->_yi );
				}
			}
			
			if(_inarea == true)
			{
				_inarea = false;
				AddEvent( Event_Create( _x+x,y+_y,mouse,(Gadget *)this,Mouse_Leave));
			}
			return false;
		}
	
	}
	
	Texture *_img;
	
};



class SliderH : public Gadget
{
public:
	SliderH(int x,int y, int w,int h,char *butname)
	{
		_x =x;
		_y =y;
		_w = w;
		_h = h;
		_txt[0] = butname;
		_txtcnt = 1;
		_range = 100;
		_value = 50;
		int mx,my;
		mx = x+w/2;
		my = y+h/2;
		int bx,by;
		bx = mx - 5;
		by = y;
		_slide = new Button( bx,by,10,h,butname );
		_slide->SetScroll( true,false );
		_slide->SetBounds( x,y,x+w-10,y );
		Attach( _slide );
	}
	void SetRange( int range )
	{
		_range = range;
	}
	void SetValue( int value )
	{
		_value = value;
		float fp = (float)_value / (float)_range;
		float bx = (float)_w * fp;
		_slide->_x = bx-5;
	}
	int GetValue()
	{
		return _value;
	}
	virtual int Update( int x,int y,Mouse *mouse )
	{
			if(UpdateAll(x,y,mouse)==true) return true;
			float xo = (float)_slide->_x+5;
			float tx = (float)_w;
			float fp = xo / tx;
			_value = _range * ( xo / tx );
	}
	
	virtual void Render( int x,int y )
	{
		_style->DrawHIdent( x+_x,x+_x+_w,y+_y+_h/2 );
		RenderAll( x,y );
	}
	virtual void Callback( Event * evt )
	{
	}
	Button *_slide;
	int _range,_value;
};

class Canvas : public Gadget
{
public:
	Canvas(int x,int y,int w,int h,int texw = 256, int texh = 256 )
	{
		_x = x;
		_y = y;
		_w = w;
		_h = h;
		_raw = (char *)malloc( texw * texh * 3 );
		_surfw = texw;
		_surfh = texh;
		ClearSurface();
		_surface = new Texture( _raw,_surfw,_surfh );
	
	}
	void SetRawAndSync(char *buf,int freeold = false)
	{
		if(freeold == true)
		{
			free(_raw);
		}
		_raw = buf;
		glEnable(GL_TEXTURE_2D);
		_surface->Bind();
		_surface->Upload( _raw );
		_surface->Unbind();
		glDisable(GL_TEXTURE_2D);
		
	}
	void ClearSurface()
	{
		for(int x=0;x<_surfw;x++)
		{
			for(int y=0;y<_surfh;y++)
			{
				char *b = _raw + ( y * _surfw * 3) + x * 3;
				b[0] = 0;
				b[1] = 0;
				b[2] = 0;
			}
		}
	}
  virtual int Update(int x,int y,Mouse *mouse)
  {
  	
  }
  virtual void Render(int x,int y)
  {
  	glEnable(GL_TEXTURE_2D);
  	_surface->Bind();
  	Blend_Solid();
  	_style->DrawTexture( _x+x,_y+y,_w,_h,_surface,false );
  	_surface->Unbind();
  	glDisable(GL_TEXTURE_2D);
  	RenderAll( x+_x,y+_y );
  }
  virtual void Callback( Event *evt )
  {
  }
	Texture *_surface;
	char *_raw;
	int _surfw,_surfh;
};

class Gui
{
public:
	Gui( Cursor *cursor,Joypad *joy )
	{
		_cur = cursor;
		_joy = joy;
		_pen = new Pen;
		_back = NULL;
		_bcon = true;
		_frame = new Window(0,0,480,272,"Main Gui Surface");
		_mouse = new Mouse;
	}
	void AttachGadget( Gadget *gad )
	{
		_frame->Attach( gad );
	}
	int CountEvents()
	{
		event_queue.start();
		int nn=0;
		while( event_queue.next()==true )
		{
			nn++;
		}
		return nn;
	}
	Event * GetEvent(int index)
	{
		int nn=0;
		Event *ret;
		event_queue.start();
		while( event_queue.next()==true )
		{
			if( nn == index )
			{
				ret = event_queue.get();
			}
			nn++;
		}
		return ret;
	}
	void ClearEvents()
	{
		event_queue.start();
		while( event_queue.next()==true )
		{
			event_queue.remove();
		}
		event_queue.start();
	}
	void Update()
	{
	
		_mouse->Update( _joy );
		float mx,my;
		mx = _joy->_x*1.5f;
		my = _joy->_y*1.5f;
		if(_bcon == true)
		{
			if(_joy->_ltrigger == 1)
			{
				mx *= 2;
				my *= 2;
			}
			if(_joy->_rtrigger == 1)
			{
				mx *= 0.6;
				my *= 0.6;
			}
		}
		_cur->_x = _mouse->_x;
		_cur->_y = _mouse->_y;
		//_cur->Move( mx,my );
		
		ProcessUpdate();
		ProcessEvents();
	
	}
	void ProcessUpdate()
	{
		_frame->Update(0,0,_mouse );
	}
	void ProcessEvents()
	{
		event_queue.start();
		while( event_queue.next() == true )
		{
			Event * e = event_queue.get();
			switch( e->_id )
			{
				
			}
		}
		
	}
	
	void Render()
	{
		Blend_Solid();
		if( !(_back==NULL))
		{
			glEnable(GL_TEXTURE_2D);
			_back->Bind();
			_pen->Rect(0,0,480,272);
			_back->Unbind();
		}
		glDisable(GL_TEXTURE_2D);
		_frame->Render(0,0);
		glEnable(GL_TEXTURE_2D);
		Blend_Mask();
		_cur->Render();
	}

	void SetBackground(Texture *tex)
	{	
		_back = tex;
	}
	Cursor *_cur;
	Joypad *_joy;
	Mouse *_mouse;
	Texture *_back;
	Pen *_pen;
	int _bcon;
	Window *_frame;
};

#define printf pspDebugScreenPrintf

void AddTest() 
{ 


int num1=5; 
int num2=10; 
int out=0; 
int to = 5;
float r17,r16;
r16=0;
r17=0;
for(int i=0;i<to;i++)
{
	float res = num1*num2;
	r16 = res;
	r17 = r16+r17;
}
printf("C Res:%f",r17);

num1 =5;
num2 = 10;
out = 0;
to = 5;
asm __volatile__ ("\n\ 
ori $5,$0,0\n\
ori $17,$0,0\n\
loop:\n\
mul %1,%2\n\
mflo $16\n\
addu $19,$16,$17\n\
move $17,$19\n\
addiu $5,$5,1\n\
bne $5,%3,loop\n\
nop\n\
move %0,$17\n\
":"=r"(out):"r"(num1),"r"(num2),"r"(to)); 

	printf("Asm Result:%d \n",out); 
}

class c1
{
	public:
		void test()
		{
			printf("Test 1 called \n");
		}
};

class c2 : c1
{
	public:
		void test()
		{
			printf("Test 2 called.\n");
		}
};

//#define new(x) memalign(64, sizeof(x) );

int lua_print(lua_State *L)
{
	glDisable(GL_TEXTURE_2D);
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glVertex2f(20,20);
	glVertex2f(200,20);
	glVertex2f(200,200);
	glVertex2f(20,200);
	glEnd();
	
	return 0;
}

//-----------------END OF Raptor3D


//-----------------DEMO classes

Texture *starTexture;
Material *starMat;

void InitDemo()
{
	starTexture = new Texture("star1.jpg",JPEG);
	starMat = new Material;
	starMat->AddTexture( starTexture );
	
}

// -------[ Opening Credit System ] --

class Credit
{
public:
	Credit( char * txt,float x,float y,float z)
	{
		_mod = Factory->ProduceText( txt,NULL );
		_mod->PointAt(0,200,0);
		_mod->Position( x,y,z );	
	}
	void Move( float x,float y,float z)
	{
		_mod->Position( _mod->XPos(false)+x,_mod->YPos(false)+y,_mod->ZPos(false)+z );
	}
	Entity * _mod;
};

class CreditEngine
{
public:
	CreditEngine(float inc,float rate)
	{
		_y = 0;
		_inc = inc;
		_rate = rate;
	}
	void Add(char *txt)
	{
		Credit *cred = new Credit( txt,0,0,_y );
		_creds.add( cred );
		_y += _inc;
	}
	void Update()
	{
		_creds.start();
		while( _creds.next() == true )
		{
			Credit *c = _creds.get();
			c->Move( 0,0,_rate );
		}
	}
	float _inc;
	float _y;
	float _rate;
	List<Credit *>_creds;
};

// End of [ Opening Credit System ]



const int m_play = 1,m_rew =2,m_ff = 3,m_pause = 4,m_stop = 5;


int main(int argc, char **argv) 
{	
	pspDebugScreenInit();
	  scePowerSetClockFrequency(333, 333, 166);
	
	InitRaptor();

	Joypad *joy = new Joypad;

	SetDefaultStyle( new GradientStyle );
	Display *sys = new Display(argc,argv);
	Pen *pen = new Pen;
	
	Texture *bmp = new Texture("back1.bmp",BMP);
	Material *mat = new Material;
	mat->AddTexture( bmp );
	InitDemo();
 	Camera *vcam = Factory->ProduceCamera();
 	
 	//Entity *box = prefab->CreateCube( 10 );
 	vcam->Position(0,20,-20);
 	
	//box->SetMaterial( mat );
	Renderer->_ActiveCam = vcam;
	//box->Position(0,0,0);
//	box->Rotate(0,0,0);
	vcam->PointAt(0,0,0);
	
	
		Texture * ftex = new Texture("star.bmp",BMP);
	ParticleEngine *pe = new ParticleEngine( NULL,1000 );
	
	float bx,by;
	bx=0;
	by=0;
	glDisable( GL_CULL_FACE );
	//Entity *ent = LoadB3D( (const char *)"zombie.b3d",NULL );

//	ent->Rotate(90,90,90);

//	ent->Position(50,0,0);
	//ent->ScaleMesh(10,10,10);

	Font *fnt = new Font("fnt1.bmp",32,32);
 	FontRenderer->SetActive( fnt );
 	FontRenderer->SetColor(1,1,1);
	char * yawtxt = NULL;
	char * angtxt = NULL;
	int mode;
	mode = m_play;
//	sceGuDisable( GU_CLIP_PLANES );
 	float cx,cy;
 	cx=0;
 	cy=0;
 float ex=0;
 Logger->Log("Returned Entity.\n");
// Dumpent( (Base *)ent,0 );


 // ent->Position(25,25,25);


  u64 fp_lms;
  int fp_frms;
	int fp_fps;
	int fp_first = true;

	int lm = sceRamLeft();
	float frame=1;
	Logger->Log("Entering Main Loop.\n");
	pe->Emit( 12,0,0,0,0,0 );
	
  SoundEngine * se = new SoundEngine();
  Sound *music = new Sound("theme.wav");
  music->Play();
 	
	vcam->Position(0,240,-80 );
	vcam->PointAt(0,0,0);
	
	CreditEngine *ce = new CreditEngine( -40,1 );
	ce->_y = -200;
	ce->Add("SWITCHBLADE");
	ce->_y -= 200;
	ce->Add("Once upon a time, far far away\n");
	ce->Add("Mankind sent secret conolists to the stars\n");
	ce->Add("To populate and install bases of power\n");
	ce->Add("In the SwitchBlade system, by 2210\n");
	ce->Add("the faction had grew to be most poowerful\n");
	ce->Add("force in the known universe\n");
	ce->Add("Their technolgoy was Godlike\n");
	ce->Add("And their intentions equally brillant\n");
	ce->Add("Earth that was, eventually developed\n");
	ce->Add("wormhole technology that bridged the gap\n");
	ce->Add("And set sail on the mission Liberty one.\n");
	ce->_y -= 100;
	ce->Add("You are Captain Buck Rogers.\n");
	ce->Add("Your mission is breach the nsx wormhole\n");
	ce->Add("To seek out and explore new systems.\n");
	ce->Add("The Ancients are waiting.\n");
	ce->Add("And they are plotting against us.\n");
	ce->Add("Local Alien races maybe your only allies.\n");
	ce->_y -= 200;
	ce->Add("SWITCHBLADE");
	
	
//	Entity *_mod;
//	_mod = Factory->ProduceText( "Once upon a time, in a galaxy far far away.\n",NULL );
//	_mod->PointAt(0,200,0);

	Texture *btex = new Texture("intro1.bmp",BMP);
	
	Entity *back = Prefab->CreateQuad(1000,700);
	back->Position(0,-200,0);
	Material *bmat = new Material;
	bmat->AddTexture(btex);
	back->SetMaterial(bmat);
	back->SetVisualizer( new VL_VertexArray );
	back->PointAt( vcam );
	
	Logger->Log("Main\n");
 	while(1)
  {
  	for(int i=0;i<2;i++)
  	{
  	se->Update();
  	}
 
  	u32 dis = lm - sceRamLeft();
  	lm = sceRamLeft();
  	//Logger->Log("Ram leak:%d \n",dis );
		ce->Update();
  	pe->Update();
 
  
  		//ent->SetAnimTime(frame);
  		frame++;
  		
  		if(frame>199) frame=1;
  //	Logger->Log("Mloop.\n");
  	//Profile->Enter("Main Loop\n");

 		u64 ms = GetTick();
 		if( (ms>fp_lms) || (fp_first == true) )
 		{
 			fp_fps = fp_frms;
 			fp_frms=0;
 			fp_lms=ms+1000;
 			fp_first = false;
 		}

 		fp_frms++;
  	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
		//vcam->Move(0,0,0);
	//	vcam->Other(joy->_x,0,joy->_y);
		glLoadIdentity();

  	joy->Update();

  	
 		float xi,yi;
 		xi = joy->_x;
 		yi = joy->_y;
 		
 		if(fabs(xi)>0.4)
 		{
 			cx+=xi;
 		}
 		if(fabs(yi)>0.4)
 		{
 			cy+=yi;
 		}
 			
 	// 		cy++;
 		if(cy>360) cy=0;
 	//	vcam->Rotate(cp,cy,0);
 		//Logger->Log("About to position.\n");
 	//	ent->Position(0,0,0);
 		//Logger->Log("About to rotate.\n");
 	
 		//Logger->Log("Done.\n");
 		
 	//	vcam->PointAt(0,0,0);

  	//FontRenderer->RenderText(0,0,"Gui Test Alpha 1");
  	
		
 		Renderer->RenderScene(); 
  

 		
 		sys->Draw2D();
 		
 		if( !(yawtxt == NULL ) )
 		{
 			free( (void *)yawtxt );
 		}
 		yawtxt = StringUtil->Num( fp_fps );
 		FontRenderer->RenderText(0,0,yawtxt);
 		Blend_Solid();
 		
 		
 	 
 	//	sys->Draw2D();
		if(joy->_select==1)
		{
			CloseRaptor();
			sceKernelExitGame();
		}
  	if( joy->_rtrigger )
  	{
  		screenshot("mplay");
  	}
  	glutSwapBuffers();
  	//Profile->Leave("Main Loop\n");
  }

  return (0);
}
