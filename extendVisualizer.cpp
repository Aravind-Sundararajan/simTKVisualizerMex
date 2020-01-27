#include <Simbody.h>
#include <OpenSim.h>
#include "C:\Users\aravi\Documents\MATLAB\mexplus-master\include\mexplus.h"
#include "lodepng.h"
#include <cstdlib>
#include <cmath>
#include <string>
#include <algorithm>
#include <set>
#include <vector>
#include <utility>
#include <limits>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <thread>
#include <condition_variable>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif
// Get gl and glut using the appropriate platform-dependent incantations.
#if defined(__APPLE__)
// OSX comes with a glut implementation. In OSX 10.9 and 10.10, this
// glut is deprecated and emits deprecation warnings.
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <GLUT/glut.h>
#elif defined(_WIN32)
#include "glut32/glut.h"    // we have our own private headers
#include "glut32/glext.h"

// A Windows-only extension for disabling vsync, allowing unreasonably
// high frame rates.
PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT;

// These will hold the dynamically-determined function addresses.

// These functions are needed for basic Visualizer functionality.
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLACTIVETEXTUREPROC glActiveTexture;

// These are needed only for saving images and movies.
// Use old EXT names for these so we only require OpenGL 2.0.
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;

// see initGlextFuncPointerIfNeeded() at end of this file
#else
// Linux: assume we have a good OpenGL 2.0 and working glut or freeglut.
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glut.h>
#endif
// Next, get the functions necessary for reading from and writing to pipes.
#ifdef _WIN32
    #include <io.h>
    #define READ _read
    #define WRITEFUNC _write
    #define CLOSE _close
#else
    #include <unistd.h>
    #define READ read
    #define WRITEFUNC write
    #define CLOSE close
#endif

#ifdef _MSC_VER
#pragma warning(disable:4996) // don't warn about strerror, sprintf, etc.
#endif

// gcc 4.4.3 complains bitterly if you don't check the return
// status from the write() system call. This avoids those
// warnings and maybe, someday, will catch an error.
#define WRITE(pipeno, buf, len) \
   {int status=WRITEFUNC((pipeno), (buf), (len)); \
    SimTK_ERRCHK4_ALWAYS(status!=-1, "simbody-visualizer",  \
    "An attempt to write() %d bytes to pipe %d failed with errno=%d (%s).", \
    (len),(pipeno),errno,strerror(errno));}

// This is the transform giving the pose of the camera's local frame in the
// model's ground frame. The camera local frame has Y as the up direction,
// -Z as the "look at" direction, and X to the right. We can't know a good
// default transform for the camera until we know what the SimTK::System
// we're viewing considers to be its "up" and "look at" directions.
static fTransform X_GC;

// Communication with the simulator.
static int inPipe, outPipe;
// If the simulator told us to stop communication, then we close the outPipe
// and can no longer write to the simulator.
static std::atomic<bool> writeToSimulator{true};

// On Mac, if the simbody-visualizer.app/Contents/MacOS/Info.plist's field
// NSHighResolutionCapable is set to true, then this function is expected
// to return 2.0 for high-DPI screens.
// This function could also be used for other operating systems that support
// high-DPI screens. For now, this is just a stub.
// https://developer.apple.com/library/archive/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/EnablingOpenGLforHighResolution/EnablingOpenGLforHighResolution.html#//apple_ref/doc/uid/TP40001987-CH1001-SW4
static int getScalingFactor() {
    #if defined(__APPLE__)
    // If we figure out a way to get the "backing scale factor", we can increase
    // this number to support high-DPI displays.
        return 1;
    #else
        return 1;
    #endif
}
using namespace std;
using namespace mexplus;
using namespace OpenSim;
using namespace SimTK;
class extendVisualizer {
public:
	extendVisualizer(const __int64 cPtr){        
		mexPrintf("Calling constructor.\n");
		m_visualizer = (ModelVisualizer *)cPtr;
		simbodyVisualizer = &(m_visualizer->updSimbodyVisualizer());
		simbodyVisualizer->setShowSimTime(false);
		simbodyVisualizer->setShowFrameNumber(false);
		simbodyVisualizer->setBackgroundType(SimTK::Visualizer::BackgroundType(2));
		simbodyVisualizer->setBackgroundColor(SimTK::Vec3(0,0,0));
		mexPrintf("Model:'%s'.\n", (m_visualizer->getModel()).getName());
	}
	virtual ~extendVisualizer(){
		mexPrintf("Calling destructor.\n");
  //delete m_visualizer;
  //delete simbodyVisualizer;
	}
private:
	ModelVisualizer* m_visualizer = NULL;
    SimTK::Visualizer* simbodyVisualizer = NULL;
};
class SaveImageTask : public ParallelWorkQueue::Task {
public:
    Array_<unsigned char> data;
    SaveImageTask(const string& filename, int width, int height) : filename(filename), width(width), height(height), data(width*height*3) {
    }
    void execute() override {
// Flip the image vertically, since OpenGL and PNG use different row orders.
        
        const int rowLength = 3*width;
        for (int row = 0; row < height/2; ++row) {
            const int base1 = row*rowLength;
            const int base2 = (height-1-row)*rowLength;
            for (int i = 0; i < rowLength; i++) {
                unsigned char temp = data[base1+i];
                data[base1+i] = data[base2+i];
                data[base2+i] = temp;
            }
        }
        LodePNG::encode(filename, data.empty() ? 0 : &data[0], width, height, 2, 8);
    }
private:
            string filename;
            int width, height;
};

static void writeImage(const string& filename) {
    int width = ((viewWidthPixels+3)/4)*4; // must be a multiple of 4 pixels
    int height = viewHeightPixels;
    
    // Create offscreen buffers for rendering the image.
    
    GLuint frameBuffer, colorBuffer, depthBuffer;
    glGenFramebuffersEXT(1, &frameBuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
    glGenRenderbuffersEXT(1, &colorBuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, colorBuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGB8, width, height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, colorBuffer);
    glGenRenderbuffersEXT(1, &depthBuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthBuffer);
    
    // Render the image and load it into memory.
    
    renderScene();
    SaveImageTask* task = new SaveImageTask(filename, width, height);
    Array_<unsigned char>& data = task->data;
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, &data[0]);
    glDeleteRenderbuffersEXT(1, &colorBuffer);
    glDeleteRenderbuffersEXT(1, &depthBuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glDeleteFramebuffersEXT(1, &frameBuffer);
    
    // Add it to the queue to be saved to disk.
    
    imageSaverQueue.addTask(task);
}

static void saveImage() {
    struct stat statInfo;
    int counter = 0;
    string filename;
    do {
        counter++;
        stringstream namestream;
        namestream << simulatorExecutableName.c_str() << "_";
        namestream << counter;
        namestream << ".png";
        filename = namestream.str();
    } while (stat(filename.c_str(), &statInfo) == 0);
    writeImage(filename);
    //setOverlayMessage("Image saved as:\n"+filename);
}
static void shakeHandsWithSimulator(int fromSimPipe, int toSimPipe) {
    unsigned char handshakeCommand;
    readDataFromPipe(fromSimPipe, &handshakeCommand, 1);
    SimTK_ERRCHK2_ALWAYS(handshakeCommand == StartupHandshake,
            "simbody-visualizer::shakeHandsWithSimulator()",
            "Expected initial handshake command %u but received %u. Can't continue.",
            (unsigned)StartupHandshake, (unsigned)handshakeCommand);
    
    unsigned SimVersion;
    readDataFromPipe(fromSimPipe, (unsigned char*)&SimVersion, sizeof(unsigned int));
    SimTK_ERRCHK2_ALWAYS(SimVersion == ProtocolVersion,
            "simbody-visualizer::shakeHandsWithSimulator()",
            "The Simbody Visualizer class protocol version %u is not compatible with "
            " simbody-visualizer protocol %u; this may be an installation problem."
            " Can't continue.",
            SimVersion, ProtocolVersion);
    
    // Get Simbody version number as major,minor,patch
    readDataFromPipe(fromSimPipe, (unsigned char*)simbodyVersion, 3*sizeof(int));
    simbodyVersionStr = String(simbodyVersion[0]) + "." + String(simbodyVersion[1]);
    if (simbodyVersion[2]) simbodyVersionStr += "." + String(simbodyVersion[2]);
    
    unsigned exeNameLength;
    char exeNameBuf[256]; // just a file name, not a path name
    readDataFromPipe(fromSimPipe, (unsigned char*)&exeNameLength, sizeof(unsigned));
    SimTK_ASSERT_ALWAYS(exeNameLength <= 255,
            "simbody-visualizer: executable name length violates protocol.");
    readDataFromPipe(fromSimPipe, (unsigned char*)exeNameBuf, exeNameLength);
    exeNameBuf[exeNameLength] = (char)0;
    
    simulatorExecutableName = std::string(exeNameBuf, exeNameLength);
    
    WRITE(outPipe, &ReturnHandshake, 1);
    WRITE(outPipe, &ProtocolVersion, sizeof(unsigned));
}
template class mexplus::Session<extendVisualizer>;
namespace {
    MEX_DEFINE(new) (int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]){
        InputArguments input(nrhs, prhs, 1);
        OutputArguments output(nlhs, plhs, 1);
        output.set(0, Session<extendVisualizer>::create(new extendVisualizer(input.get<__int64>(0))));
    }
    MEX_DEFINE(delete) (int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]){
        InputArguments input(nrhs, prhs, 1);
        OutputArguments output(nlhs, plhs, 0);
        Session<extendVisualizer>::destroy(input.get(0));
    }
} //namespace
MEX_DISPATCH