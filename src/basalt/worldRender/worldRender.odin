package worldRender

import gl "vendor:OpenGL"

import skeewb "skeewb:core"
import "../world"
import "../util"
import mesh "meshGenerator"

ChunkBuffer :: struct{
    x, y, z: i32,
	VAO, VBO, EBO: u32,
    length: i32
}

iVec3 :: struct {
    x, y, z: i32
}

chunkMap := make(map[iVec3]ChunkBuffer)

setupChunk :: proc (chunk: world.Chunk) -> ChunkBuffer {
    indices, vertices := mesh.generateMesh(chunk)
    defer delete(indices)
    defer delete(vertices)
    VAO, VBO, EBO: u32
    
	gl.GenVertexArrays(1, &VAO)
	gl.BindVertexArray(VAO)

	gl.GenBuffers(1, &VBO)
    gl.BindBuffer(gl.ARRAY_BUFFER, VBO)
    gl.BufferData(gl.ARRAY_BUFFER, len(vertices)*size_of(vertices[0]), raw_data(vertices), gl.STATIC_DRAW)
	
	gl.GenBuffers(1, &EBO)
	gl.BindBuffer(gl.ELEMENT_ARRAY_BUFFER, EBO)
	gl.BufferData(gl.ELEMENT_ARRAY_BUFFER, len(indices)*size_of(indices[0]), raw_data(indices), gl.STATIC_DRAW)

	gl.EnableVertexAttribArray(0)
	gl.EnableVertexAttribArray(1)
	gl.EnableVertexAttribArray(2)
	gl.EnableVertexAttribArray(3)
	gl.VertexAttribPointer(0, 3, gl.FLOAT, false, 9 * size_of(f32), 0)
	gl.VertexAttribPointer(1, 3, gl.FLOAT, false, 9 * size_of(f32), 3 * size_of(f32))
	gl.VertexAttribPointer(2, 2, gl.FLOAT, false, 9 * size_of(f32), 6 * size_of(f32))
	gl.VertexAttribPointer(3, 1, gl.FLOAT, false, 9 * size_of(f32), 8 * size_of(f32))

    return ChunkBuffer{chunk.x, chunk.y, chunk.z, VAO, VBO, EBO, i32(len(indices))}
}

eval :: proc (chunk: world.Chunk) -> ChunkBuffer {
    pos := iVec3{chunk.x, chunk.y, chunk.z}
    chunkBuffer, ok, _ := util.map_force_get(&chunkMap, pos)
    if ok {
        chunkBuffer^ = setupChunk(chunk)
    }
    return chunkBuffer^
}

setupManyChunks :: proc (chunks: [dynamic]world.Chunk) -> [dynamic]ChunkBuffer {
    chunksBuffers: [dynamic]ChunkBuffer

    for chunk in chunks {
        append(&chunksBuffers, eval(chunk))
    }

    return chunksBuffers;
}

nuke :: proc () {
	for pos, &chunk in chunkMap {
		gl.DeleteBuffers(1, &chunk.VBO)
		gl.DeleteBuffers(1, &chunk.EBO)
	}
    delete(chunkMap)
}
