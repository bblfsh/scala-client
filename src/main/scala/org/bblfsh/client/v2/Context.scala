package org.bblfsh.client.v2
	
import java.nio.ByteBuffer

case class Context(nativeContext: Long) {
    def this() = this(Context.create())

    @native def root(): Node
    @native def encode(n: Node): ByteBuffer
    @native def dispose()

    @native def filter() // TODO(bzz)

    // TODO(bzz): add loading of the root node, after clarifying when it's needed
    // https://github.com/bblfsh/client-python/blob/master/bblfsh/pyuast.cc#L364
    // @native def load(): ?

    override def finalize(): Unit = {
        this.dispose()
    }
}

object Context {
    @native def create(): Long
}