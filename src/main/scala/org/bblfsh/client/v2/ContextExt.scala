package org.bblfsh.client.v2
	
import java.nio.ByteBuffer

/**
  * Represents Go-side results of Libuast.decode()
  *
  * This is equivalent of pyuast.ContextExt API
  */
case class ContextExt(nativeContext: Long) {
    @native def root(): Node
    // @native def load() // TODO(bzz): implement after clarifying when it's needed
    @native def filter()
    @native def encode(n: Node): ByteBuffer

    @native def dispose()
    override def finalize(): Unit = {
        this.dispose()
    }
}


/**
  * Represents JVM-side constructed tree
  *
  * This is equivalent of pyuast.Context API
  */
case class Context(nativeContext: Long) {
    @native def root(): JNode
    @native def filter()
    @native def encode(n: JNode): ByteBuffer

    @native def dispose()
    override def finalize(): Unit = {
        this.dispose()
    }
}
object Context {
    @native def create(): Long
    def apply(): Context = new Context(create())
}
