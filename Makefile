GCC_OPTIONS=-pedantic -I ./include -lSOIL
GL_OPTIONS=-framework OpenGL -framework GLUT -framework CoreFoundation 
OPTIONS=$(GCC_OPTIONS) $(GL_OPTIONS)

.cpp: 
	g++ $@.cpp ./Common/initShader.cpp $(OPTIONS) -o $@  

