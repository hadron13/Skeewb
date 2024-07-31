package renderer

import skeewb "skeewb:core"
import "base:runtime"
import "vendor:sdl2"
import gl "vendor:OpenGL"


@(export)
load :: proc"c"(core: ^skeewb.core_interface) -> skeewb.module_desc{
    context = runtime.default_context()

    core.event_listen("start", skeewb.event_callback(start));
    core.event_listen("loop", skeewb.event_callback(loop));
    core.event_listen("quit", skeewb.event_callback(quit));

    return (skeewb.module_desc){
        modid = "renderer",
        version = {0, 0, 1},
        interface = nil,
    }
}

window : ^sdl2.Window
gl_context : sdl2.GLContext

start :: proc"c"(core: ^skeewb.core_interface){
    context = runtime.default_context()

    sdl2.Init(sdl2.INIT_EVERYTHING)

    sdl2.GL_SetAttribute(sdl2.GLattr.CONTEXT_MAJOR_VERSION, 3)
    sdl2.GL_SetAttribute(sdl2.GLattr.CONTEXT_MINOR_VERSION, 3)
    sdl2.GL_SetAttribute(sdl2.GLattr.CONTEXT_PROFILE_MASK, i32(sdl2.GLprofile.CORE))
    


    window = sdl2.CreateWindow("Skeewb", sdl2.WINDOWPOS_CENTERED, sdl2.WINDOWPOS_CENTERED, 600, 400, sdl2.WINDOW_RESIZABLE | sdl2.WINDOW_OPENGL)
    if(window == nil){
        skeewb.console_log(.ERROR, "could not create a window sdl error: %s", sdl2.GetError())
        core.quit(-1)
    }
    skeewb.console_log(.INFO, "successfully created a window")

    gl_context = sdl2.GL_CreateContext(window);
    if(gl_context == nil){
        skeewb.console_log(.ERROR, "could not create an OpenGL context sdl error: %s", sdl2.GetError())
        core.quit(-1)
    }
    skeewb.console_log(.INFO, "successfully created an OpenGL context")

    sdl2.GL_SetSwapInterval(-1)

    gl.load_up_to(3, 3, sdl2.gl_set_proc_address)
    skeewb.console_log(.INFO, "loaded OpenGL version %s", gl.GetString(gl.VERSION))
    skeewb.console_log(.INFO, "vendor: %s", gl.GetString(gl.VERSION))
    
}

loop :: proc"c"(core: ^skeewb.core_interface){

    context = runtime.default_context()
    event : sdl2.Event
        
    for ;sdl2.PollEvent(&event);{
        if event.type == sdl2.EventType.QUIT {
            core.quit(0)
        }
    }
    gl.ClearColor(0.1, 0.1, 0.1, 1.0)
    gl.Clear(gl.COLOR_BUFFER_BIT)

    sdl2.GL_SwapWindow(window)

}

quit :: proc"c"(core: ^skeewb.core_interface){
    sdl2.GL_DeleteContext(gl_context)
    sdl2.DestroyWindow(window)
    sdl2.Quit()
}

