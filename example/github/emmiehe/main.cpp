#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string.h> 

//include header file for glfw library so that we can use OpenGL
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
static DWORD lastTime;
#else
static struct timeval lastTime;
#endif

#define PI 3.14159265 // Should be used from mathlib

using namespace std;

//****************************************************
// Global Variables
//****************************************************
GLfloat translation[3] = {0.0f, 0.0f, 0.0f};
bool auto_strech = false;
int Width_global = 400;
int Height_global = 400;

inline float sqr(float x) { return x*x; }
float*** cmd = (float***)calloc(1, sizeof(float**));

// More global varibles

// ds is <= 5 groups of directional light sources;
float** ds;
// ps is <= 5 groups of point light sources;
float** ps;

float* dl;
float* I_d;
float* pl;
float* I_p;

float* ka;
float* kd;
float* ks;

// view direction
float* view;

// exponent
float* sp_coef;
float* spu;
float* spv;
float* a_sm;

//****************************************************
// Simple init function
//****************************************************
void initializeRendering()
{
    glfwInit();
}


//****************************************************
// A routine to set a pixel by drawing a GL point.  This is not a
// general purpose routine as it assumes a lot of stuff specific to
// this example.
//****************************************************
void setPixel(float x, float y, GLfloat r, GLfloat g, GLfloat b) {
    glColor3f(r, g, b);
    glVertex2f(x+0.5, y+0.5);  // The 0.5 is to target pixel centers
    // Note: Need to check for gap bug on inst machines.
}

//****************************************************
// Keyboard inputs
//****************************************************
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key) {
            
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        case GLFW_KEY_Q: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        case GLFW_KEY_LEFT :
            if (action) translation[0] -= 0.01f * Width_global; break;
        case GLFW_KEY_RIGHT:
            if (action) translation[0] += 0.01f * Width_global; break;
        case GLFW_KEY_UP   :
            if (action) translation[1] += 0.01f * Height_global; break;
        case GLFW_KEY_DOWN :
            if (action) translation[1] -= 0.01f * Height_global; break;
        case GLFW_KEY_F:
            if (action) auto_strech = !auto_strech; break;
        case GLFW_KEY_SPACE: break;
            
        default: break;
    }
    
}


//****************************************************
// Some helper functions I used to draw the sphere
//****************************************************


// normalize takes in a vector and normalize it;
float* normalize(float *ar) {
    float *result  = (float*) calloc(3, sizeof(float));
    float length = sqrt(ar[0] * ar[0] + ar[1] * ar[1] + ar[2]*ar[2]);
    result[0] = ar[0] / length;
    result[1] = ar[1] / length;
    result[2] = ar[2] / length;
    //cout << "( " << result[0] << ", "<< result[1] << ", " << result[2] << " )" <<endl;
    return result;
}


// max returns the bigger float
float max(float a, float b){
    if (a < b) {
        return b;
    }
    return a;
}

// dot product of two vectors, return a float number
float dot(float* a, float* b) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
    
}

// cross product of two vectors, return a vector
float* cross(float *a, float *b){
    float *c = (float*) calloc(3, sizeof(float));
    c[0] = a[1]*b[2] - a[2]*b[1];
    c[1] = a[2]*b[0] - a[0]*b[2];
    c[2] = a[0]*b[1] - a[1]*b[0];
    return c;
    
}

// vector addition
float* vector_add(float *a, float *b){
    float *c = (float*) calloc(3, sizeof(float));
    c[0] = a[0] + b[0];
    c[1] = a[1] + b[1];
    c[2] = a[2] + b[2];
    return c;
}

// vector scaling
float* vector_scal(float *a, float s){
    float *b = (float*) calloc(3, sizeof(float));
    b[0] = s*a[0];
    b[1] = s*a[1];
    b[2] = s*a[2];
    return b;
}

// ambient component = I * k_a
float* ambient(float *ka, float * I) {
    float *result = (float*) calloc(3, sizeof(float));
    result[0] = ka[0] * I[0];
    result[1] = ka[1] * I[1];
    result[2] = ka[2] * I[2];
    return result;
}

// diffuse component = I * k_d * max(dot(normalized light, surface normal), 0)
float* diffuse(float* kd, float * I, float *nl, float *nn){
    float *result = (float*) calloc(3, sizeof(float));
    float m = max(dot(nl, nn), 0);
    result[0] = kd[0] * I[0] * m;
    result[1] = kd[1] * I[1] * m;
    result[2] = kd[2] * I[2] * m;
    //cout << "( " << result[0] << ", "<< result[1] << ", " << result[2] << " )" <<endl;
    return result;
}

// specular component = I * k_s * max(dot(r, view), 0)_p; r = -l + 2(dot(l, n))*n
float* specular(float* ks, float * I, float *nl, float *nn, float *nv, float* sp_coef){
    
    float *result = (float*) calloc(3, sizeof(float));
    
    float *nr = (float*) calloc(3, sizeof(float));
    
    nr = vector_add(vector_scal(nl, -1), vector_scal(nn, 2*dot(nl, nn)));
    nr = normalize(nr);
    
    float m = max(dot(nr, nv), 0); //nv should be already normalized
    float m_p = pow(m, sp_coef[0]);
    result[0] = ks[0] * I[0] * m_p;
    result[1] = ks[1] * I[1] * m_p;
    result[2] = ks[2] * I[2] * m_p;
    //free(nr);
    //free(temp);
    return result;
}



float* asm_diff(float* kd, float* ks, float *nl, float *nn, float *nv, float *I){
    
    float *result = (float*) calloc(3, sizeof(float));
    float temp1 = 28/(23*PI);
    float temp2 = (1 - pow((1 - dot(nn, nv)/2), 5)) * (1 - pow((1 - dot(nn, nl)/2), 5));
    float *f = (float*) calloc(3, sizeof(float));
    f[0] = kd[0]*(1 - ks[0]);
    f[1] = kd[1]*(1 - ks[1]);
    f[2] = kd[2]*(1 - ks[2]);
    result[0] = temp1*temp2*f[0]*I[0];
    result[1] = temp1*temp2*f[1]*I[1];
    result[2] = temp1*temp2*f[2]*I[2];
    return result;
}



float* asm_spec(float* ks, float *nl, float *nn, float *nv, float* spu, float* spv, float* h, float* U, float* V, float* I){
    
    float *result = (float*) calloc(3, sizeof(float));
    float p = (spu[0]*pow(dot(h, U), 2) + spv[0]*pow(dot(h, V), 2))/(1 - pow(dot(h, nn), 2));
    
    
    float temp1 = sqrt((spu[0] + 1)*(spv[0]+1))/(8*PI);
    
    float temp2 = (pow(dot(nn, h), p))/(dot(h, nv)*max(dot(nn, nv), dot(nn, nl)));
    
    
    float* f = (float*)calloc(3, sizeof(float));
    f[0] = ks[0] + (1-ks[0])*pow((1 - dot(h, nv)), 5);
    f[1] = ks[1] + (1-ks[1])*pow((1 - dot(h, nv)), 5);
    f[2] = ks[2] + (1-ks[2])*pow((1 - dot(h, nv)), 5);
    
    result[0] = temp1*temp2*f[0]*I[0];
    result[1] = temp1*temp2*f[1]*I[1];
    result[2] = temp1*temp2*f[2]*I[2];
    return result;
}

//****************************************************
// Draw a filled circle.
//****************************************************

void drawCircle(float centerX, float centerY, float radius) {
    // Draw inner circle
    glBegin(GL_POINTS);

    // We could eliminate wasted work by only looping over the pixels
    // inside the sphere's radius.  But the example is more clear this
    // way.  In general drawing an object by loopig over the whole
    // screen is wasteful.

    int minI = max(0,(int)floor(centerX-radius));
    int maxI = min(Width_global-1,(int)ceil(centerX+radius));

    int minJ = max(0,(int)floor(centerY-radius));
    int maxJ = min(Height_global-1,(int)ceil(centerY+radius));
    
    // viewing point
    float view[3] = {0, 0, 1};
    

    for (int i = 0; i < Width_global; i++) {
        for (int j = 0; j < Height_global; j++) {

            // Location of the center of pixel relative to center of sphere
            float x = (i+0.5-centerX);
            float y = (j+0.5-centerY);

            float dist = sqrt(sqr(x) + sqr(y));

            if (dist <= radius) {

                // This is the front-facing Z coordinate
                float z = sqrt(radius*radius-dist*dist);
                //cout << "( " << x << ", "<< y << ", " << z << " )" <<endl;
                
                
                float *temp = (float*) calloc(3, sizeof(float));
                temp[0] = x;
                temp[1] = y;
                temp[2] = z;
                
                float *normal = normalize(temp);
                //free(temp);
                float *nv = normalize(view);
                
                // V and U
                float *y = (float*) calloc(3, sizeof(float));
                y[0] = 0;
                y[1] = 1;
                y[2] = 0;
                float *V = normalize(vector_add(y, vector_scal(normal, (-1)*dot(normal, y))));
                float *U = normalize(cross(V, normal));
                
                // default rgb should be (0, 0, 0);
                float *rgb1 = (float*) calloc(3, sizeof(float));
                rgb1[0] = 0;
                rgb1[1] = 0;
                rgb1[2] = 0;
                float *rgb2 = (float*) calloc(3, sizeof(float));
                rgb2[0] = 0;
                rgb2[1] = 0;
                rgb2[2] = 0;
                
                //if there are multiple directional sources
                if (ds){
                    int i = 0;
                    while (ds[i]) {
                        dl = &(ds[i][0]);
                        I_d = &(ds[i][3]);
                        
                        // normalize directional light
                        float *ndl = normalize(vector_scal(dl, -1));
                        // half vector
                        float *hdl = normalize(vector_add(ndl, nv));
                        
                        
                        
                        // asm attempt
                        if (spu&&spv&&a_sm){
                            //cout << "ASM" <<endl;
                            rgb1 = vector_add(vector_add(vector_add(ambient(ka, I_d),
                                                                    asm_diff(kd, ks, ndl, normal, nv, I_d)),
                                                         asm_spec(ks, ndl, normal, nv, spu, spv, hdl, U, V, I_d)), rgb1);
                            
                        }else if (spu && spv && !a_sm){  // if there is spu and spv to replace exp
                        
                            float *new_dp = (float*) calloc(1, sizeof(float));
                            
                            //alternative way of calculating the exponent
                            //new_dp[0] = spu[0] * pow(dot(normalize(vector_add(hdl, vector_scal(normal, (-1)*dot(hdl, normal)))), U), 2) + spv[0] * (1 - pow(dot(normalize(vector_add(hdl, vector_scal(normal, (-1)*dot(hdl, normal)))), U), 2));
                            
                            
                            new_dp[0] = (spu[0]*pow(dot(hdl, U), 2) + spv[0]*pow(dot(hdl, V), 2))/(1 - pow(dot(hdl, normal), 2));
                            
                            
                            rgb1 = vector_add(vector_add(vector_add(ambient(ka, I_d),
                                                         diffuse(kd, I_d, ndl, normal)),
                                              specular(ks, I_d, ndl, normal, nv, new_dp)), rgb1);
                            
                            
                            
                        }else {
                            rgb1 = vector_add(vector_add(vector_add(ambient(ka, I_d),
                                                         diffuse(kd, I_d, ndl, normal)),
                                              specular(ks, I_d, ndl, normal, nv, sp_coef)), rgb1);
                            
                        }
                        i += 1;
                        //cout << i << " directional light" << endl;
                    }
                
                }
                
                if (ps){
                    //cout << "there is point light!" << endl;
                    int i = 0;
                    while (ps[i]) {
                        pl = &(ps[i][0]);
                        I_p = &(ps[i][3]);
                    
                        float *npl = normalize(vector_add(pl, vector_scal(normal, -1)));
                
                        float *hpl = normalize(vector_add(npl, nv));
                        
                        
                        // asm attempt
                        if (spu&&spv&& a_sm){
                            rgb2 = vector_add(vector_add(vector_add(ambient(ka, I_p),
                                                                    asm_diff(kd, ks, npl, normal, nv, I_p)),
                                                         asm_spec(ks, npl, normal, nv, spu, spv, hpl, U, V, I_p)), rgb2);
                        }else if (spu && spv && !a_sm){
                            float *new_pp = (float*) calloc(1, sizeof(float));
                             
                            //new_pp[0] = spu[0] * pow(dot(normalize(vector_add(hpl, vector_scal(normal, (-1)*dot(hpl, normal)))), U), 2) + spv[0] * (1 - pow(dot(normalize(vector_add(hpl, vector_scal(normal, (-1)*dot(hpl, normal)))), U), 2));
                        
                            
                            new_pp[0] = (spu[0]*pow(dot(hpl, U), 2) + spv[0]*pow(dot(hpl, V), 2))/(1 - pow(dot(hpl, normal), 2));
                             
                            
                            rgb2 = vector_add(vector_add(vector_add(ambient(ka, I_p),
                                                                diffuse(kd, I_p, npl, normal)),
                                                     specular(ks, I_p, npl, normal, nv, new_pp)), rgb2);
                                                     
                            
                        }else {
                    
                            rgb2 = vector_add(vector_add(vector_add(ambient(ka, I_p),
                                                                diffuse(kd, I_p, npl, normal)),
                                                     specular(ks, I_p, npl, normal, nv, sp_coef)), rgb2);
                            //cout << rgb2[0] << ", "<< rgb2[1] << ", " << rgb2[2] << endl;
                        }
                        i += 1;
                    }
                }
                
                //cout << "( " << rgb2[0] << ", "<< rgb2[1] << ", " << rgb2[2] << " )" <<endl;
                float *rgb = vector_add(rgb1, rgb2);

    
                //cout << "( " << rgb[0] << ", "<< rgb[1] << ", " << rgb[2] << " )" <<endl;
                
            
                setPixel(i, j, rgb[0], rgb[1], rgb[2]);
                //free(rgb1);
                //free(rgb2);
                //free(rgb);

                // This is amusing, but it assumes negative color values are treated reasonably.
                // setPixel(i,j, x/radius, y/radius, z/radius );
                
                // Just for fun, an example of making the boundary pixels yellow.
                /*
                if (dist > (radius-1.0)) {
                    setPixel(i, j, 1.0, 1.0, 0.0);
                }
                 */
            }
        }
    }

    glEnd();
}

//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void display( GLFWwindow* window)
{
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f ); //clear background screen to black
    
    glClear(GL_COLOR_BUFFER_BIT);                // clear the color buffer (sets everything to black)
    
    glMatrixMode(GL_MODELVIEW);                  // indicate we are specifying camera transformations
    glLoadIdentity();                            // make sure transformation is "zero'd"
    
    //----------------------- code to draw objects --------------------------
    glPushMatrix();
    glTranslatef (translation[0], translation[1], translation[2]);
    
    drawCircle(Width_global / 2.0 , Height_global / 2.0 , min(Width_global, Height_global) * 0.9 / 2.0);
    
    
    glPopMatrix();
    
    glfwSwapBuffers(window);
    
}

//****************************************************
// function that is called when window is resized
//***************************************************
void size_callback(GLFWwindow* window, int width, int height)
{
    // Get the pixel coordinate of the window
    // it returns the size, in pixels, of the framebuffer of the specified window
    glfwGetFramebufferSize(window, &Width_global, &Height_global);
    
    glViewport(0, 0, Width_global, Height_global);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Width_global, 0, Height_global, 1, -1);
    
    display(window);
}


//****************************************************
// function that is reading the command line opt
//***************************************************

float*** readCmd(int argc, char *argv[]){
    //ka, kd, ks, spu, spv, sp, pl, dl
    float*** retVal = (float***)calloc(9, sizeof(float**));
    int i = 1, plc = 0, dlc = 0;
    while (i < argc) {
        //cout<<"start"<<endl;
        if (strcmp(argv[i], "-ka") == 0){
            //cout<<"ka found"<<endl;
            float** ret = (float**)calloc(1, sizeof(float*));
            retVal[0] = ret;
            float* r = (float*)calloc(3, sizeof(float));
            ret[0] = r;
            r[0] = stof(argv[i+1]);
            r[1] = stof(argv[i+2]);
            r[2] = stof(argv[i+3]);
            i += 4;
        } else if (strcmp(argv[i], "-kd") == 0){
            //cout<<"kd found"<<endl;
            float** ret = (float**)calloc(1, sizeof(float*));
            retVal[1] = ret;
            float* r = (float*)calloc(3, sizeof(float));
            ret[0] = r;
            r[0] = stof(argv[i+1]);
            r[1] = stof(argv[i+2]);
            r[2] = stof(argv[i+3]);
            i += 4;
        } else if (strcmp(argv[i], "-ks") == 0){
            //cout<<"ks found"<<endl;
            float** ret = (float**)calloc(1, sizeof(float*));
            retVal[2] = ret;
            float* r = (float*)calloc(3, sizeof(float));
            ret[0] = r;
            r[0] = stof(argv[i+1]);
            r[1] = stof(argv[i+2]);
            r[2] = stof(argv[i+3]);
            i += 4;
        } else if (strcmp(argv[i], "-spu") == 0){
            //cout<<"spu found"<<endl;
            float** ret = (float**)calloc(1, sizeof(float*));
            retVal[3] = ret;
            float* r = (float*)calloc(1, sizeof(float));
            ret[0] = r;
            r[0] = stof(argv[i+1]);
            i += 2;
        } else if (strcmp(argv[i], "-spv") == 0){
            //cout<<"spv found"<<endl;
            float** ret = (float**)calloc(1, sizeof(float*));
            retVal[4] = ret;
            float* r = (float*)calloc(1, sizeof(float));
            ret[0] = r;
            r[0] = stof(argv[i+1]);
            i += 2;
        } else if (strcmp(argv[i], "-sp") == 0){
            //cout<<"sp found"<<endl;
            float** ret = (float**)calloc(1, sizeof(float*));
            retVal[5] = ret;
            float* r = (float*)calloc(1, sizeof(float));
            ret[0] = r;
            r[0] = stof(argv[i+1]);
            i += 2;
        } else if (strcmp(argv[i], "-pl") == 0){
            //cout<<"pl found"<<endl;
            
            if (plc == 0){
                float** ret = (float**)calloc(5, sizeof(float*));
                
                retVal[6] = ret;
            }
            float* r = (float*) calloc(6, sizeof(float));
            retVal[6][plc] = r;
            r[0] = stof(argv[i+1]);
            r[1] = stof(argv[i+2]);
            r[2] = stof(argv[i+3]);
            r[3] = stof(argv[i+4]);
            r[4] = stof(argv[i+5]);
            r[5] = stof(argv[i+6]);
            plc += 1;
            i += 7;
        } else if (strcmp(argv[i], "-dl") == 0){
            //cout<<"dl found"<<endl;
            
            if (dlc == 0){
                float** ret = (float**)calloc(5, sizeof(float*));
                retVal[7] = ret;
            }
            float* r = (float*) calloc(6, sizeof(float));
            retVal[7][dlc] = r;
            r[0] = stof(argv[i+1]);
            r[1] = stof(argv[i+2]);
            r[2] = stof(argv[i+3]);
            r[3] = stof(argv[i+4]);
            r[4] = stof(argv[i+5]);
            r[5] = stof(argv[i+6]);
            dlc += 1;
            i += 7;
        }else if (strcmp(argv[i], "-asm") == 0){
            //cout<<"asm found"<<endl;
            float** ret = (float**)calloc(1, sizeof(float*));
            retVal[8] = ret;
            float* r = (float*)calloc(1, sizeof(float));
            ret[0] = r;
            r[0] = 1;
            i += 1;
            
        } else {
            cout << "Unrecognized command! " << endl;
            return NULL;
        }

        
    }
    return retVal;

}




//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
    //cout << argc << endl;
    if (argc > 1) {
        cmd = readCmd(argc, argv);
        if (cmd) {
            for (int i = 0; i < 9; ++i) {
                //cout << "start assigning" << endl;
                if (cmd[i]){
                    if (i == 0) {
                        ka = cmd[i][0];
                    }else if (i == 1) {
                        kd = cmd[i][0];
                    }else if (i == 2) {
                        ks = cmd[i][0];
                    }else if (i == 3) {
                        spu = cmd[i][0];
                    }else if (i == 4) {
                        spv = cmd[i][0];
                    }else if (i == 5) {
                        sp_coef = cmd[i][0];
                    }else if (i == 6) {
                        ps = cmd[i];
                        //pl = &(ps[0][0]);
                        //I_p = &(ps[0][3]);
                    }else if (i == 7) {
                        ds = cmd[i];
                        //dl = &(ds[0][0]);
                        //I_d = &(ds[0][3]);
                    }else if (i == 8) {
                        a_sm = cmd[i][0];
                    }
                }
            }
        }
    }
    
    if (!ka) {
        ka = (float*)calloc(3, sizeof(float));
        ka[0] = 0;
        ka[1] = 0;
        ka[2] = 0;
    }
    if (!kd) {
        kd = (float*)calloc(3, sizeof(float));
        kd[0] = 0;
        kd[1] = 0;
        kd[2] = 0;
    }
    if (!ks) {
        ks = (float*)calloc(3, sizeof(float));
        ks[0] = 0;
        ks[1] = 0;
        ks[2] = 0;
    }
    /*
    if (!spu) {
        spu = (float*) calloc(1, sizeof(float));
        spu[0] = 1;
    }
    if (!spv) {
        spv = (float*) calloc(1, sizeof(float));
        spv[0] = 1;
    }
     */
    if (!sp_coef) {
        sp_coef = (float*) calloc(1, sizeof(float));
        sp_coef[0] = 1;
    }
    
    if (!ps){
        pl = (float*) calloc(3, sizeof(float));
        pl[0] = 0;
        pl[1] = 0;
        pl[2] = 0;
        I_p = (float*) calloc(3, sizeof(float));
        I_p[0] = 0;
        I_p[1] = 0;
        I_p[2] = 0;
    }
    if (!ds){
        dl = (float*) calloc(3, sizeof(float));
        dl[0] = 0;
        dl[1] = 0;
        dl[2] = 0;
        I_d = (float*) calloc(3, sizeof(float));
        I_d[0] = 0;
        I_d[1] = 0;
        I_d[2] = 0;
    }
    if (!a_sm) {
        a_sm = NULL;
    }
    
    //This initializes glfw
    initializeRendering();
    
    GLFWwindow* window = glfwCreateWindow( Width_global, Height_global, "CS184", NULL, NULL );
    if ( !window )
    {
        cerr << "Error on window creating" << endl;
        glfwTerminate();
        return -1;
    }
    
    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if ( !mode )
    {
        cerr << "Error on getting monitor" << endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent( window );
    
    // Get the pixel coordinate of the window
    // it returns the size, in pixels, of the framebuffer of the specified window
    glfwGetFramebufferSize(window, &Width_global, &Height_global);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Width_global, 0, Height_global, 1, -1);
    
    glfwSetWindowTitle(window, "CS184");
    glfwSetWindowSizeCallback(window, size_callback);
    glfwSetKeyCallback(window, key_callback);
    
    
    
    while( !glfwWindowShouldClose( window ) ) // infinite loop to draw object again and again
    {   // because once object is draw then window is terminated
        display( window);
        
        if (auto_strech){
            glfwSetWindowSize(window, mode->width, mode->height);
            glfwSetWindowPos(window, 0, 0);
        }
        
        glfwPollEvents();
        
    }
    

    return 0;
}