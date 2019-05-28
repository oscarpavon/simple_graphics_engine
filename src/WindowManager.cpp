#include "engine.h"
#include "WindowManager.hpp"


#if defined(LINUX) && defined (ES2)

void WindowManager::configure_egl(){
    ///////  the egl part  //////////////////////////////////////////////////////////////////
   //  egl provides an interface to connect the graphics related functionality of openGL ES
   //  with the windowing interface and functionality of the native operation system (X11
   //  in our case.
 
   egl_display  =  eglGetDisplay( (EGLNativeDisplayType) x_display );
   if ( egl_display == EGL_NO_DISPLAY ) {
      cerr << "Got no EGL display." << endl;
      
   }
 
   if ( !eglInitialize( egl_display, NULL, NULL ) ) {
      cerr << "Unable to initialize EGL" << endl;
     
   }
 
   EGLint attr[] = {       // some attributes to set up our egl-interface
      EGL_BUFFER_SIZE, 16,
      EGL_RENDERABLE_TYPE,
      EGL_OPENGL_ES2_BIT,
      EGL_DEPTH_SIZE, 24,
      EGL_NONE
   };
 
   EGLConfig  ecfg;
   EGLint     num_config;
   if ( !eglChooseConfig( egl_display, attr, &ecfg, 1, &num_config ) ) {
      cerr << "Failed to choose config (eglError: " << eglGetError() << ")" << endl;
      
   }
 
   if ( num_config != 1 ) {
      cerr << "Didn't get exactly one config, but " << num_config << endl;
    
   }
 
   egl_surface = eglCreateWindowSurface ( egl_display, ecfg, x_window, NULL );
   if ( egl_surface == EGL_NO_SURFACE ) {
      cerr << "Unable to create EGL surface (eglError: " << eglGetError() << ")" << endl;
      
   }
 
   //// egl-contexts collect all state descriptions needed required for operation
   EGLint ctxattr[] = {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };
   egl_context = eglCreateContext ( egl_display, ecfg, EGL_NO_CONTEXT, ctxattr );
   if ( egl_context == EGL_NO_CONTEXT ) {
      cerr << "Unable to create EGL context (eglError: " << eglGetError() << ")" << endl;
      
   } 
   //// associate the egl-context with the egl-surface
   eglMakeCurrent( egl_display, egl_surface, egl_surface, egl_context );
}

void WindowManager::move_cursor_to_center(){
   
   XWarpPointer(x_display, None, x_window, 0, 0, 0, 0, 400, 300);
}


void WindowManager::create_window_xorg(){
       ///////  the X11 part  //////////////////////////////////////////////////////////////////
   // in the first part the program opens a connection to the X11 window manager
   //
 
   x_display = XOpenDisplay ( NULL );   // open the standard display (the primary screen)
   if ( x_display == NULL ) {
      cerr << "cannot connect to X server" << endl;
      
   }
 
   x_window  =  DefaultRootWindow( x_display );   // get the root window (usually the whole screen)
 
   XSetWindowAttributes  swa;
   swa.event_mask  =    ExposureMask | PointerMotionMask | KeyPressMask | 
                        KeyReleaseMask | StructureNotifyMask | ButtonPressMask |
                        ButtonReleaseMask;
 
   x_window  =  XCreateWindow (   // create a window with the provided parameters
              x_display, x_window,
              0, 0, this->window_width, this->window_height,   0,
              CopyFromParent, InputOutput,
              CopyFromParent, CWEventMask,
              &swa ); 

   XMapWindow ( x_display , x_window );             // make the window visible on the screen
   XStoreName ( x_display , x_window , this->window_name); // give the window a name 

   Atom wmDeleteMessage = XInternAtom(x_display, "WM_DESTROY_WINDOW", False);
   XSetWMProtocols(x_display, x_window, &wmDeleteMessage, 1);

   configure_egl();
    
}

void WindowManager::clear(){
      eglDestroyContext ( egl_display, egl_context );
      eglDestroySurface ( egl_display, egl_surface );
      eglTerminate      ( egl_display );
      XDestroyWindow    ( x_display, x_window );
      XCloseDisplay     ( x_display );
}
#endif


#if defined (GLFW)
void WindowManager::create_window_glfw(){
   	if( !glfwInit() )
		{
			fprintf( stderr, "Failed to initialize GLFW\n" );
			return;
		}	
	
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		
		glfw_window = glfwCreateWindow(this->window_width, this->window_height, this->window_name.c_str(), nullptr, nullptr);
		if( glfw_window == NULL ){
			fprintf( stderr, "Failed to open GLFW window\n" );
			glfwTerminate();
			return;
		}
}
void WindowManager::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
   Engine* engine = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
   engine->renderer.framebufferResized = true;
}

#endif
void WindowManager::error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s \n Error Number: %i\n", description, error);
}
void WindowManager::create_window(){
   #ifdef X11
      create_window_xorg();
   #endif
   #ifdef GLFW
      create_window_glfw();
   #endif
   #ifdef WAYLAND
      create_wayland_window();
   #endif
   #if defined (WINDOWS) && defined (GLFW)
	  glfwSetErrorCallback(error_callback);
      if( !glfwInit() )
		{
			fprintf( stderr, "Failed to initialize GLFW\n" );
			return;
		}	
	
		//glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);


		glfw_window = glfwCreateWindow(this->window_width, this->window_height, this->window_name.c_str(), nullptr, nullptr);
		if( glfw_window == NULL ){
			fprintf( stderr, "Failed to open GLFW window\n" );
			glfwTerminate();
			return;
		}

  

   #endif



}
#ifdef WINDOWS

Engine* WindowManager::static_engine_pointer;

void WindowManager::create_window_windows(HINSTANCE hInstance) {
	LPCSTR title = this->window_name;
	LPCSTR clas_name = "engine";
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = WindowManager::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = clas_name;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Windows Desktop Guided Tour"),
			NULL);

		
	}


	window_handler = CreateWindow(
		wcex.lpszClassName,
		title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		800, 600,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!window_handler)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Windows Desktop Guided Tour"),
			NULL);

	
	}
	this->window_instance = hInstance;
	WindowManager::static_engine_pointer = engine;

	#ifdef OPENGL
	prepare_window_to_opengl();
	#endif // OPENGL

	
}


LRESULT CALLBACK WindowManager::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		static_engine_pointer->window_manager.close_window = true;
		break;
	case WM_KEYDOWN: {
		static_engine_pointer->input.handle_key_pressed(wParam);
		break;
	}
	case WM_KEYUP: {
		static_engine_pointer->input.handle_key_released(wParam);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#endif // WINDOWS

#ifdef OPENGL
void WindowManager::prepare_window_to_opengl()
{
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
		PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
		32,                   // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                   // Number of bits for the depthbuffer
		8,                    // Number of bits for the stencilbuffer
		0,                    // Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};


	device_context = GetDC(window_handler);


	int  letWindowsChooseThisPixelFormat;
	letWindowsChooseThisPixelFormat = ChoosePixelFormat(device_context, &pfd);
	SetPixelFormat(device_context, letWindowsChooseThisPixelFormat, &pfd);


	rendering_context = wglCreateContext(device_context);

	wglMakeCurrent(device_context,rendering_context);

	MessageBoxA(0, (char*)glGetString(GL_VERSION), "OPENGL VERSION", 0);
}

#endif // OPENGL



#ifdef WAYLAND
void WindowManager::create_wayland_window(){
   struct wl_display* display = nullptr;
   display = wl_display_connect(nullptr);
   

}

#endif 

#ifdef ANDROID
   #include <android/log.h>
   #include "android_helper.h"

	void WindowManager::create_window(android_app *pApp) {
		display = eglGetDisplay(EGL_DEFAULT_DISPLAY);


		eglInitialize(display, NULL, NULL);

		/* get an appropriate EGL frame buffer configuration */
		eglChooseConfig(display, attribute_list, &config, 1, &num_config);


		context = eglCreateContext(display, config, EGL_NO_CONTEXT, GiveMeGLES2);


		surface = eglCreateWindowSurface(display, config, pApp->window, NULL);

      EGLint w = 0;
      EGLint h = 0;

		eglQuerySurface(display, surface, EGL_WIDTH, &w);

      eglQuerySurface(display, surface, EGL_HEIGHT, &h);

        eglMakeCurrent(display, surface, surface, context);
      LOGW("windows size: %f %f",(float)w,(float)h);


        this->window_width = float(w);
        this->window_height = float(h);

        this->update_window_size();



	}
	
#endif

void WindowManager::check_events(){
    #if defined(LINUX) && defined(X11) && defined (ES2)
      while ( XPending ( x_display ) ){
         XEvent  xev;
        
        
         XNextEvent( x_display, &xev );
         engine->input.check_input_event(engine,xev);

         if(xev.type == ConfigureNotify){
            XConfigureEvent xce = xev.xconfigure;
            this->window_width = xce.width;
            this->window_height = xce.height;
            update_window_size();           
         }
         
        /*  if(xev.type == ClientMessage && xev.xclient.data.l[0] == wmDeleteMessage){
            std::cout << "close message\n";
            
         } */
      }
      
     
   #endif
#if defined GLFW
      	glfwPollEvents();
   #endif

#ifdef  ANDROID
    int events;
    android_poll_source *pSource;
    if (ALooper_pollAll(0, nullptr, &events, (void **) &pSource) >= 0) {
        if (pSource) {
            pSource->process(engine->pAndroid_app, pSource);
        }
    }
#endif

#ifdef WINDOWS
	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#endif // WINDOWS

}



void WindowManager::swap_buffers(){
      #ifdef ANDROID
         eglSwapBuffers(display, surface);
      #endif
       #if defined(LINUX) && defined(X11) && defined (ES2)
         eglSwapBuffers ( egl_display, egl_surface );
      #endif
      #if defined (GLFW)
         glfwSwapBuffers(glfw_window);
      #endif
}

WindowManager::~WindowManager(){
   #ifdef GLFW
   glfwDestroyWindow(glfw_window);

   glfwTerminate();
   #endif
}

void WindowManager::update_window_size(){
   #ifdef LINUX
	int width = 0, height = 0;
   #ifdef VULKAN
      while (width == 0 || height == 0) {
         glfwGetFramebufferSize(glfw_window, &width, &height);
         glfwWaitEvents();
      }
   #endif

   if(width != 0 && height != 0){
      window_width = width;
	   window_height = height;
   }

   engine->main_camera.screen_width = window_width;
   engine->main_camera.screen_height = window_height;
   #if defined (ES2) || defined (ANDROID)  || defined(VULKAN)
   engine->update_render_size();
   #endif
   #endif//(linux)
}

bool WindowManager::window_should_close(){
  
   #if defined (GLFW)
      this->close_window = glfwWindowShouldClose(glfw_window);
   #endif
   #if defined(ES2)
      
   #endif

#ifdef ANDROID
    this->close_window = engine->pAndroid_app->destroyRequested;
#endif
   return this->close_window;
}