//==============================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2016, CHAI3D.
    (www.chai3d.org)
    This code is adoopted from the Chai3d haptic effects programming examples and from the frameworks Github page: 
*/
//==============================================================================

#include "chai3d.h"
#include <GLFW/glfw3.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080
#include <chrono>
#include <iostream>
#include <sys/time.h>
#include <ctime>

//Enable logging of timestamps in milliseconds since epoch**
using std::cout; using std::endl;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;
using namespace chai3d;
using namespace std;


//------------------------------------------------------------------------------
// GENERAL DISPLAY AND IMAGE RENDERING SETTINGS
//------------------------------------------------------------------------------

// stereo Mode
/*
    C_STEREO_DISABLED:            Stereo is disabled
    C_STEREO_ACTIVE:              Active stereo for OpenGL NVDIA QUADRO cards
    C_STEREO_PASSIVE_LEFT_RIGHT:  Passive stereo where L/R images are rendered next to each other
    C_STEREO_PASSIVE_TOP_BOTTOM:  Passive stereo where L/R images are rendered above each other
*/
cStereoMode stereoMode = C_STEREO_DISABLED;

// fullscreen mode
bool fullscreen = false;

// mirrored display
bool mirroredDisplay = false;


//------------------------------------------------------------------------------
// DECLARED VARIABLES
//------------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera to render the world in the window display
cCamera* camera;

// a light source to illuminate the objects in the world
cDirectionalLight *light;

// a small sphere (cursor) representing the haptic device
cShapeSphere* cursor;

// a line representing the velocity vector of the haptic device
cShapeLine* velocity;

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the current haptic device
cGenericHapticDevicePtr hapticDevice;

// a label to display the haptic device model
cLabel* labelHapticDeviceModel;

// a label to display the position [m] of the haptic device
cLabel* labelHapticDevicePosition;

// a global variable to store the position [m] of the haptic device
cVector3d hapticDevicePosition;

// a font for rendering text
cFontPtr font;

// a label to display the rate [Hz] at which the simulation is running
cLabel* labelRates;

// a flag for using damping (ON/OFF)
bool useDamping = false;

// a flag for using force field (ON/OFF)
bool useForceField = true;

// a flag to indicate if the haptic simulation currently running
bool simulationRunning = false;

// a flag to indicate if the haptic simulation has terminated
bool simulationFinished = true;

// a frequency counter to measure the simulation graphic rate
cFrequencyCounter freqCounterGraphics;

// a frequency counter to measure the simulation haptic rate
cFrequencyCounter freqCounterHaptics;

// haptic thread
cThread* hapticsThread;

// a handle to window display context
GLFWwindow* window = NULL;

// current width of window
int width  = 0;

// current height of window
int height = 0;

// swap interval for the display context (vertical synchronization)
int swapInterval = 1;


//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

// callback when the window display is resized
void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height);

// callback when an error GLFW occurs
void errorCallback(int error, const char* a_description);

// callback when a key is pressed
void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

// this function renders the scene
void updateGraphics(void);

// this function contains the main haptics simulation loop
void updateHaptics(void);

// this function closes the application
void close(void);


//==============================================================================
/*
    DEMO:   mydevice.cpp

    This application illustrates how to program position, forces, torques and gripper forces to the connected haptic device.

    In this code, the application opens an OpenGL window and displays 3D cursor for the haptic device connected to the computer. When the operator presses
    onto the button, the color of the cursor changes from blue to green.

    In the main haptics loop function  "updateHaptics()" , the stylus' position, orientation and user switch status are read at each haptic cycle (every 1 ms.)
    Force and torque vectors are computed and sent back to the haptic device to enable the operator to perceive the resulting haptic feedback.
*/
//==============================================================================

int main(int argc, char* argv[])
{
    //--------------------------------------------------------------------------
    // HAPTIC DEVICE INITIALIZATION
    //--------------------------------------------------------------------------

    cout << endl;
    cout << "-----------------------------------" << endl;
    cout << "CHAI3D" << endl;
    cout << "Demo: Haptic Device Demo" << endl;
    cout << "Copyright 2003-2016" << endl;
    cout << "-----------------------------------" << endl << endl << endl;
    cout << "Keyboard Options:" << endl << endl;
    cout << "[1] - Enable/Disable potential field" << endl;
    cout << "[2] - Enable/Disable damping" << endl;
    cout << "[f] - Enable/Disable full screen mode" << endl;
    cout << "[m] - Enable/Disable vertical mirroring" << endl;
    cout << "[q] - Exit application" << endl;
    cout << endl << endl;


    //----------------------------------------------------------------------------------
    // OPENGL - WINDOW DISPLAY---Invokes the GLUT library for image rendering on screen
    //-----------------------------------------------------------------------------------

    // initialize GLFW library
    if (!glfwInit())
    {
        cout << "failed initialization" << endl;
        cSleepMs(1000);
        return 1;
    }

    // set error callback
    glfwSetErrorCallback(errorCallback);

    // compute desired size of window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int w = 0.8 * mode->height;
    int h = 0.5 * mode->height;
    int x = 0.5 * (mode->width - w);
    int y = 0.5 * (mode->height - h);



    // set OpenGL version---Licensing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // set active stereo mode
    if (stereoMode == C_STEREO_ACTIVE)
    {
        glfwWindowHint(GLFW_STEREO, GL_TRUE);
    }
    else
    {
        glfwWindowHint(GLFW_STEREO, GL_FALSE);
    }

    // create display context
    window = glfwCreateWindow(w, h, "CHAI3D", NULL, NULL);
    if (!window)
    {
        cout << "failed to create window" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // get width and height of window
    glfwGetWindowSize(window, &width, &height);

    // set position of window
    glfwSetWindowPos(window, x, y);

    // set key callback
    glfwSetKeyCallback(window, keyCallback);

    // set resize callback
    glfwSetWindowSizeCallback(window, windowSizeCallback);

    // set current display context
    glfwMakeContextCurrent(window);

    // sets the swap interval for the current display context
    glfwSwapInterval(swapInterval);

#ifdef GLEW_VERSION
    // initialize GLEW library
    if (glewInit() != GLEW_OK)
    {
        cout << "failed to initialize GLEW library" << endl;
        glfwTerminate();
        return 1;
    }
#endif


    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.
    world = new cWorld();

    // set the background color of the environment
    world->m_backgroundColor.setBlack();

    // create a camera and insert it into the virtual world
    camera = new cCamera(world);
    world->addChild(camera);

    // position and orient the camera
    camera->set( cVector3d (0.5, 0.0, 0.0),    // camera position (eye)
                 cVector3d (0.0, 0.0, 0.0),    // look at position (target)
                 cVector3d (0.0, 0.0, 1.0));   // direction of the (up) vector

    // set the near and far clipping planes of the camera
    camera->setClippingPlanes(0.01, 10.0);

    // set stereo mode
    camera->setStereoMode(stereoMode);

    // set stereo eye separation and focal length (applies only if stereo is enabled)
    camera->setStereoEyeSeparation(0.01);
    camera->setStereoFocalLength(0.5);

    // set vertical mirrored display mode
    camera->setMirrorVertical(mirroredDisplay);

    // create a directional light source
    light = new cDirectionalLight(world);

    // insert light source inside world
    world->addChild(light);

    // enable light source
    light->setEnabled(true);

    // define direction of light beam
    light->setDir(-1.0, 0.0, 0.0);

    // create a sphere (cursor) to represent the haptic device
    cursor = new cShapeSphere(0.01);

    // insert cursor inside world
    world->addChild(cursor);

    // create small line to illustrate the velocity of the haptic device
    velocity = new cShapeLine(cVector3d(0,0,0),
                              cVector3d(0,0,0));

    // insert line inside world
    world->addChild(velocity);


    //--------------------------------------------------------------------------
    // HAPTIC DEVICE VARIABLE DEFINITIONS
    //--------------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get a handle to the first haptic device
    handler->getDevice(hapticDevice, 0);

    // open a connection to haptic device
    hapticDevice->open();

    // calibrate device (if necessary)
    hapticDevice->calibrate();

    // retrieve information about the current haptic device
    cHapticDeviceInfo info = hapticDevice->getSpecifications();

    // display a reference frame if haptic device supports orientations
    if (info.m_sensedRotation == true)
    {
        // display reference frame
        cursor->setShowFrame(true);

        // set the size of the reference frame
        cursor->setFrameSize(0.05);
    }

    // if the device has a gripper, enable the gripper to simulate a user switch
    hapticDevice->setEnableGripperUserSwitch(true);


    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    font = NEW_CFONTCALIBRI20();

    // create a label to display the haptic device model
    labelHapticDeviceModel = new cLabel(font);
    camera->m_frontLayer->addChild(labelHapticDeviceModel);
    labelHapticDeviceModel->setText(info.m_modelName);

    // create a label to display the position of haptic device
    labelHapticDevicePosition = new cLabel(font);
    camera->m_frontLayer->addChild(labelHapticDevicePosition);

    // create a label to display the haptic and graphic rate of the simulation
    labelRates = new cLabel(font);
    camera->m_frontLayer->addChild(labelRates);


    //--------------------------------------------------------------------------
    // START HAPTIC SIMULATION
    //--------------------------------------------------------------------------

    // create a thread which starts the main haptics rendering loop
    hapticsThread = new cThread();
    hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

    // setup callback when application exits
    atexit(close);


    //--------------------------------------------------------------------------
    // MAIN GRAPHIC LOOP
    //--------------------------------------------------------------------------

    // call window size callback at initialization
    windowSizeCallback(window, width, height);

    // main graphic loop
    while (!glfwWindowShouldClose(window))
    {
        // get width and height of window
        glfwGetWindowSize(window, &width, &height);

        // render graphics
        updateGraphics();

        // swap buffers
        glfwSwapBuffers(window);

        // process events
        glfwPollEvents();

        // signal frequency counter
        freqCounterGraphics.signal(1);
    }

    // close window
    glfwDestroyWindow(window);

    // terminate GLFW library
    glfwTerminate();

    // exit
    return 0;
}

//------------------------------------------------------------------------------

void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height)
{
    // update window size
    width  = a_width;
    height = a_height;

    // update position of label
    labelHapticDeviceModel->setLocalPos(20, height - 40, 0);

    // update position of label
    labelHapticDevicePosition->setLocalPos(20, height - 60, 0);
}

//------------------------------------------------------------------------------

void errorCallback(int a_error, const char* a_description)
{
    cout << "Error: " << a_description << endl;
}

//------------------------------------------------------------------------------

void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
{
    // filter calls that only include a key press
    if ((a_action != GLFW_PRESS) && (a_action != GLFW_REPEAT))
    {
        return;
    }

    // option - exit
    else if ((a_key == GLFW_KEY_ESCAPE) || (a_key == GLFW_KEY_Q))
    {
        glfwSetWindowShouldClose(a_window, GLFW_TRUE);
    }

    // option - enable/disable force field
    else if (a_key == GLFW_KEY_1)
    {
        useForceField = !useForceField;
        if (useForceField)
            cout << "> Enable force field     \r";
        else
            cout << "> Disable force field    \r";
    }

    // option - enable/disable damping
    else if (a_key == GLFW_KEY_2)
    {
        useDamping = !useDamping;
        if (useDamping)
            cout << "> Enable damping         \r";
        else
            cout << "> Disable damping        \r";
    }

    // option - toggle fullscreen
    else if (a_key == GLFW_KEY_F)
    {
        // toggle state variable
        fullscreen = !fullscreen;

        // get handle to monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        // get information about monitor
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // set fullscreen or window mode
        if (fullscreen)
        {
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            glfwSwapInterval(swapInterval);
        }
        else
        {
            int w = 0.8 * mode->height;
            int h = 0.5 * mode->height;
            int x = 0.5 * (mode->width - w);
            int y = 0.5 * (mode->height - h);
            glfwSetWindowMonitor(window, NULL, x, y, w, h, mode->refreshRate);
            glfwSwapInterval(swapInterval);
        }
    }

    // option - toggle vertical mirroring
    else if (a_key == GLFW_KEY_M)
    {
        mirroredDisplay = !mirroredDisplay;
        camera->setMirrorVertical(mirroredDisplay);
    }
}

//------------------------------------------------------------------------------

void close(void)
{
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }

    // close haptic device
    hapticDevice->close();

    // delete resources
    delete hapticsThread;
    delete world;
    delete handler;
}

//------------------------------------------------------------------------------

void updateGraphics(void)
{
    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////

    // update position data
    labelHapticDevicePosition->setText(hapticDevicePosition.str(3));

    // update haptic and graphic rate data
    labelRates->setText(cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
                        cStr(freqCounterHaptics.getFrequency(), 0) + " Hz");

    // update position of label
    labelRates->setLocalPos((int)(0.5 * (width - labelRates->getWidth())), 15);


    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // update shadow maps (if any)
    world->updateShadowMaps(false, mirroredDisplay);

    // render world
    camera->renderView(width, height);

    // wait until all OpenGL commands are completed
    glFinish();

    // check for any OpenGL errors
    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR) cout << "Error:  %s\n" << gluErrorString(err);
}

//------------------------------------------------------------------------------

void updateHaptics(void)

{

    //socket variables
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };
    string str_position;


    // simulation in now running
    simulationRunning  = true;
    simulationFinished = false;

    // main haptic simulation loop
    while(simulationRunning)
    {
        /////////////////////////////////////////////////////////////////////
        // READ HAPTIC DEVICE STATUS AND SPECIFICATIONS
        /////////////////////////////////////////////////////////////////////
        cVector3d position;
        int counter=0;
        // Creating socket file descriptor to connect with the server controller,i.e. the remote robotic controller
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0))
                == 0) {
                perror("socket failed");
                exit(EXIT_FAILURE);
        }
        // Attaching socket to the port 8080
        if (setsockopt(server_fd, SOL_SOCKET,
                                SO_REUSEADDR | SO_REUSEPORT, &opt,
                                sizeof(opt))) {
                perror("setsockopt");
                exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        
        if (bind(server_fd, (struct sockaddr*)&address,
                        sizeof(address))
                < 0) {
                perror("bind failed");
                exit(EXIT_FAILURE);
        }
        if (listen(server_fd, 3) < 0) {
                perror("listen");
                exit(EXIT_FAILURE);
        }
        while  ((new_socket
                = accept(server_fd, (struct sockaddr*)&address,
                                (socklen_t*)&addrlen))
                > 0) {

                // read position
                cVector3d position;
                hapticDevice->getPosition(position);
                double x_position = position.x();
                double y_position = position.y();
                double z_position = position.z();
                string x = to_string(x_position) + "\n";
                x.insert(0,"X: ");
                string y = to_string(y_position) + "\n";
                y.insert(0,"Y: ");
                string z = to_string(z_position) + "\n";
                z.insert(0,"Z: ");
                string c = to_string(counter) + "\n";
                c.insert(0,"Packet_Label: ");
                const char* finalx= x.c_str();
                const char* finaly= y.c_str();
                const char* finalz= z.c_str();
                const char* finalc= c.c_str();
                //Final position values to send to the slave controller
                send(new_socket, finalx, strlen(finalx), 0);
                send(new_socket, finaly, strlen(finaly), 0);
                send(new_socket, finalz, strlen(finalz), 0);
                send(new_socket, finalc, strlen(finalc), 0);
                auto millisec_since_epoch = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                cout << millisec_since_epoch << endl;
                close(new_socket);
                //Increment counter index and append packet label to the X,Y,Z axes position values
                counter+=1;
                cout << "label: " << counter;
                cout << " ";
      
                hapticDevicePosition = position;

        }
        cMatrix3d rotation;
        hapticDevice->getRotation(rotation);

        // read gripper position
        double gripperAngle;
        hapticDevice->getGripperAngleRad(gripperAngle);

        // read linear velocity
        cVector3d linearVelocity;
        hapticDevice->getLinearVelocity(linearVelocity);

        // read angular velocity
        cVector3d angularVelocity;
        hapticDevice->getAngularVelocity(angularVelocity);

        // read gripper angular velocity
        double gripperAngularVelocity;
        hapticDevice->getGripperAngularVelocity(gripperAngularVelocity);

        // read user-switch status (button 0)
        bool button0, button1, button2, button3;
        button0 = false;
        button1 = false;
        button2 = false;
        button3 = false;

        hapticDevice->getUserSwitch(0, button0);
        hapticDevice->getUserSwitch(1, button1);
        hapticDevice->getUserSwitch(2, button2);
        hapticDevice->getUserSwitch(3, button3);


        /////////////////////////////////////////////////////////////////////
        // UPDATE 3D CURSOR MODEL ON THE GRAPHICS VIEWING SCREEN
        /////////////////////////////////////////////////////////////////////

        // update arrow
        velocity->m_pointA = position;
        velocity->m_pointB = cAdd(position, linearVelocity);

        // update position and orientation of cursor
        cursor->setLocalPos(position);
        cursor->setLocalRot(rotation);

        // adjust the  color of the cursor according to the status of
        // the user-switch (ON = TRUE / OFF = FALSE)
        if (button0)
        {
            cursor->m_material->setGreenMediumAquamarine();
        }
        else if (button1)
        {
            cursor->m_material->setYellowGold();
        }
        else if (button2)
        {
            cursor->m_material->setOrangeCoral();
        }
        else if (button3)
        {
            cursor->m_material->setPurpleLavender();
        }
        else
        {
            cursor->m_material->setBlueRoyal();
        }

        // update global variable for graphic display update
        hapticDevicePosition = position;


        /////////////////////////////////////////////////////////////////////
        // COMPUTE AND APPLY FORCES FOR FEEDBACK PERCEPTION
        /////////////////////////////////////////////////////////////////////

        // Original reference position of the stylus
        cVector3d desiredPosition;
        desiredPosition.set(0.0, 0.0, 0.0);

        // Original reference orientation of the stylus 
        cMatrix3d desiredRotation;
        desiredRotation.identity();

        // Original variables for reference gripper force
        cVector3d force (0,0,0);
        cVector3d torque (0,0,0);
        double gripperForce = 0.0;

        // Define the force field
        if (useForceField)
        {
            // computing linear force --- For the 3D Touch haptic device, the output force maximum force s
            double Kp = 25; // [N/m]
            cVector3d forceField = Kp * (desiredPosition - position);
            force.add(forceField);

            // computing angular torque
            double Kr = 0.05; // [N/m.rad]
            cVector3d axis;
            double angle;
            cMatrix3d deltaRotation = cTranspose(rotation) * desiredRotation;
            deltaRotation.toAxisAngle(axis, angle);
            torque = rotation * ((Kr * angle) * axis);
        }

        // apply damping effects -- to contain vibrations and excess mechanical oscillations and noise
        if (useDamping)
        {
            cHapticDeviceInfo info = hapticDevice->getSpecifications();

            // compute linear damping force
            double Kv = 1.0 * info.m_maxLinearDamping;
            cVector3d forceDamping = -Kv * linearVelocity;
            force.add(forceDamping);

            // compute angular damping force
            double Kvr = 1.0 * info.m_maxAngularDamping;
            cVector3d torqueDamping = -Kvr * angularVelocity;
            torque.add(torqueDamping);

            // compute gripper angular damping force
            double Kvg = 1.0 * info.m_maxGripperAngularDamping;
            gripperForce = gripperForce - Kvg * gripperAngularVelocity;
        }

        // send computed force, torque, and gripper force to haptic device to allow the operator to perceive haptic feedback
        hapticDevice->setForceAndTorqueAndGripperForce(force, torque, gripperForce);

        // signal frequency counter
        freqCounterHaptics.signal(1);
    }

    // exit haptics thread
    simulationFinished = true;
}

//------------------------------------------------------------------------------


