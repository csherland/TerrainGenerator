//
// Display a color cube
//
// Colors are assigned to each vertex and then the rasterizer interpolates
//   those colors across the triangles.  We us an orthographic projection
//   as the default projetion.

#include "Angel.h"
#include "SOIL.h"
#include <stdlib.h>     
#include <time.h>
#include <iostream>

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
typedef Angel::vec3  point3; 

#define sideLength 512
int w = 512, h = 512;

using namespace std;

// Triangles
point4 points[6*(sideLength-1)*(sideLength-1)];
color4 colors[6*(sideLength-1)*(sideLength-1)];
point4 vertTx[6*(sideLength-1)*(sideLength-1)];
point3 normal[6*(sideLength-1)*(sideLength-1)];
vec2 tex_cord[6*(sideLength-1)*(sideLength-1)];

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Xaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

GLuint  theta;  // The location of the "theta" shader uniform variable

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;

// Texture objects and storage for texture image
enum { water = 0, grass = 1, rock = 2, snow = 3, numTex = 4 };
GLuint textures[4];
GLuint textureItem[4];


color4 colorForHeight(double h) {
    color4 retVal;
    
    if (h < 0.01) {
        retVal = color4(0.0,0.2,1.0,1.0);
    } else if (h < .05) {
        retVal = color4(0.0,1.0,0.0,1.0);
    } else if (h < .2) {
        retVal = color4(1.0,1.0,1.0,1.0);
    } else {
    	cout << "cool guy" << endl;
        retVal = color4(1.0,1.0,1.0,1.0);
    }
    
    return retVal;
} 

point4 textForHeight(double h) {
	point4 textureVal;
	
	if (h < 0.01) {			//water
        textureVal = color4(1.0,0.0,0.0,0.0);
    } else if (h < .05) {	//grass
        textureVal = color4(0.0,1.0,0.0,0.0);
    } else if (h < .12) {	//rock
        textureVal = color4(0.0,0.0,1.0,0.0);
    } else {				//snow
        textureVal = color4(0.0,0.0,0.0,1.0);
    }
	
	return textureVal;
}

// Determine normal for triangle of three points
point3 calcNorm(point4 p1, point4 p2, point4 p3) {
    point4 u = p2 - p1;
    point4 v = p3 - p1;
    
    point3 normV = normalize(cross(u,v));
    return normV;
    
}

// Implement diamond square algorithm to generate terrain
// CRITICAL ASSUMPTION: sideLength is a power of 2
void diamondSquareAlg() {
    // Matrix to store diamond-square generated heights
    double diamondSquare[sideLength][sideLength];
    
    // Initialize height map to all 0
    for (int i = 0; i < sideLength; ++i) {
        for (int j = 0; j < sideLength; ++j) {
            diamondSquare[i][j] = 0;
        }
    }
    
    //Set up variables
    int range = sideLength;
    double randHeight = 1;
    
    while (range > 1) {
        // Diamond step
        for (int i = range; i < sideLength; i += range) {
            for (int j = range; j < sideLength; j += range) {
                double a = diamondSquare[i - range][j - range];
                double b = diamondSquare[i][j - range];
                double c = diamondSquare[i - range][j];
                double d = diamondSquare[i][j];
                
                double e = diamondSquare[i - range / 2][j - range / 2] = 
                	(a + b + c + d) / 4 
                	+ (double)rand()/(RAND_MAX+1)*(randHeight)+randHeight/2;
            }
        }
        
        // Square step
        for (int i = 2 * range; i < sideLength; i += range) {
            for (int j = 2 * range; j < sideLength; j += range) {
                double a = diamondSquare[i - range][j - range];
                double b = diamondSquare[i][j - range];
                double c = diamondSquare[i - range][j];
                double d = diamondSquare[i][j];
                double e = diamondSquare[i - range / 2][j - range / 2];
                
                double f = diamondSquare[i - range][j - range / 2] = 
                	(a + c + e + diamondSquare[i - 3 * range / 2][j - range / 2]) / 4 
                	+ (double)rand()/(RAND_MAX+1)*(randHeight)+randHeight/2;
                double g = diamondSquare[i - range / 2][j - range] = 
                	(a + b + e + diamondSquare[i - range / 2][j - 3 * range / 2]) / 4 
                	+ (double)rand()/(RAND_MAX+1)*(randHeight)+randHeight/2;
            }
        }
        randHeight /= 2; range /= 2; // Reduce range of algorithm
    } 
    
    // Everything with negative height is water (0 = sea level)
    for (int i = 0; i < sideLength; ++i) {
        for (int j = 0; j < sideLength; ++j) {
            if (diamondSquare[i][j] < 0) {
                diamondSquare[i][j] = 0;
            }
        }
    }
    
    // Turn height matrix into vertices to be drawn as triangles
    int ind = 0;
    for (int i = 0; i < sideLength - 1; ++i) {
        for (int j = 0; j < sideLength - 1; ++j) {
    		// Generate points and texture coordinates           
    		points[ind] = point4(2*double(i+1)/sideLength-1, diamondSquare[i+1][j], 2*double(j)/sideLength-1, 1.0);
            tex_cord[ind] = vec2(1.0,1.0);
            colors[ind] = colorForHeight(diamondSquare[i][j]);
            vertTx[ind] = textForHeight(diamondSquare[i][j]);
            ind++;
            
            points[ind] = point4(2*double(i)/sideLength-1, diamondSquare[i][j], 2*double(j)/sideLength-1, 1.0);
            tex_cord[ind] = vec2(0.0,0.0);
            colors[ind] = colorForHeight(diamondSquare[i][j]);
            ind++;
            
            points[ind] = point4(2*double(i)/sideLength-1, diamondSquare[i][j+1], 2*double(j+1)/sideLength-1, 1.0);
            tex_cord[ind] = vec2(0.0,1.0);
            colors[ind] = colorForHeight(diamondSquare[i][j]);
            vertTx[ind] = textForHeight(diamondSquare[i][j]);
            ind++;
            
            points[ind] = point4(2*double(i+1)/sideLength-1, diamondSquare[i+1][j], 2*double(j)/sideLength-1, 1.0);
            tex_cord[ind] = vec2(1.0,0.0);
            colors[ind] = colorForHeight(diamondSquare[i][j]);
            vertTx[ind] = textForHeight(diamondSquare[i][j]);
            ind++;
            
            points[ind] = point4(2*double(i)/sideLength-1, diamondSquare[i][j+1], 2*double(j+1)/sideLength-1, 1.0);
            tex_cord[ind] = vec2(0.0,1.0);
            colors[ind] = colorForHeight(diamondSquare[i][j]);
            vertTx[ind] = textForHeight(diamondSquare[i][j]);
            ind++;
            
            points[ind] = point4(2*double(i+1)/sideLength-1, diamondSquare[i+1][j+1], 2*double(j+1)/sideLength-1, 1.0);
            tex_cord[ind] = vec2(1.0,1.0);
            colors[ind] = colorForHeight(diamondSquare[i][j]);
            vertTx[ind] = textForHeight(diamondSquare[i][j]);
            ind++;
            
            //Calculate normal for this guy
            ind -= 6;
            point3 norm = calcNorm(points[ind],points[ind+1],points[ind+2]);
            
            //Set Normals
            normal[ind] = norm; ind++;
            normal[ind] = norm; ind++;
            normal[ind] = norm; ind++;
            
            norm = -calcNorm(points[ind+2],points[ind+1],points[ind]);
            normal[ind] = norm; ind++;
            normal[ind] = norm; ind++;
            normal[ind] = norm; ind++;
        }
    }
}


// OpenGL initialization
void init() {
	// Generate the terrain!
    diamondSquareAlg();
    
    // TEXTURE STUFF ---------------------------------------------------
    // Initialize texture objects and bind the three textures
    
    // Load .png images as textures 
	textures[water] = SOIL_load_OGL_texture("WaterTexture.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	textures[grass] = SOIL_load_OGL_texture("GrassTexture.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	textures[rock]  = SOIL_load_OGL_texture("RockTexture.png",  SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	textures[snow]  = SOIL_load_OGL_texture("SnowTexture.png",  SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	
	// check for an error during the load process 
	if( (textures[water] == 0) || (textures[grass] == 0) || (textures[rock] == 0) || (textures[snow] == 0) ) {
		cout << "Error loading textures." << endl;
		exit(1);
	}
	
	// Setup program for textures
	glGenTextures(4, textureItem);
	
	// Bind water texture to texture 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[water]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    // Bind grass texture to texture 1
    glActiveTexture(GL_TEXTURE1); 
	glBindTexture(GL_TEXTURE_2D, textures[grass]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    // Bind rock texture to texture 2
    glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textures[rock]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    // Bind snow texture to texture 3
    glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, textures[snow]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    //fixes a weird bug
    glActiveTexture(GL_TEXTURE0);
    
    // Create a vertex array object
    GLuint vao;
    glGenVertexArraysAPPLE(1, &vao);
    glBindVertexArrayAPPLE(vao);

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normal) + sizeof(tex_cord) + sizeof(colors) + sizeof(vertTx),
		  NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) , sizeof(normal), normal);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normal), sizeof(tex_cord), tex_cord);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normal) + sizeof(tex_cord), sizeof(colors), colors);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normal) + sizeof(tex_cord) + sizeof(colors), sizeof(vertTx), vertTx);

    // Load shaders and use the resulting shader program
    GLuint program = InitShader("vshader_env.glsl", "fshader_env.glsl");
    glUseProgram(program);

    // Set up vertex arrays
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0));
    
    // Lighting stuff (from Chapter 5-3)
    GLuint vNormal = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(points)) );
                          
    //Texture Coordinates
    GLuint vTexCoord = glGetAttribLocation( program, "vTexCoord" );
    glEnableVertexAttribArray( vTexCoord );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(points)+sizeof(normal)) );
                          
    // Color stuff
    GLuint vColor = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(points)+sizeof(normal)+sizeof(tex_cord)) );
                        
	// Which texture to apply to fragment
    GLuint vText = glGetAttribLocation( program, "vText" );
    glEnableVertexAttribArray( vText );
    glVertexAttribPointer( vText, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(points)+sizeof(normal)+sizeof(tex_cord)+sizeof(colors)) );
     
    // Initialize shader lighting parameters
    point4 light_position(0.75, 0.0, -1.0, 0.0 );
    color4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
    color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
    color4 light_specular( 1.0, 1.0, 1.0, 1.0 );
     
    color4 material_ambient( 1.0, 1.0, 1.0, 1.0 );
    color4 material_diffuse( 0.8, 0.8, 0.8, 1.0 );
    color4 material_specular( 0.8, 0.8, 0.8, 1.0 );
    float  material_shininess = 8.0;
     
    color4 ambient_product = light_ambient * material_ambient;
    color4 diffuse_product = light_diffuse * material_diffuse;
    color4 specular_product = light_specular * material_specular;
     
    glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),
     1, ambient_product );
    glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),
     1, diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),
     1, specular_product );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"),
    1, light_position );
      
    glUniform1f( glGetUniformLocation(program, "Shininess"),
    material_shininess );
     
    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation(program, "ModelView");
    Projection = glGetUniformLocation(program, "Projection");
    
    // Set the value of the fragment shader texture sampler variable
    //   ("texture") to the the appropriate texture unit. 
    glUniform1i(glGetUniformLocation(program, "waterTex"), 0 );
    glUniform1i(glGetUniformLocation(program, "grassTex"), 1 );
    glUniform1i(glGetUniformLocation(program, "rockTex"), 2 );
    glUniform1i(glGetUniformLocation(program, "snowTex"), 3 );
    
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glClearColor(1.0, 1.0, 1.0, 1.0);
}


// Display function
vec3 viewer_pos( 0.0, 0.0, 2.0 );
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//const vec3 viewer_pos( 0.0, 0.0, 2.0 );
    mat4  model_view = ( Translate( -viewer_pos ) *
                        RotateX( Theta[Xaxis] ) *
                        RotateY( Theta[Yaxis] ) *
                        RotateZ( Theta[Zaxis] ) );
    
    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );
    glDrawArrays(GL_TRIANGLES, 0, 6*(sideLength-1)*(sideLength-1));
    
    glutSwapBuffers();
}

double yAng = 45.0;
void reshape(int width, int height) {
	w = width; h = height;
    glViewport( 0, 0, width, height );
    
    GLfloat aspect = GLfloat(width)/height;
    mat4  projection = Perspective( yAng, aspect, 0.5, 3.0 );
    
    glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
}

// Handle keyboard interactions
void keyboard(unsigned char key, int x, int y) {
    switch( key ) {
    	// Exit program
        case 033:  	  // Escape key	
            exit(EXIT_SUCCESS);
            break;
        
        // Handle user moving through view space
        case 'q':
        	viewer_pos.x += 0.1;
        	break;
        case 'a':   
        	viewer_pos.x -= 0.1;
            break;
        case 'w':    
        	viewer_pos.y += 0.1;
            break;
        case 's':     
        	viewer_pos.y -= 0.1;
            break;
        case 'e':
        	viewer_pos.z += 0.1;
        	break;
        case 'd':     
        	viewer_pos.z -= 0.1;
            break;
    	
    	// Handle user view rotation
    	case 'y':
    		Theta[Xaxis] += 0.5;
    		break;
    	case 'h':
    		Theta[Xaxis] -= 0.5;
    		break;
    	case 'u':
    		Theta[Yaxis] += 0.5;
    		break;
    	case 'j':
    		Theta[Yaxis] -= 0.5;
    		break;
    	case 'i':
    		Theta[Zaxis] += 0.5;
    		break;
    	case 'k':
    		Theta[Zaxis] -= 0.5;
    		break;
    		
    	// Allow user to scale size of the image
    	case '0':
    		yAng += 1.0; //Scale smaller
    		reshape(w,h);
    		break;
    	case '9':
    		yAng -= 1.0; //Scale  bigger
    		reshape(w,h);
    		break; 
    		
    	// Default switch case
        default:
            cout << "Invalid keyboard interaction." << endl;
            
    }
}

void idle(void) {
    if ( Theta[Axis] > 360.0 ) {
        Theta[Axis] -= 360.0;
    }
    
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    srand (time(NULL)); // Initialize random seed
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w,h);
    glutCreateWindow("Environment Generator");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    glutReshapeFunc( reshape );

    glutMainLoop();
    return 0;
}
