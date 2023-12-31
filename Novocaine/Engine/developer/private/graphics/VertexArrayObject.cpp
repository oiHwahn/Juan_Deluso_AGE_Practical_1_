#include "graphics/VertexArrayObject.h"
#include "glew/glew.h"
#include "graphics/ShapeMatrices.h"

VertexArrayObject::VertexArrayObject(const TArray<NovoVertex>& vertexData, const TArray<NovoUint>& indexData, const NovoUint RowSize)
	: VertexData(vertexData), IndexData(indexData)
{
	VaoID = VboID = EabID = 0;

	// ask open GL to generate an ID for this VAO
	glGenVertexArrays(1, &VaoID);

	// if VAO fails to generate ID, delete this class
	if (VaoID == GL_INVALID_VALUE) {
		NOVO_MSG_ERR("Vertex Array Object", "VAO failed to generate an Open GL ID.");
		delete this;
		return;
	}

	// activate the VAO so we can work on it
	Bind();

	// set the id's and buffers for the vbo (vertex positions) and eab (vertex indices)
	GenerateAndSetBuffers();

	// save the data in the vertex matrix to a readable format for shaders
	// point to the positions in the vertex matrix
	// find the positions in the matrix
	SetAttributePointer(0, 3, GL_FLOAT, RowSize * sizeof(float), 0);
	
	// store the colours for the shader
	SetAttributePointer(4, 3, GL_FLOAT, RowSize * sizeof(float), (void*) (3 * sizeof(float)));

 
	// once everything is set clear the VAO from the VAO slot
	Unbind();
}

VertexArrayObject::~VertexArrayObject()
{
	// no need to delete anything if it never worked
	if (VaoID == GL_INVALID_VALUE)
		return;

	glDeleteVertexArrays(1, &VaoID);
	glDeleteBuffers(1, &VboID);
	glDeleteBuffers(1, &EabID);
}

void VertexArrayObject::SetAttributePointer(NovoUint index, NovoUint size, NovoUint type, int stride, const void* pointer)
{
	// create a slot in the attribute pointer array for this data 
	glEnableVertexAttribArray(index);
	// we can add the data into the array element
	glVertexAttribPointer(index, size, type, GL_FALSE, stride, pointer);
}

void VertexArrayObject::Bind()
{
	glBindVertexArray(VaoID);
}

void VertexArrayObject::Unbind()
{
	glBindVertexArray(0);
}

void VertexArrayObject::GenerateAndSetBuffers()
{
	///// set up the vertex matrix

	// generate an ID for the VBO
	glGenBuffers(1, &VboID);

	// set the postions buffer so that open gl understands how to read our vertices
	// bind this buffer
	glBindBuffer(GL_ARRAY_BUFFER, VboID);
	// set the values
	glBufferData(GL_ARRAY_BUFFER,
		VertexData.size() * sizeof(NovoVertex), // size of the buffer
		VertexData.data(),
		GL_STATIC_DRAW);

	///// set up the index matrix

	// generate an ID for the EAB
	glGenBuffers(1, &EabID);

	// ditto above but for indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EabID);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		IndexData.size() * sizeof(NovoUint),
		IndexData.data(),
		GL_STATIC_DRAW);
}

void VertexArrayObject::Draw()
{
	Bind();
	glDrawElements(GL_TRIANGLES, IndexData.size(), GL_UNSIGNED_INT, 0);
	Unbind();
}

TArray<NovoVertex> NovoVertex::ConvertShapeMatrix(ShapeMatrices Shape)
{
	// create a local array
	TArray<NovoVertex> VertexArray;
	
	// loop every 3 positions since we know a vertex is made up of 3 numbers (x, y, z)
	for (NovoUint i = 0; i < Shape.Data.size(); i += Shape.RowSize) {
		// assign the number based on the index + its relevant value
		// this will get position
		glm::vec3 vPosition = glm::vec3(
			Shape.Data[i], 
			Shape.Data[i + 1], 
			Shape.Data[i + 2]);

		// this will get colour
		glm::vec3 vColour = glm::vec3(
			Shape.Data[i + 3],     
			Shape.Data[i + 4], 
			Shape.Data[i + 5]); 

		// add the position into the vertex array
		VertexArray.push_back(NovoVertex(vPosition, vColour));
	}

	return VertexArray;
}
 