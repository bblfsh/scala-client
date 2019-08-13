package org.bblfsh.client.v2
	
import java.nio.ByteBuffer

/**
  * Represents Go-side constructed tree, result of Libuast.decode()
  *
  * This is equivalent of pyuast.ContextExt API
  */
case class ContextExt(nativeContext: Long) {
    // @native def load(): JNode // TODO(bzz): clarify when it's needed VS just .root().load()
    @native def root(): NodeExt
    @native def filter()
    @native def encode(n: NodeExt): ByteBuffer

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
