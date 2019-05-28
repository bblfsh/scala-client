package org.bblfsh.client.v2
	
import java.nio.ByteBuffer
	
case class Context(nativeContext: Long) {
    @native def root(): Node
    @native def encode(n: Node, format: Int): ByteBuffer
    @native def dispose()

    @native def filter()

    override def finalize(): Unit = {
        this.dispose()
    }
}
